/*
* This plugin comes directly from the source code in xswallow.
* xswallow is an excellent app and I have taken the code becuase
* I wanted to create a very specific netscape plugin for my application,
* gxmlviewer, that I could place into an rpm and have it installed 
* via rpm.
*
* The code here has been modified slightly to accomodate some specific needs.
*
* credits for xswallow.
* > XSwallow documentation is at
* > http://www.csn.ul.ie/~caolan/docs/XSwallow.html
* > Caolan McNamara <caolan@skynet.ie>
*/

#include <stdio.h>
#include "npapi.h"
#include <sys/types.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <string.h>

#include "../gxmlviewer.h"

#define GIVEUP 4000
#define REPARENT_LOOPS 50

#define MIME_DESCRIPTION "text/xml:xml:text/xml"
#define NAME             "npgxmlviewer 1.2"
#define PURPOSE          "gxmlviewer was written by <a href=\"http://members.home.net/sstuckless/index.html\">Sean Stuckless</a> &lt;<a href=\"mailto:sstuckless@home.com\">sstuckless@home.com</a>&gt;.  For more information on gxmlviewer go to <a href=\"http://www.sourceforge.net/projects/gxmlviewer/\">gxmlviewer project homepage</a>."

int abortflag;

/***********************************************************************
* Instance state information about the plugin.
*
* PLUGIN DEVELOPERS:
* Use this struct to hold per-instance information that you'll
* need in the various functions in this file.
***********************************************************************/
typedef struct _PluginInstance {
    Window window;
    Window victim;
    Widget resizewatch;
    Widget netscape_widget;
    Display *display;
    uint32 x, y;
    uint32 width, height;
    int ran;
    int child_pid;
    XtIntervalId timerid;
    int count;
    int fullscreen;

    char *swallowedApp;
    char *appTitle;
    char *xmlFile;
} PluginInstance;

void Redraw(Widget w, XtPointer closure, XEvent * event);
void resizeCB(Widget, PluginInstance *, XEvent *, Boolean *);
void abortswallow(int);
void abortswallowX(Widget, XtPointer, XEvent *);
void do_swallow(PluginInstance *);
void swallow_check(PluginInstance *);
int run_child(char *, const char *, int, int);

char *NPP_GetMIMEDescription(void)
{
	return(MIME_DESCRIPTION);
}

NPError NPP_GetValue(void *future, NPPVariable variable, void *value)
{
    NPError err = NPERR_NO_ERROR;

    switch (variable) {
    case NPPVpluginNameString:
	*((char **) value) = NAME;
	break;
    case NPPVpluginDescriptionString:
	*((char **) value) = PURPOSE;
	break;
    default:
	err = NPERR_GENERIC_ERROR;
    }
    return err;
}

NPError NPP_Initialize(void)
{
    return NPERR_NO_ERROR;
}


jref NPP_GetJavaClass()
{
    return NULL;
}

void NPP_Shutdown(void)
{
}


NPError
NPP_New(NPMIMEType mimeType,
	NPP instance,
	uint16 mode,
	int16 argc, char *argn[], char *argv[], NPSavedData * saved)
{
    PluginInstance *This;


    if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;


    instance->pdata = NPN_MemAlloc(sizeof(PluginInstance));

    This = (PluginInstance *) instance->pdata;
    if (This == NULL)
	return NPERR_OUT_OF_MEMORY_ERROR;

    This->count = 0;
    This->timerid = -1;
    This->child_pid = -1;
    This->fullscreen = 0;
    This->victim = 0;;
    This->width = 0;
    This->height = 0;
    This->swallowedApp = "gxmlviewer";
    /* NOTE: this has to be set to the title of the gxmlviewer application,
    *  or the gxmlviewer application will not be swallowed */
    This->appTitle     = GXMLVIEWER_TITLE;
    This->xmlFile      = (char *)malloc(256);
    This->xmlFile[0]   = 0;
    This->netscape_widget = NULL;
    return NPERR_NO_ERROR;
}


