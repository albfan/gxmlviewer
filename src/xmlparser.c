/*
 * $Id: xmlparser.c,v 1.5 2001/11/29 01:28:46 sean_stuckless Exp $
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include <gtk/gtk.h>
#ifndef WIN32
#include <SAX.h>
#include <parser.h>
#include <parserInternals.h>
#else
#include <libxml/SAX.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#endif
#include "xmlparser.h"
#include "support.h"

/* different parse states */
#define STATE_UNKNOWN   0 /* unknown/uninitialized state */
#define STATE_STARTTAG  1 /* handling start tag */
#define STATE_TEXT      2 /* handling text */
#define STATE_ENDTAG    3 /* handling end tag */
#define STATE_CDATA     4 /* handling cdata */
#define STATE_COMMENT   5 /* handling comment */


/* internal Application Parse State structure, used to track the parsing */
typedef struct _AppParseState AppParseState;

struct _AppParseState {
   int state;               /* keep track of state, will match one of STATE_XXX above */
   GtkWidget *tree;         /* gtk_tree to fill */
   GtkWidget *parent_tree;  /* always points to the absolute parent. ie, the tree that we started with. */

   /* various styles to apply to text */
   GtkStyle *style_tag;
   GtkStyle *style_quoted_text;
   GtkStyle *style_attr;
   GtkStyle *style_text;
   GtkStyle *style_comment;
   GtkStyle *style_cdata;
   GtkStyle *style_error;

   /* text containers for various types of data */
   GtkWidget *start_tag;
   GtkWidget *end_tag;
   GtkWidget *body_text;
};

/* SAX call back functions */
static void handleStartDocument(AppParseState *state);
static void handleEndDocument(AppParseState *state);
static void handleStartElement(AppParseState *state, char *name, char **attr);
static void handleEndElement(AppParseState *state, char *name);
static void handleCharacters(AppParseState *state, char *data, int len);
static void handleCdataBlock(AppParseState *state, char *data, int len);
static void handleComment (AppParseState *state, char *data);
static void handleDTD(AppParseState *state, char *name, char *publicId, char *systemId);


/* prototypes */
inline void addNode(AppParseState * state);
inline void addTextNode(AppParseState * state);
inline void addStartNode(AppParseState * state);
inline void addEndNode(AppParseState * state);
inline void forgetNode(AppParseState * state);
inline void setState(AppParseState * state, int stateValue);
inline GtkStyle *createStyle(GdkColor forecolor);
inline GdkColor createColor(int red, int green, int blue);
inline GtkWidget *createTextContainer(void);
inline void addColorText(AppParseState * state, GtkStyle * color, const char *strText);
inline void addErrorText(AppParseState * state, char *strText);
inline GtkWidget *getCurrentTextContainer(AppParseState * state);
void gxmlviewer_init(AppParseState *state, GtkTree *parent);

/* SAX Structure */
static xmlSAXHandler appSAXParser = {
    (internalSubsetSAXFunc) handleDTD,	/* internalSubset */
    0,				/* isStandalone */
    0,				/* hasInternalSubset */
    0,				/* hasExternalSubset */
    0,				/* resolveEntity */
    (getEntitySAXFunc) NULL,	/* getEntity */
    0,				/* entityDecl */
    0,				/* notationDecl */
    0,				/* attributeDecl */
    0,				/* elementDecl */
    0,				/* unparsedEntityDecl */
    0,				/* setDocumentLocator */
    (startDocumentSAXFunc) handleStartDocument,	/* startDocument */
    (endDocumentSAXFunc) handleEndDocument,	/* endDocument */
    (startElementSAXFunc) handleStartElement,	/* startElement */
    (endElementSAXFunc) handleEndElement,	/* endElement */
    0,				/* reference */
    (charactersSAXFunc) handleCharacters,	/* characters */
    0,				/* ignorableWhitespace */
    0,				/* processingInstruction */
    (commentSAXFunc) handleComment,	/* comment */
    (warningSAXFunc) NULL,	/* warning */
    (errorSAXFunc) NULL,	/* error */
    (fatalErrorSAXFunc) NULL,	/* fatalError */
    (getParameterEntitySAXFunc) NULL,	/* getParameterEntity */
    (cdataBlockSAXFunc) handleCdataBlock,	/* cdataBlock */
    (externalSubsetSAXFunc) NULL,	/* externalSubset */
};

/* reset current node information */
inline void forgetNode(AppParseState * state)
{
    state->body_text = NULL;
    state->start_tag = NULL;
    state->end_tag = NULL;
    state->state = STATE_UNKNOWN;
}

