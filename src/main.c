/*
 * $Id: main.c,v 1.4 2001/11/20 02:02:54 sean_stuckless Exp $
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef NEED_GNOMESUPPORT_H
#include <gnome.h>
#else
#include <gtk/gtk.h>
#include <string.h>
#endif

#include "interface.h"
#include "support.h"
#include "xmlparser.h"
#include "constants.h"
#include "gxmlviewer.h"

/* this is required so that we can always access the window */
GtkWidget *gMainWindow;

int
main (int argc, char *argv[])
{
  GtkWidget *mainWin;
  char *filename = argv[1];
 
  GtkTargetEntry target_entry[] = {
		{"text/uri-list", 0, 1},
	};

#ifdef ENABLE_NLS
  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);
#endif

#ifdef NEED_GNOMESUPPORT_H
  gnome_init ("gxmlviewer", VERSION, argc, argv);
#else
  gtk_init(&argc, &argv);
#endif

  /* create the main display */
  mainWin = create_mainWin ();
  gtk_window_set_title(GTK_WINDOW(mainWin), GXMLVIEWER_TITLE);
  gMainWindow = mainWin;

  /* dnd support */

  gtk_drag_dest_set(
                mainWin,
                GTK_DEST_DEFAULT_ALL,
                target_entry,sizeof(target_entry) / sizeof(*target_entry),
                GDK_ACTION_COPY | GDK_ACTION_MOVE   
        			 );

  /* check for swallowed parameter. if it exists then remove the menu */
  if (argc > 1 && strcmp(argv[1],"swallowed") == 0) {
	  GtkWidget *menu = lookup_widget(mainWin, MENU_CONTAINER);
	  gtk_widget_hide(menu);
	  filename = argv[2];
  }

  /* show the xml file */
  if (filename != NULL && show_xmlfile(filename, lookup_widget(mainWin, XMLTREE_WIDGET_NAME))) {
     /* there were errors */
  }
 
  /* show the main window */ 
  gtk_widget_show (mainWin);

  /* gtk main event loop handler */
  gtk_main ();
  return 0;
}