NPError NPP_Destroy(NPP instance, NPSavedData ** save)
{
    PluginInstance *This;

    if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

    This = (PluginInstance *) instance->pdata;

    if (This != NULL) {
	if ((This->timerid != -1) && (This->timerid != -2)) {
	    XtRemoveTimeOut(This->timerid);
	}

	if (This->ran == 2) {
	    XtRemoveEventHandler(This->resizewatch, StructureNotifyMask,
				 False, (XtEventHandler) resizeCB,
				 (XtPointer) This);
	}

	/*
	 * kill child 
	 */
	if (This->child_pid != -1) {
	    kill(This->child_pid * -1, SIGTERM);	/* this should
							 * kill all
							 * children down
							 * along */
	}


	if (This->xmlFile) {
		free(This->xmlFile);
	}

	NPN_MemFree(instance->pdata);
	instance->pdata = NULL;
    }

    return NPERR_NO_ERROR;
}



NPError NPP_SetWindow(NPP instance, NPWindow * window)
{
    static int t;
    PluginInstance *This;

    if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

    if (window == NULL)
	return NPERR_NO_ERROR;

    This = (PluginInstance *) instance->pdata;

    if (t == 0)
	This->window = (Window) window->window;


    This->x = window->x;
    This->y = window->y;
    This->width = window->width;
    This->height = window->height;
    This->display =
	((NPSetWindowCallbackStruct *) window->ws_info)->display;

    /*
     * caolan begin
     * the app wasnt run yet
     */
    if (This->window != (Window) window->window) {
	fprintf(stderr, "gxmlviewer: this should never happen\n");
	return NPERR_NO_ERROR;
    }
	This->window = (Window) window->window;
	This->netscape_widget =
	    XtWindowToWidget(This->display, This->window);
	if (This->ran == 2) {
	    XReparentWindow(This->display, This->victim, This->window, 0,
			    0);
	    XMapWindow(This->display, This->victim);
	    XSync(This->display, FALSE);
	    XResizeWindow(This->display, This->victim, This->width,
			  This->height);
	    XSync(This->display, FALSE);
	} else {
	    XtAddEventHandler(This->netscape_widget, ExposureMask, FALSE,
			      (XtEventHandler) Redraw, This);
	    XtAddEventHandler(This->netscape_widget, ButtonPress, FALSE,
			      (XtEventHandler) abortswallowX, This);
	    Redraw(This->netscape_widget, (XtPointer) This, NULL);


	    if (This->timerid == -2) {
		/*
		 * the stream is ready, but the window wasnt ready at the
		 * time to add the timer 
		 */
		This->child_pid =
		    run_child(This->swallowedApp, This->xmlFile,
			      This->width, This->height);
		if (This->child_pid == -1)
		    fprintf(stderr,
			    "npxmlviewer: unable to lauch gxmlviewer app: %s\n",
			    This->swallowedApp);
		else {
		    setpgid(This->child_pid, This->child_pid);
		    do_swallow(This); 
		}
	    }
	}
   
    return NPERR_NO_ERROR;
}


NPError
NPP_NewStream(NPP instance,
	      NPMIMEType type,
	      NPStream * stream, NPBool seekable, uint16 * stype)
{
    *stype = NP_ASFILEONLY;
    return NPERR_NO_ERROR;
}


int32 STREAMBUFSIZE = 0X0FFFFFFF; 
int32 NPP_WriteReady(NPP instance, NPStream * stream)
{
    return STREAMBUFSIZE;
}


int32
NPP_Write(NPP instance, NPStream * stream, int32 offset, int32 len,
	  void *buffer)
{
    return len; 
}


NPError NPP_DestroyStream(NPP instance, NPStream * stream, NPError reason)
{
    return NPERR_NO_ERROR;
}