/* add text (no additional indent)*/
inline void addTextNode(AppParseState * state)
{
    GtkWidget *treeItem = NULL;
    DEBUG_CMD(g_message("adding text node"));
    treeItem = gtk_tree_item_new();
    gtk_container_add(GTK_CONTAINER(treeItem), state->body_text);
    gtk_tree_append(GTK_TREE(state->tree), treeItem);
    gtk_widget_show(treeItem);
    gtk_widget_show(state->body_text);
    state->body_text = NULL;
}

/* add a start element node (create an indentation)*/
inline void addStartNode(AppParseState * state)
{
    GtkWidget *subTree = NULL;
    GtkWidget *treeItem = NULL;

    DEBUG_CMD(g_message("adding start tag."));

    subTree = gtk_tree_new();
    treeItem = gtk_tree_item_new();
    gtk_container_add(GTK_CONTAINER(treeItem), state->start_tag);
    gtk_widget_show(state->start_tag);
    gtk_tree_append(GTK_TREE(state->tree), treeItem);
    gtk_tree_item_set_subtree(GTK_TREE_ITEM(treeItem), subTree);
    gtk_widget_show(treeItem);
    state->tree = subTree;
}

/* add a end element node (close an indentation)*/
inline void addEndNode(AppParseState * state)
{
  GtkWidget *treeItem = NULL;
  GtkObject *treeItemObj = NULL;

  DEBUG_CMD(g_message("adding end tag."));

  treeItemObj = GTK_OBJECT(GTK_TREE(state->tree)->tree_owner);

  state->tree = GTK_WIDGET(state->tree->parent);
  treeItem = gtk_tree_item_new();
  gtk_container_add(GTK_CONTAINER(treeItem), state->end_tag);
  gtk_tree_append(GTK_TREE(state->tree), treeItem);
  gtk_widget_show(treeItem);

  /* Show or hide closing tag */
  gtk_signal_connect_object(GTK_OBJECT(treeItemObj),
			    "collapse", gtk_widget_hide,
			    GTK_OBJECT(state->end_tag));
  gtk_signal_connect_object(GTK_OBJECT(treeItemObj),
			    "expand", gtk_widget_show,
			    GTK_OBJECT(state->end_tag));
  
}

/* general add node method.  It will figure out what type of node to add
 * and then proceed to build the appropriate node */
inline void addNode(AppParseState * state)
{
    GtkWidget *treeItem = NULL;
    GList *list;

    if (state->state == STATE_UNKNOWN) {
	DEBUG_CMD(g_message("gxmlviewer_add_node(): state == UNKNOWN, node not added."));
	return;
    }

    /* check for start tag without end tag */
    if (state->start_tag != NULL && state->end_tag == NULL) {
	addStartNode(state);
	if (state->body_text != NULL) {
	    addTextNode(state);
	}
	
    /* check for an end tage without a start tag */
    } else if (state->start_tag == NULL && state->end_tag != NULL) {
	if (state->body_text != NULL) {
	    addTextNode(state);
	}
	addEndNode(state);
    
    /* have both start and end tags, and maybe some text */
    } else if (state->start_tag != NULL && state->end_tag != NULL) {
	DEBUG_CMD(g_message("adding start tag and end tag."));
	treeItem = gtk_tree_item_new();
	if (state->body_text == NULL) {
	    DEBUG_CMD(g_message("no text so will compress the nodes"));
	    gtk_widget_destroy(GTK_WIDGET(state->end_tag));
	    /* get last box, and modify this */
	    list = gtk_container_children(GTK_CONTAINER(state->start_tag));
	    gtk_label_set_text(GTK_LABEL( g_list_last(list)->data), " />");
	    g_list_free(list);

	    gtk_container_add(GTK_CONTAINER(treeItem),
			      GTK_WIDGET(state->start_tag));
	    gtk_tree_append(GTK_TREE(state->tree), treeItem);
	    gtk_widget_show(treeItem);
	    gtk_widget_show(state->start_tag);
	    gtk_widget_show(state->end_tag);
	} else {
	    DEBUG_CMD(g_message("Added text before the end node"));
	    gtk_box_pack_start(GTK_BOX(state->start_tag),
			       GTK_WIDGET(state->body_text), FALSE, FALSE, 0);
	    gtk_box_pack_start(GTK_BOX(state->start_tag),
			       GTK_WIDGET(state->end_tag), FALSE, FALSE, 0);
	    gtk_container_add(GTK_CONTAINER(treeItem),
			      GTK_WIDGET(state->start_tag));
	    gtk_tree_append(GTK_TREE(state->tree), treeItem);
	    gtk_widget_show(treeItem);
	    gtk_widget_show(state->start_tag);
	    if (state->body_text != NULL)
		gtk_widget_show(state->body_text);
	    gtk_widget_show(state->end_tag);
	}

    /* have text only */
    } else if (state->body_text != NULL) {
	addTextNode(state);

    /* this should never happen */
    } else {
	g_message("Unhandled case for addNode(): ");
    }

    /* now that the node is added, lets forget its state */
    forgetNode(state);
}

