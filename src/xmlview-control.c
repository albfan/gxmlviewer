#include "xmlview-control.h"
#include <parser.h>
#include <parserInternals.h>

#include "xmlparser.h"

#define READ_CHUNK_SIZE 4096

static xmlSAXHandler appSAXParser = {
   0, /* internalSubset */
   0, /* isStandalone */
   0, /* hasInternalSubset */
   0, /* hasExternalSubset */
   0, /* resolveEntity */
   (getEntitySAXFunc)NULL, /* getEntity */
   0, /* entityDecl */
   0, /* notationDecl */
   0, /* attributeDecl */
   0, /* elementDecl */
   0, /* unparsedEntityDecl */
   0, /* setDocumentLocator */
   (startDocumentSAXFunc)startDocument, /* startDocument */
   (endDocumentSAXFunc)endDocument, /* endDocument */
   (startElementSAXFunc)handleStartElement, /* startElement */
   (endElementSAXFunc)handleEndElement, /* endElement */
   0, /* reference */
   (charactersSAXFunc)handleCharacters, /* characters */
   0, /* ignorableWhitespace */
   0, /* processingInstruction */
   (commentSAXFunc)NULL, /* comment */
   (warningSAXFunc)NULL, /* warning */
   (errorSAXFunc)NULL, /* error */
   (fatalErrorSAXFunc)NULL, /* fatalError */
};

static gint refcount = 0;

static void xmlview_ps_load (BonoboPersistStream * ps,
		       Bonobo_Stream stream,
		       Bonobo_Persist_ContentType type,
		       GtkWidget * tree,
		       CORBA_Environment * ev)
{
  Bonobo_Stream_iobuf * buffer;

  if (strcmp (type, "text/xml") != 0) {
    CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			 ex_Bonobo_Persist_WrongDataType, NULL);
    return;
  }
  
  do {
    Bonobo_Stream_read (stream, READ_CHUNK_SIZE,
			&buffer, ev);
    if (ev->_major != CORBA_NO_EXCEPTION)
      break;
    if (buffer->_length <= 0)
      break;
    printf("%s\n", buffer->_buffer);
    {
      xmlParserCtxtPtr ctxt;
      AppParseState state;
      gchar * strBuf;
      int errNo = 0;

      /* Set the default tree state: No Lines */
      gtk_tree_set_view_mode (GTK_TREE (tree), GTK_TREE_VIEW_ITEM);
      gtk_tree_set_view_lines (GTK_TREE (tree), FALSE);

      /* set the tree in the parser state */
      state.tree = tree;
      ctxt = xmlCreateMemoryParserCtxt(buffer->_buffer,
				       buffer->_length);
      if (!ctxt) {
	strBuf = g_strdup(_("Unable to parse XML."));
	errNo = 1;
      }
      if (errNo == 0) {
	ctxt->sax = &appSAXParser;
	ctxt->userData = &state;
	xmlParseDocument(ctxt);
	if (!ctxt->wellFormed) {
	  strBuf = g_strdup(_("XML not formatted well."));
	  errNo = 2;
	}
	ctxt->sax = NULL;
	xmlFreeParserCtxt(ctxt);
      }

      /* if we have errors, the lets show them */
      if (errNo != 0) {
	GtkWidget *treeItem;
	treeItem = gtk_tree_item_new_with_label((strBuf != NULL) ? strBuf : "No Message");
	gtk_tree_append(GTK_TREE(tree), treeItem);
	gtk_widget_show(treeItem);
      }

      /* free the string buffer */
      if (strBuf != NULL)
	g_free(strBuf);
    }
    CORBA_free (buffer);
  } while (1);
  
  if (ev->_major != CORBA_NO_EXCEPTION) {
  } else {
    CORBA_free (buffer);
  }
}

static Bonobo_Persist_ContentTypeList *
xmlview_ps_types (BonoboPersistStream * ps,
		   gpointer data,
		   CORBA_Environment * ev)
{
	return bonobo_persist_generate_content_types (1, "text/xml");
}

static void
control_destroy_cb (BonoboControl * control, gpointer data)
{
	bonobo_object_unref (BONOBO_OBJECT (data));
	if (--refcount < 1)
		gtk_main_quit ();
}

static BonoboObject *
xmlview_factory (BonoboGenericFactory * factory, void * closure)
{
	BonoboControl * control;
	BonoboPersistStream * stream;
	GtkWidget * mainWin, * viewport1, * xmltree;

#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	textdomain (PACKAGE);
#endif

	mainWin = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_ref (mainWin);
	gtk_object_set_data_full (GTK_OBJECT (mainWin), "scrolledwindow1",
				  mainWin,
				  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (mainWin);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (mainWin), 
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	viewport1 = gtk_viewport_new (NULL, NULL);
	gtk_widget_ref (viewport1);
	gtk_object_set_data_full (GTK_OBJECT (mainWin), "viewport1",
				  viewport1,
				  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (viewport1);
	gtk_container_add (GTK_CONTAINER (mainWin), viewport1);
	
	xmltree = gtk_tree_new ();
	gtk_widget_ref (xmltree);
	gtk_object_set_data_full (GTK_OBJECT (mainWin), "xmltree", xmltree,
				  (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (xmltree);
	gtk_container_add (GTK_CONTAINER (viewport1), xmltree);
	gtk_tree_set_view_mode (GTK_TREE (xmltree), GTK_TREE_VIEW_ITEM);
	gtk_tree_set_view_lines (GTK_TREE (xmltree), FALSE);

	/* show the main window */ 
	gtk_widget_show (mainWin);

	control = bonobo_control_new (mainWin);

	bonobo_control_set_automerge (control, TRUE);

	if (!control) {
		gtk_object_destroy (GTK_OBJECT (mainWin));
		return NULL;
	}

	stream = bonobo_persist_stream_new (xmlview_ps_load, NULL, 
					    NULL, xmlview_ps_types,
					    xmltree);

	bonobo_object_add_interface (BONOBO_OBJECT (control),
				     BONOBO_OBJECT (stream));

	gtk_signal_connect (GTK_OBJECT (mainWin), "destroy",
			    control_destroy_cb, control);

	refcount++;

	return BONOBO_OBJECT (control);
}

static gboolean
xmlview_factory_init (void)
{
  static BonoboGenericFactory * xmlfact = NULL;

  if (!xmlfact) {
    xmlfact = bonobo_generic_factory_new ("OAFIID:GNOME_XMLView_Factory",
					 xmlview_factory, NULL);
    if (!xmlfact)
      g_error ("Cannot create xmlview factory");
  }
  return FALSE;
}

int main (int argc, gchar ** argv)
{
	CORBA_Environment ev;
	CORBA_ORB orb;

	CORBA_exception_init (&ev);

	gnome_init_with_popt_table ("GNOMEXMLView", VERSION,
				    argc, argv,
				    oaf_popt_options, 0, NULL);

	orb = oaf_init (argc, argv);

	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error ("Couldn't initialize Bonobo");

	gtk_idle_add ((GtkFunction) xmlview_factory_init, NULL);

	bonobo_main ();

	return 0;
}