void NPP_StreamAsFile(NPP instance, NPStream * stream, const char *fname)
{
    PluginInstance *This = NULL;
    if (instance != NULL)
	This = (PluginInstance *) instance->pdata;
    This->ran = 1;

    /*
     * ready to roll 
     */
    /*
     * but wait until we have a window before we do the bizz 
     */

    abortflag = 0;

		/*
		 * this app is to be run in a window, so must have a
		 * window to be usefull 
		 */

		if (This->netscape_widget != NULL) {
		    This->child_pid =
			run_child(This->swallowedApp, fname,
				  This->width, This->height);
		    if (This->child_pid == -1)
			fprintf(stderr,
				"gxmlviewer: the attempt to run command %s failed\n",
				This->swallowedApp);
		    else {
			setpgid(This->child_pid, This->child_pid);
			do_swallow(This);	/* in here timerid will be 
						 * set away from -1 */
		    }
		} else {
		    This->timerid = -2;	/* inform setwindow to run it
					 * instead */
		    strcpy(This->xmlFile, fname);
		}
}


void NPP_Print(NPP instance, NPPrint * printInfo)
{
    if (printInfo == NULL)
	return;

    if (instance != NULL) {
	if (printInfo->mode == NP_FULL) {
	    /*
	     * PLUGIN DEVELOPERS:
	     * If your plugin would like to take over
	     * printing completely when it is in full-screen mode,
	     * set printInfo->pluginPrinted to TRUE and print your
	     * plugin as you see fit. If your plugin wants Netscape
	     * to handle printing in this case, set
	     * printInfo->pluginPrinted to FALSE (the default) and
	     * do nothing. If you do want to handle printing
	     * yourself, printOne is true if the print button
	     * (as opposed to the print menu) was clicked.
	     * On the Macintosh, platformPrint is a THPrint; on
	     * Windows, platformPrint is a structure
	     * (defined in npapi.h) containing the printer name, port,
	     * etc.
	     */
	    printInfo->print.fullPrint.pluginPrinted = FALSE;
	} else {		 
	    /*
	     *PLUGIN DEVELOPERS:
	     *If your plugin is embedded, or is full-screen
	     *you returned false in pluginPrinted above, NPP_Print
	     *will be called with mode == NP_EMBED. The NPWindow
	     *in the printInfo gives the location and dimensions of
	     *the embedded plugin on the printed page. On the
	     *Macintosh, platformPrint is the printer port; on
	     *Windows, platformPrint is the handle to the printing
	     *device context.
	     */
	}
    }
}
void Redraw(Widget w, XtPointer closure, XEvent * event)
{
    PluginInstance *This = (PluginInstance *) closure;
    GC gc;
    XGCValues gcv;
    char *text = "click to abort xml viewer.";
    XtVaGetValues(w, XtNbackground, &gcv.background,
		  XtNforeground, &gcv.foreground, 0);
    gc = XCreateGC(This->display, This->window,
		   GCForeground | GCBackground, &gcv);

    XDrawRectangle(This->display, This->window, gc,
		   2, 2, This->width - 4, This->height - 4);
    XDrawString(This->display, This->window, gc, This->width / 2 - 100,
		This->height / 2 + 20, text, strlen(text));

}

void
resizeCB(Widget w, PluginInstance * data, XEvent * event, Boolean * cont)
{
    Arg args[2];
    Widget temp;


    /*
     * search up to cound hildren of drawingArea, if > 0 then delete, if
     * not the resize 
     */

    temp = data->netscape_widget;
    while (strcmp(XtName(temp), "drawingArea"))
	temp = XtParent(temp);

    if (data->fullscreen == 0) {
	XReparentWindow(data->display, data->victim,
			XtWindow(data->resizewatch), 0, 0);
	XSync(data->display, FALSE);
    } else {
	XtSetArg(args[0], XtNwidth, (XtArgVal) & (data->width));
	XtSetArg(args[1], XtNheight, (XtArgVal) & (data->height));
	XtGetValues(temp, args, 2);
	XResizeWindow(data->display, data->window, data->width,
		      data->height);
	XResizeWindow(data->display, data->victim, data->width,
		      data->height);
    }
}