/* creates a new style given a color */
inline GtkStyle *createStyle(GdkColor forecolor)
{
    GtkStyle *style = gtk_widget_get_default_style();
    GtkStyle *newstyle = gtk_style_copy(style);
    int i = 0;
    for (i = 0; i < 5; i++) {
	newstyle->fg[i] = forecolor;
    }
    return newstyle;
}

/* creates a color given 16bit rgb values */
inline GdkColor createColor(int red, int green, int blue)
{
    GdkColormap *cmap;
    GdkColor color;

    cmap = gdk_colormap_get_system();
    color.red = red;
    color.green = green;
    color.blue = blue;

    if (!gdk_colormap_alloc_color(cmap, &color, TRUE, TRUE)) {
	g_error("couldn't allocate color: %d,%d,%d", red, green, blue);
    }

    return color;
}

/* creates a new text container */
inline GtkWidget *createTextContainer(void)
{
    return gtk_hbox_new(FALSE, 0);
}

/* adds colored text to a text container */
inline void addColorText(AppParseState * state, GtkStyle * color, const char *strText)
{
    GtkWidget *textBox = getCurrentTextContainer(state);
    GtkWidget *text = gtk_label_new(strText);
    gtk_widget_set_style(GTK_WIDGET(text), color);
    gtk_box_pack_start(GTK_BOX(textBox), GTK_WIDGET(text), FALSE, FALSE,
		       0);
    gtk_widget_show(text);
}

/* specifically used add error text to the xml tree 
 * sort of hack method that will end up calling 
 * addColorText with the state->tree reassigned to the
 * root tree so that the message gets added to the 
 * end of the root tree.*/
inline void addErrorText(AppParseState * state, char *strText) {
   /* save the current tree reference */
   GtkWidget *oldTree = state->tree;

   /* flush the node */   
   addNode(state);
   setState(state, STATE_TEXT);

   /* reassign the tree to the parent so the the message
    * gets added to the root */
   state->tree = state->parent_tree;

   /* add message as red text */
   addColorText(state, state->style_error, strText);
   addNode(state);

   /* reset the tree */
   state->tree = oldTree;
}

/* given the current parsing state, get the container to put text */
inline GtkWidget *getCurrentTextContainer(AppParseState * state)
{
    GtkWidget *text = NULL;
    switch (state->state) {
    case STATE_STARTTAG:
        if (state->start_tag == NULL) state->start_tag = createTextContainer();
        text = state->start_tag;
	break;
    case STATE_ENDTAG:
        if (state->end_tag == NULL) state->end_tag = createTextContainer();
        text = state->end_tag;
	break;
    case STATE_TEXT:
	if (state->body_text == NULL) state->body_text = createTextContainer();
	text = state->body_text;
	break;
    default:
	g_error("Uknown State: %d! unable to get a text container",
		state->state);
    }
    return text;
}


/* sets the parser state */
inline void setState(AppParseState * state, int stateValue)
{
    state->state = stateValue;
}

/* SAX handlers */
static void handleStartDocument(AppParseState * state)
{
    DEBUG_CMD(g_message("starting to parse xml document"));

    /* initialize the state */
    forgetNode(state);
}

static void handleEndDocument(AppParseState * state)
{
    DEBUG_CMD(g_message("Finished parsing document and building xml tree."));
}

static void handleStartElement(AppParseState * state, char *name, char **attr)
{
    addNode(state);
    setState(state, STATE_STARTTAG);

    addColorText(state, state->style_tag, "<");
    addColorText(state, state->style_tag, name);

    while (attr != NULL && *attr != NULL) {
	addColorText(state, state->style_attr, " ");
	addColorText(state, state->style_attr, *attr);
	addColorText(state, state->style_attr, "=");

	attr++;
	addColorText(state, state->style_quoted_text, "\"");
	addColorText(state, state->style_quoted_text, *attr);
	addColorText(state, state->style_quoted_text, "\"");

	attr++;
    };
    addColorText(state, state->style_tag, ">");
}

