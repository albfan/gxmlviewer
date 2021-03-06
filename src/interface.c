/*
 * $Id: interface.c,v 1.8 2001/11/29 01:28:46 sean_stuckless Exp $ 
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>

#ifdef NEED_GNOMESUPPORT_H
#include <gnome.h>
#else
#include <gtk/gtk.h>
#endif

#include "xmlwindow.h"
#include "callbacks.h"
#include "interface.h"
#include "support.h"

#ifdef NEED_GNOMESUPPORT_H
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
#else
static GtkItemFactoryEntry menu_items[] = {
  { "/_File",           NULL,         NULL, 0, "<Branch>" },
  { "/File/_Open...", "<control>O", on_open1_activate, 0, NULL },
  { "/File/---",        NULL,         NULL, 0, "<Separator>" },
  { "/File/_Quit",   "<control>Q", gtk_main_quit,  0, NULL },

  { "/_Help",              NULL,         NULL, 0, "<Branch>"/*"<LastBranch>" */},
  { "/_Help/_About...", NULL,         on_about1_activate,    0, NULL }
};
#endif

GtkWidget*
create_mainWin (void)
{
  GtkWidget *mainWin;
  GtkWidget *menuContainer;
  GtkWidget *menubar1;
  GtkWidget *scrolledwindow1;
  GtkWidget *vbox1;

#ifndef NEED_GNOMESUPPORT_H
  int nmenu_items;
  GtkItemFactory *item_factory;
  GtkAccelGroup *accel_group;
#endif

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

#ifdef NEED_GNOMESUPPORT_H
  menubar1 = gtk_menu_bar_new ();

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

#else /* NEED_GNOMESUPPORT_H */

  nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);
  accel_group = gtk_accel_group_new ();

  item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", 
				       accel_group);
  gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, NULL);

  gtk_window_add_accel_group (GTK_WINDOW (mainWin), accel_group);

  menubar1 = gtk_item_factory_get_widget (item_factory, "<main>");

#endif /* NEED_GNOMESUPPORT_H */

  gtk_widget_ref (menubar1);
  gtk_object_set_data_full (GTK_OBJECT (mainWin), "menubar1", menubar1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (menubar1);
  gtk_container_add (GTK_CONTAINER (menuContainer), menubar1);

  scrolledwindow1 = xmlwindow_new(mainWin);
  gtk_box_pack_start (GTK_BOX (vbox1), scrolledwindow1, TRUE, TRUE, 0);

  gtk_signal_connect (GTK_OBJECT (mainWin), "delete_event",
                      GTK_SIGNAL_FUNC (gtk_main_quit),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (mainWin), "destroy_event",
                      GTK_SIGNAL_FUNC (gtk_main_quit),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (mainWin), "drag_data_received",
                      GTK_SIGNAL_FUNC (on_mainWin_drag_data_received),
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
                      GTK_SIGNAL_FUNC (on_file_select_cancel_button_clicked),
                      NULL);

  return fileselection;
}

GtkWidget*
create_aboutBox (void)
{
  GtkWidget *aboutBox;
#ifdef NEED_GNOMESUPPORT_H
  const gchar *authors[] = {
    "Sean Stuckless <sean@stuckless.org>",
    NULL
  };

  aboutBox = gnome_about_new ("gxmlviewer", VERSION,
                        "",
                        authors,
                        _("xmlviewer for gnome."),
                        NULL);
#else /* NEED_GNOMESUPPORT_H */

  GtkWidget *dialog_vbox;
  GtkWidget *fixed;
  GtkWidget *label;

  GtkWidget *dialog_action_area;
  GtkWidget *button;

  aboutBox = gtk_dialog_new();
  gtk_window_set_title (GTK_WINDOW (aboutBox), _("About"));

  gtk_window_set_policy (GTK_WINDOW (aboutBox), FALSE, FALSE, FALSE);


  dialog_vbox = GTK_DIALOG (aboutBox)->vbox;
  gtk_object_set_data (GTK_OBJECT (aboutBox), "dialog_vbox", dialog_vbox);
  gtk_widget_show (dialog_vbox);

  fixed = gtk_fixed_new ();
  gtk_widget_ref (fixed);
  gtk_object_set_data_full (GTK_OBJECT (aboutBox), "fixed", fixed,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fixed);
  gtk_box_pack_start (GTK_BOX (dialog_vbox), fixed, TRUE, TRUE, 0);

  label = gtk_label_new (_("gxmlviewer\nAuthor : Sean Stuckless <sean@stuckless.org>"));
  gtk_widget_ref (label);
  gtk_object_set_data_full (GTK_OBJECT (aboutBox), "label", label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label);

  gtk_fixed_put (GTK_FIXED (fixed), label, 112, 40);
  gtk_widget_set_uposition (label, 112, 40);
  //gtk_widget_set_usize (label, 200, 112);


  dialog_action_area = GTK_DIALOG (aboutBox)->action_area;
  gtk_object_set_data (GTK_OBJECT (aboutBox), "dialog_action_area", dialog_action_area);
  gtk_widget_show (dialog_action_area);
  gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area), 10);


  button = gtk_button_new_with_label (_("Ok"));
 gtk_widget_set_usize (button,40, 25);
  gtk_widget_ref (button);
  gtk_object_set_data_full (GTK_OBJECT (aboutBox), "button", button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (dialog_action_area), button, FALSE, FALSE, 0);

  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
                             GTK_SIGNAL_FUNC (gtk_widget_destroy),
                             GTK_OBJECT (aboutBox));



  gtk_widget_show(GTK_WIDGET(aboutBox));
#endif /* NEED_GNOMESUPPORT_H */
  gtk_object_set_data (GTK_OBJECT (aboutBox), "aboutBox", aboutBox);
  gtk_window_set_modal (GTK_WINDOW (aboutBox), TRUE);

  return aboutBox;
}