int
run_child(char *commandLine, const char *filename,
	  int width, int height)
{
    int childPID;
    int parentPID;

    parentPID = getpid();
    childPID = fork();

    /*
     * if fork failed 
     */
    if (childPID == -1) {
	fprintf(stderr, "gxmlviewer: Fork failed: %s\n", strerror(errno));
	childPID = 0;
    } else if (childPID == 0) {
	pid_t mine = getpid();
	if (setpgid(mine, mine) < 0)
	    fprintf(stderr, "child group set failed\n");
	else {
	   char *args[4];
	   args[0] = commandLine;
	   args[1] = "swallowed";
	   args[2] = (char *)filename;
	   args[3] = NULL;
	   if (execvp(args[0], args) == -1) {
	      fprintf(stderr, "gxmlviewer: unable to launch gxmlviwer app: %s\n", commandLine);
	      return -1;
	   }
	}
    } else {
	return (childPID);
    }
    return (-1);
}

void swallow_check(PluginInstance * This)
{
    Arg args[2];
    Widget temp;
    char *word;
    int i, k, l, j, m, ready = FALSE;
    int width, height;
    char *windowname;
    unsigned int number_of_subkids = 0,
	number_of_subkids2 = 0,
	number_of_kids = 0,
	number_of_kids2 = 0,
	number_of_subsubkids = 0, number_of_subsubkids2 = 0;
    Window root,
	parent,
	*children = NULL,
	*subchildren = NULL,
	*subchildren2 = NULL,
	*subsubchildren = NULL, *children2 = NULL, *subsubchildren2 = NULL;


    Atom type_ret;
    int fmt_ret;
    unsigned long nitems_ret;
    unsigned long bytes_after_ret;
    Window *win = NULL;
    Window *win2 = NULL;
    Atom _XA_WM_CLIENT_LEADER;
    XWMHints *leader_change;

    This->timerid = -1;
    word = This->appTitle;

    if ((This->count < GIVEUP) && (abortflag == 0)) {
	    This->count++;
	    if (children != (Window *) NULL)
		XFree(children);
	    if (0 !=
		XQueryTree(This->display,
			   RootWindowOfScreen(XtScreen
					      (This->netscape_widget)),
			   &root, &parent, &children, &number_of_kids))
		for (i = 0; i < number_of_kids; i++) {
		    if (0 !=
			XFetchName(This->display, children[i],
				   &windowname)) {
			if (!strncmp(windowname, word, strlen(word))) {
			    ready = TRUE;
			    This->victim = children[i];
			}
			XFree(windowname);
		    }
		    if (ready == FALSE) {
			if (subchildren != (Window *) NULL)
			    XFree(subchildren);
			if (0 !=
			    XQueryTree(This->display, children[i], &root,
				       &parent, &subchildren,
				       &number_of_subkids))
			    for (k = 0; k < number_of_subkids; k++) {
				if (0 !=
				    XFetchName(This->display,
					       subchildren[k],
					       &windowname)) {
				    if (!strncmp
					(windowname, word, strlen(word))) {
					ready = TRUE;
					This->victim = subchildren[k];
				    }
				    XFree(windowname);
				}
				if (ready == FALSE) {
				    if (subsubchildren != (Window *) NULL)
					XFree(subsubchildren);
				    if (0 !=
					XQueryTree(This->display,
						   subchildren[k], &root,
						   &parent,
						   &subsubchildren,
						   &number_of_subsubkids))
					for (l = 0;
					     l < number_of_subsubkids;
					     l++) {
					    if (0 !=
						XFetchName(This->display,
							   subsubchildren
							   [l],
							   &windowname)) {
						if (!strncmp
						    (windowname, word,
						     strlen(word))) {
						    ready = TRUE;
						    This->victim =
							subsubchildren[l];
						}
						XFree(windowname);
					    }
					}
				}
			    }
		    }
		}
	    }


	if (ready == TRUE) {

	    /*
	     * *search up the current tree to add a resize event handler
	     */
	    temp = XtWindowToWidget(This->display, This->window);
	    while (strcmp(XtName(temp), "form")) {
		temp = XtParent(temp);
		if (!(strcmp(XtName(temp), "scroller"))) {
		    XtSetArg(args[0], XtNwidth, (XtArgVal) & width);
		    XtSetArg(args[1], XtNheight, (XtArgVal) & height);
		    XtGetValues(temp, args, 2);
		    if ((width == This->width) && (height == This->height))
			This->fullscreen = 1;
		}
		if (!(strcmp(XtName(XtParent(temp)), "drawingArea")))
		    temp = XtParent(temp);
	    }
	    This->resizewatch = temp;
	    This->ran = 2;
	    XtAddEventHandler(This->resizewatch, StructureNotifyMask,
			      False, (XtEventHandler) resizeCB,
			      (XtPointer) This);
	    XResizeWindow(This->display, This->victim, This->width,
			  This->height);
	    XSync(This->display, FALSE);

	    _XA_WM_CLIENT_LEADER =
		XInternAtom(This->display, "WM_CLIENT_LEADER", False);
	    if (XGetWindowProperty
		(This->display, This->victim, _XA_WM_CLIENT_LEADER, 0,
		 sizeof(Window), False, AnyPropertyType, &type_ret,
		 &fmt_ret, &nitems_ret, &bytes_after_ret,
		 (unsigned char **) &win) == Success) {
		if (win != NULL)
		    if (*win == This->victim) {
			/*
			 * if im a multiwindow app, i have to check all the toplevel
			 * windows to see if they're part of me, highly dodge i know
			 * what if they havent popped up yet, and some might later etc
			 * etc. And hey you say, havent you just done this XQuery
			 * already, and the answer is yeah i have, im giving a
			 * possible second window a chance to pop up :-0, so i just
			 * cut and pasted the code in again, basically as
			 * im breaking so many other rules as im going about it, why
			 * stop now. anyhow maybe ill figure out a proper way of doing it
			 * anyone want to buy me some advanced X programming books to
			 * help me achieve this task ? i take visa too.
			 */
			if (0 !=
			    XQueryTree(This->display,
				       RootWindowOfScreen(XtScreen
							  (This->
							   netscape_widget)),
				       &root, &parent, &children2,
				       &number_of_kids2))
			    for (l = 0; l < number_of_kids2; l++) {
				win2 = NULL;
				if (XGetWindowProperty
				    (This->display, children2[l],
				     _XA_WM_CLIENT_LEADER, 0,
				     sizeof(Window), False,
				     AnyPropertyType, &type_ret, &fmt_ret,
				     &nitems_ret, &bytes_after_ret,
				     (unsigned char **) &win2) == Success)
				{
				    if (win2 != NULL)
					if (*win2 == *win) {
					    leader_change =
						XGetWMHints(This->display,
							    children2[l]);
					    leader_change->flags =
						(leader_change->
						 flags | WindowGroupHint);
					    leader_change->window_group =
						RootWindowOfScreen(XtScreen
								   (This->
								    netscape_widget));

					    XSetWMHints(This->display,
							children2[l],
							leader_change);
					    XFree(win2);
					}
				}
				if (0 !=
				    XQueryTree(This->display, children2[l],
					       &root, &parent,
					       &subchildren2,
					       &number_of_subkids2))
				    for (k = 0; k < number_of_subkids2;
					 k++) {
					win2 = NULL;
					if (XGetWindowProperty
					    (This->display,
					     subchildren2[k],
					     _XA_WM_CLIENT_LEADER, 0,
					     sizeof(Window), False,
					     AnyPropertyType, &type_ret,
					     &fmt_ret, &nitems_ret,
					     &bytes_after_ret,
					     (unsigned char **) &win2) ==
					    Success) {
					    if (win2 != NULL)
						if (*win2 == *win) {

						    leader_change =
							XGetWMHints(This->
								    display,
								    subchildren2
								    [k]);
						    leader_change->flags =
							(leader_change->
							 flags |
							 WindowGroupHint);
						    leader_change->
							window_group =
							RootWindowOfScreen
							(XtScreen
							 (This->
							  netscape_widget));

						    XSetWMHints(This->
								display,
								subchildren2
								[k],
								leader_change);

						    XFree(win2);
						}
					}
					XQueryTree(This->display,
						   subchildren2[k], &root,
						   &parent,
						   &subsubchildren2,
						   &number_of_subsubkids2);
					for (m = 0;
					     m < number_of_subsubkids2;
					     m++) {
					    win2 = NULL;
					    if (XGetWindowProperty
						(This->display,
						 subsubchildren2[m],
						 _XA_WM_CLIENT_LEADER, 0,
						 sizeof(Window), False,
						 AnyPropertyType,
						 &type_ret, &fmt_ret,
						 &nitems_ret,
						 &bytes_after_ret,
						 (unsigned char **) &win2)
						== Success) {
						if (win2 != NULL)
						    if (*win2 == *win) {

							leader_change =
							    XGetWMHints
							    (This->display,
							     subsubchildren2
							     [m]);
							leader_change->
							    flags =
							    (leader_change->
							     flags |
							     WindowGroupHint);
							leader_change->
							    window_group =
							    RootWindowOfScreen
							    (XtScreen
							     (This->
							      netscape_widget));

							XSetWMHints(This->
								    display,
								    subsubchildren2
								    [m],
								    leader_change);

							XFree(win2);
						    }
					    }
					}
				    }
			    }
			if (subsubchildren2 != (Window *) NULL)
			    XFree(subsubchildren2);
			if (subchildren2 != (Window *) NULL)
			    XFree(subchildren2);
			if (children2 != (Window *) NULL)
			    XFree(children2);
		    }
	    }
	    if (win != (Window *) NULL)
		XFree(win);

	    XSync(This->display, FALSE);

	    XWithdrawWindow(This->display, This->victim,
			    XScreenNumberOfScreen(XtScreen
						  (This->
						   netscape_widget)));
	    XSync(This->display, FALSE);
	    XMapWindow(This->display, This->window);
	    XResizeWindow(This->display, This->window, This->width,
			  This->height);
	    XSync(This->display, FALSE);

	    for (j = 0; j < REPARENT_LOOPS; j++) {
		/*
		 * more bloody serious dodginess
		 */
		 /**/
		    XReparentWindow(This->display, This->victim,
				    This->window, 0, 0);
		XSync(This->display, FALSE);
	     /**/}


	    XMapWindow(This->display, This->victim);
	    XSync(This->display, FALSE);

	    if (children != (Window *) NULL)
		XFree(children);
	    if (subchildren != (Window *) NULL)
		XFree(subchildren);
	    if (subsubchildren != (Window *) NULL)
		XFree(subsubchildren);
	} else
	    This->timerid =
		XtAppAddTimeOut(XtDisplayToApplicationContext
				(This->display), 333,
				(XtTimerCallbackProc) swallow_check,
				(XtPointer) This);
    }


void do_swallow(PluginInstance * This)
{
	This->timerid =
	    XtAppAddTimeOut(XtDisplayToApplicationContext(This->display),
			    333, (XtTimerCallbackProc) swallow_check,
			    (XtPointer) This);
	/*
	 * add a timer 
	 */
}


void abortswallow(int ignored)
{
    fprintf(stderr,
	    "gxmlviewer:The app to be swallowed appears not to have been launched,dang!\n");
    abortflag = 1;
}

void abortswallowX(Widget w, XtPointer closure, XEvent * whocares)
{
    PluginInstance *This = (PluginInstance *) closure;
    XtRemoveEventHandler(This->netscape_widget, ExposureMask, FALSE,
			 (XtEventHandler) Redraw, This);
    XtRemoveEventHandler(This->netscape_widget, ButtonPress, FALSE,
			 (XtEventHandler) abortswallowX, This);
    XClearWindow(This->display, This->window);
    if (This->child_pid != -1)
	kill(This->child_pid * -1, SIGTERM);	/* this should kill all
						 * children down along */
    abortflag = 1;
}