static void handleEndElement(AppParseState * state, char *name)
{
    setState(state, STATE_ENDTAG);
    addColorText(state, state->style_tag, "</");
    addColorText(state, state->style_tag, name);
    addColorText(state, state->style_tag, ">");
    addNode(state);
}

static void handleCharacters(AppParseState * state, char *data, int len)
{
    gchar *buf = g_strndup(data, len);
    g_strstrip(buf);
    if (strlen(buf) != 0) {
	setState(state, STATE_TEXT);
	addColorText(state, state->style_text, buf);
	DEBUG_CMD(g_message("Adding text [%s]", buf));
    } else {
	DEBUG_CMD(g_message("String is empty, will not add"));
    }
    g_free(buf);
}

static void handleCdataBlock(AppParseState * state, char *data, int len)
{
   GList *list = NULL;
   GtkObject *treeItem = NULL;
   GtkObject *labelEnd = NULL;

   gchar *buf = g_strndup(data, len);
   //g_strstrip(buf);
   DEBUG_CMD(g_message("adding cdata [%s]", buf));
   addNode(state);
   if (strstr(buf, "\n") != NULL) {
      DEBUG_CMD(g_message("adding a multiline CDATA"));
      setState(state, STATE_STARTTAG);
      addColorText(state, state->style_cdata, "<![CDATA[");
      addColorText(state, state->style_cdata, "    ]]>");

      /* get last box */
      list = gtk_container_children(GTK_CONTAINER(state->start_tag));
      labelEnd = GTK_OBJECT(g_list_last(list)->data);
      g_list_free(list);

      addNode(state);

      /* show or hide last box */
      treeItem = GTK_OBJECT(GTK_TREE(state->tree)->tree_owner);
      gtk_signal_connect_object(treeItem, "collapse", gtk_widget_show, labelEnd);
      gtk_signal_connect_object(treeItem, "expand",   gtk_widget_hide, labelEnd);
 
      setState(state, STATE_TEXT);
      addColorText(state, state->style_text, buf);
      
      setState(state, STATE_ENDTAG);
      addColorText(state, state->style_cdata, "]]>");
   } else {
      DEBUG_CMD(g_message("adding single cdata to text node"));
      setState(state, STATE_TEXT);
      addColorText(state, state->style_cdata, "<![CDATA[");
      addColorText(state, state->style_text, buf);
      addColorText(state, state->style_cdata, "]]>");
   }
   addNode(state);
   g_free(buf);
}

static void handleComment(AppParseState * state, char *data)
{
   GList *list = NULL;
   GtkObject *treeItem = NULL;
   GtkObject *labelEnd = NULL;

   gchar *buf = g_strdup(data);
   buf = g_strstrip(buf);
   DEBUG_CMD(g_message("adding comment [%s]", buf));
   addNode(state);
   if (strstr(buf, "\n") != NULL) {
      DEBUG_CMD(g_message("adding a multiline comment"));
      setState(state, STATE_STARTTAG);
      addColorText(state, state->style_comment, "<!--");
      addColorText(state, state->style_comment, "    -->");

      /* get last box */
      list = gtk_container_children(GTK_CONTAINER(state->start_tag));
      labelEnd = GTK_OBJECT(g_list_last(list)->data);
      g_list_free(list);

      addNode(state);

      /* show or hide last box */
      treeItem = GTK_OBJECT(GTK_TREE(state->tree)->tree_owner);
      gtk_signal_connect_object(treeItem, "collapse", gtk_widget_show, labelEnd);
      gtk_signal_connect_object(treeItem, "expand",   gtk_widget_hide, labelEnd);
           
      setState(state, STATE_TEXT);
      addColorText(state, state->style_comment, buf);
      
      setState(state, STATE_ENDTAG);
      addColorText(state, state->style_comment, "-->");
   } else {
      if (state->body_text != NULL) {
         DEBUG_CMD(g_message("adding single comment to text node"));
         setState(state, STATE_TEXT);
         addColorText(state, state->style_comment, "<!-- ");
         addColorText(state, state->style_comment, buf);
         addColorText(state, state->style_comment, " -->");
      } else {
         DEBUG_CMD(g_message("adding single comment node"));
         addNode(state);
         setState(state, STATE_STARTTAG);
         addColorText(state, state->style_comment, "<!-- ");
         
         setState(state, STATE_TEXT);
         addColorText(state, state->style_comment, buf);
         
         setState(state, STATE_ENDTAG);
         addColorText(state, state->style_comment, " -->");
      }
   }
   addNode(state);
   g_free(buf);
}

