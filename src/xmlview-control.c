/*
* $Id: xmlview-control.c,v 1.3 2001/08/12 16:58:07 sean_stuckless Exp $
*/

#include "xmlview-control.h"
#include <parser.h>
#include <parserInternals.h>

#include "xmlparser.h"
#include "xmlwindow.h"
#include "constants.h"
#include "support.h"

#include <stdio.h>
#include <unistd.h>

#define READ_CHUNK_SIZE 4096

static gint refcount = 0;

static char *RCSID="$Id: xmlview-control.c,v 1.3 2001/08/12 16:58:07 sean_stuckless Exp $";

static void
xmlview_ps_load(BonoboPersistStream * ps,
		Bonobo_Stream stream,
		Bonobo_Persist_ContentType type,
		GtkWidget * tree, CORBA_Environment * ev)
{
    Bonobo_Stream_iobuf *buffer;
    char tmpfile[256] = "";
    FILE *xmlfile = NULL;

    if (strcmp(type, "text/xml") != 0) {
	CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			    ex_Bonobo_Persist_WrongDataType, NULL);
	return;
    }

    tmpnam(tmpfile);
    xmlfile = fopen(tmpfile, "w");
    if (xmlfile == NULL) {
	return;
    }

    do {
	Bonobo_Stream_read(stream, READ_CHUNK_SIZE, &buffer, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
	    break;
	if (buffer->_length <= 0)
	    break;

	fwrite(buffer->_buffer, buffer->_length, sizeof(char), xmlfile);

	CORBA_free(buffer);
    }
    while (1);

    fflush(xmlfile);
    fclose(xmlfile);

    if (ev->_major != CORBA_NO_EXCEPTION) {
    } else {
	CORBA_free(buffer);
    }

    show_xmlfile((const char*)tmpfile, tree);

    unlink(tmpfile);
}

static Bonobo_Persist_ContentTypeList *xmlview_ps_types(BonoboPersistStream
							* ps,
							gpointer data,
							CORBA_Environment *
							ev)
{
    return bonobo_persist_generate_content_types(1, "text/xml");
}

static void control_destroy_cb(BonoboControl * control, gpointer data)
{
    bonobo_object_unref(BONOBO_OBJECT(data));
    if (--refcount < 1)
	gtk_main_quit();
}

static BonoboObject *xmlview_factory(BonoboGenericFactory * factory,
				     void *closure)
{
    BonoboControl *control;
    BonoboPersistStream *stream;
    GtkWidget *mainWin;

#ifdef ENABLE_NLS
    bindtextdomain(PACKAGE, PACKAGE_LOCALE_DIR);
    textdomain(PACKAGE);
#endif

    mainWin = xmlwindow_new(NULL);

    /* show the main window */
    gtk_widget_show(mainWin);

    control = bonobo_control_new(mainWin);

    bonobo_control_set_automerge(control, TRUE);

    if (!control) {
	gtk_object_destroy(GTK_OBJECT(mainWin));
	return NULL;
    }

    stream = bonobo_persist_stream_new(xmlview_ps_load, NULL,
				       NULL, xmlview_ps_types,
				       lookup_widget(mainWin,
						     XMLTREE_WIDGET_NAME));

    bonobo_object_add_interface(BONOBO_OBJECT(control),
				BONOBO_OBJECT(stream));

    gtk_signal_connect(GTK_OBJECT(mainWin), "destroy",
		       control_destroy_cb, control);

    refcount++;

    return BONOBO_OBJECT(control);
}

static gboolean xmlview_factory_init(void)
{
    static BonoboGenericFactory *xmlfact = NULL;

    if (!xmlfact) {
	xmlfact =
	    bonobo_generic_factory_new("OAFIID:GNOME_XMLView_Factory",
				       xmlview_factory, NULL);
	if (!xmlfact)
	    g_error("Cannot create xmlview factory");
    }
    return FALSE;
}

int main(int argc, gchar ** argv)
{
    CORBA_Environment ev;
    CORBA_ORB orb;

    CORBA_exception_init(&ev);

    gnome_init_with_popt_table("GNOMEXMLView", VERSION,
			       argc, argv, oaf_popt_options, 0, NULL);

    orb = oaf_init(argc, argv);

    if (bonobo_init(orb, NULL, NULL) == FALSE)
	g_error("Couldn't initialize Bonobo");

    gtk_idle_add((GtkFunction) xmlview_factory_init, NULL);

    bonobo_main();

    return 0;
}
