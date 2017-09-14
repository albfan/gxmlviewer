/*
 * $Id: xmlwindow.c,v 1.4 2001/11/29 01:28:46 sean_stuckless Exp $
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

#include <gtk/gtk.h>

#include "constants.h"
#include "xmlwindow.h"
#include "support.h"

/**
* Creates the base scrollable xml window
* if parent is not null, then the window will be added to the parent
*/
GtkWidget *xmlwindow_new(GtkWidget *parent) {
  GtkWidget *scrolledwindow1 = NULL;
  GtkWidget *viewport1;
  GtkWidget *xmltree;
 
  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  g_object_ref (scrolledwindow1);
  if (parent == NULL) parent = scrolledwindow1;

  //gtk_object_set_data_full (GTK_OBJECT (parent), "scrolledwindow1", scrolledwindow1,
  //                          (GtkDestroyNotify) gtk_object_unref);
  gtk_widget_show (scrolledwindow1);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  viewport1 = gtk_viewport_new (NULL, NULL);
  g_object_ref (viewport1);
  //gtk_object_set_data_full (GTK_OBJECT (parent), "viewport1", viewport1,
  //                          (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (viewport1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), viewport1);

  //xmltree = gtk_tree_new ();
  //g_object_ref (xmltree);
  //gtk_object_set_data_full (GTK_OBJECT (parent), "xmltree", xmltree,
  //                          (GtkDestroyNotify) gtk_widget_unref);
  //gtk_widget_show (xmltree);
  //gtk_container_add (GTK_CONTAINER (viewport1), xmltree);
  //gtk_tree_set_view_mode (GTK_TREE (xmltree), GTK_TREE_VIEW_ITEM);
  //gtk_tree_set_view_lines (GTK_TREE (xmltree), FALSE);

  return scrolledwindow1;
}