static void handleDTD(AppParseState * state, char *name, char *publicId,
	   char *systemId)
{

    setState(state, STATE_TEXT);

    addColorText(state, state->style_tag, "<!DOCTYPE ");
    addColorText(state, state->style_attr, name);
    if (publicId != NULL) {
        addColorText(state, state->style_attr, " PUBLIC ");
        addColorText(state, state->style_quoted_text, "\"");
        addColorText(state, state->style_quoted_text, publicId);
        addColorText(state, state->style_quoted_text, "\"");
    }
    if (systemId != NULL) {
        addColorText(state, state->style_attr, " SYSTEM ");
        addColorText(state, state->style_quoted_text, "\"");
        addColorText(state, state->style_quoted_text, systemId);
        addColorText(state, state->style_quoted_text, "\"");
    }
    addColorText(state, state->style_tag, ">");
    addNode(state);
}

void gxmlviewer_init(AppParseState *state, GtkTree *parent) {
   DEBUG_CMD(g_message("gxmlviewer is initializing...."));

   state->tree = GTK_WIDGET(parent);
   state->parent_tree = GTK_WIDGET(parent);

   /* load the styles for the colors... 
    * someday we may even load this from a file??
    * */
   state->style_tag = createStyle(createColor(0, 0, 35584));
   state->style_attr = createStyle(createColor(13824, 25600, 35584));
   state->style_quoted_text = createStyle(createColor(35584, 18176, 23808));
   state->style_text = createStyle(createColor(0, 0, 0));
   state->style_cdata = createStyle(createColor(26880, 26880, 26880));
   state->style_comment = createStyle(createColor(0, 25600, 0));
   state->style_error = createStyle(createColor(0xffff, 0, 0));

   /* initialize the state */
   forgetNode(state);
}

void show_xmlheader(AppParseState *state, const xmlParserCtxtPtr ctxt)
{
  char buf[1024];
  GtkWidget *treeItem = NULL;
  *buf = 0;
 
  setState(state, STATE_TEXT);

  addColorText(state, state->style_tag, "<?xml");

  if (ctxt->version != NULL)
    {
      addColorText(state, state->style_attr, " version=");
      addColorText(state, state->style_quoted_text, "\"");
      addColorText(state, state->style_quoted_text, ctxt->version);
      addColorText(state, state->style_quoted_text, "\"");
    }
  if (ctxt->input->encoding != NULL)
    {
      addColorText(state, state->style_attr, " encoding=");
      addColorText(state, state->style_quoted_text, "\"");
      addColorText(state, state->style_quoted_text, ctxt->input->encoding);
      addColorText(state, state->style_quoted_text, "\"");
    }
  addColorText(state, state->style_tag, " ?>");
 
  treeItem = gtk_tree_item_new();
  gtk_tree_prepend(GTK_TREE(state->tree), treeItem);
  gtk_container_add(GTK_CONTAINER(treeItem), state->body_text);
  gtk_widget_show(state->body_text);
  gtk_widget_show(treeItem);
  state->body_text = NULL;
}

/* show the parsed xml file in a tree */
int show_xmlfile(const char *filename, GtkWidget * tree)
{
    xmlParserCtxtPtr ctxt;
    AppParseState state;
    const char *fileName = filename;
    int errNo = 0;
    char *strBuf = NULL;
    
    /* initialize gxmlviewer state */
    gxmlviewer_init(&state, GTK_TREE(tree));

    ctxt = xmlCreateFileParserCtxt(fileName);
    if (!ctxt) {
	strBuf = g_strdup_printf("Unable to parse xml file: [%s]. The filename may not exist.", fileName);
	errNo = 1;
    }

    if (errNo == 0) {
	ctxt->sax = &appSAXParser;
	ctxt->userData = &state;
	ctxt->validate = 1;
	ctxt->replaceEntities = 0;
	xmlParseDocument(ctxt);
	if (!ctxt->wellFormed) {
	    strBuf = g_strdup_printf("Parse error in XML file: [%s]",fileName);
	    errNo = 2;
	}

	show_xmlheader(&state, ctxt);

	ctxt->sax = NULL;
	xmlFreeParserCtxt(ctxt);
    }

    /* if we have errors, the lets show them */
    if (errNo != 0) {
       setState(&state, STATE_TEXT);
       addErrorText(&state, strBuf);
       addNode(&state);
    }

    /* free the string buffer */
    if (strBuf != NULL)
	g_free(strBuf);
    return errNo;
}
