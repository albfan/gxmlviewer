/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <gnome.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

static GnomeUIInfo file1_menu_uiinfo[] =
{
  GNOMEUIINFO_MENU_OPEN_ITEM (on_open1_activate, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_EXIT_ITEM (gtk_main_quit, NULL),
  GNOMEUIINFO_END
};

static GnomeUIInfo help1_menu_uiinfo[] =
{
  GNOMEUIINFO_MENU_ABOUT_ITEM (on_about1_activate, NULL),
  GNOMEUIINFO_END
};

static GnomeUIInfo menubar1_uiinfo[] =
{
  GNOMEUIINFO_MENU_FILE_TREE (file1_menu_uiinfo),
  GNOMEUIINFO_MENU_HELP_TREE (help1_menu_uiinfo),
  GNOMEUIINFO_END
};

GtkWidget*
create_mainWin (void)
{
  GtkWidget *mainWin;
  GtkWidget *vbox1;
  GtkWidget *menuContainer;
  GtkWidget *menubar1;
  GtkWidget *scrolledwindow1;
  GtkWidget *viewport1;
  GtkWidget *xmltree;

  mainWin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (mainWin), "mainWin", mainWin);
  gtk_window_set_title (GTK_WINDOW (mainWin), _("XML Viewer"));
  gtk_window_set_default_size (GTK_WINDOW (mainWin), 640, 480);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox1);
  gtk_object_set_data_full (GTK_OBJECT (mainWin), "vbox1", vbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (mainWin), vbox1);

  menuContainer = gtk_handle_box_new ();
  gtk_widget_ref (menuContainer);
  gtk_object_set_data_full (GTK_OBJECT (mainWin), "menuContainer", menuContainer,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (menuContainer);
  gtk_box_pack_start (GTK_BOX (vbox1), menuContainer, FALSE, FALSE, 0);

  menubar1 = gtk_menu_bar_new ();
  gtk_widget_ref (menubar1);
  gtk_object_set_data_full (GTK_OBJECT (mainWin), "menubar1", menubar1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (menubar1);
  gtk_container_add (GTK_CONTAINER (menuContainer), menubar1);
  gnome_app_fill_menu (GTK_MENU_SHELL (menubar1), menubar1_uiinfo,
                       NULL, FALSE, 0);

  gtk_widget_ref (menubar1_uiinfo[0].widget);
  gtk_object_set_data_full (GTK_OBJECT (mainWin), "file1",
                            menubar1_uiinfo[0].widget,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_ref (file1_menu_uiinfo[0].widget);
  gtk_object_set_data_full (GTK_OBJECT (mainWin), "open1",
                            file1_menu_uiinfo[0].widget,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_ref (file1_menu_uiinfo[1].widget);
  gtk_object_set_data_full (GTK_OBJECT (mainWin), "separator1",
                            file1_menu_uiinfo[1].widget,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_ref (file1_menu_uiinfo[2].widget);
  gtk_object_set_data_full (GTK_OBJECT (mainWin), "exit1",
                            file1_menu_uiinfo[2].widget,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_ref (menubar1_uiinfo[1].widget);
  gtk_object_set_data_full (GTK_OBJECT (mainWin), "help1",
                            menubar1_uiinfo[1].widget,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_ref (help1_menu_uiinfo[0].widget);
  gtk_object_set_data_full (GTK_OBJECT (mainWin), "about1",
                            help1_menu_uiinfo[0].widget,
                            (GtkDestroyNotify) gtk_widget_unref);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_ref (scrolledwindow1);
  gtk_object_set_data_full (GTK_OBJECT (mainWin), "scrolledwindow1", scrolledwindow1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow1);
  gtk_box_pack_start (GTK_BOX (vbox1), scrolledwindow1, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  viewport1 = gtk_viewport_new (NULL, NULL);
  gtk_widget_ref (viewport1);
  gtk_object_set_data_full (GTK_OBJECT (mainWin), "viewport1", viewport1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (viewport1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), viewport1);

  xmltree = gtk_tree_new ();
  gtk_widget_ref (xmltree);
  gtk_object_set_data_full (GTK_OBJECT (mainWin), "xmltree", xmltree,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (xmltree);
  gtk_container_add (GTK_CONTAINER (viewport1), xmltree);
  gtk_tree_set_view_mode (GTK_TREE (xmltree), GTK_TREE_VIEW_ITEM);
  gtk_tree_set_view_lines (GTK_TREE (xmltree), FALSE);

  gtk_signal_connect (GTK_OBJECT (mainWin), "delete_event",
                      GTK_SIGNAL_FUNC (gtk_main_quit),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (mainWin), "destroy_event",
                      GTK_SIGNAL_FUNC (gtk_main_quit),
                      NULL);

  return mainWin;
}

GtkWidget*
create_fileselection (void)
{
  GtkWidget *fileselection;
  GtkWidget *ok_button1;
  GtkWidget *cancel_button1;

  fileselection = gtk_file_selection_new (_("Select File"));
  gtk_object_set_data (GTK_OBJECT (fileselection), "fileselection", fileselection);
  gtk_container_set_border_width (GTK_CONTAINER (fileselection), 10);

  ok_button1 = GTK_FILE_SELECTION (fileselection)->ok_button;
  gtk_object_set_data (GTK_OBJECT (fileselection), "ok_button1", ok_button1);
  gtk_widget_show (ok_button1);
  GTK_WIDGET_SET_FLAGS (ok_button1, GTK_CAN_DEFAULT);

  cancel_button1 = GTK_FILE_SELECTION (fileselection)->cancel_button;
  gtk_object_set_data (GTK_OBJECT (fileselection), "cancel_button1", cancel_button1);
  gtk_widget_show (cancel_button1);
  GTK_WIDGET_SET_FLAGS (cancel_button1, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (ok_button1), "clicked",
                      GTK_SIGNAL_FUNC (on_file_select_button_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (cancel_button1), "clicked",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy),
                      NULL);

  return fileselection;
}

GtkWidget*
create_aboutBox (void)
{
  const gchar *authors[] = {
    "Sean Stuckless",
    NULL
  };
  GtkWidget *aboutBox;

  aboutBox = gnome_about_new ("gXMLviewer", VERSION,
                        "",
                        authors,
                        _("xmlviewer for gnome."),
                        NULL);
  gtk_object_set_data (GTK_OBJECT (aboutBox), "aboutBox", aboutBox);
  gtk_window_set_modal (GTK_WINDOW (aboutBox), TRUE);

  return aboutBox;
}

