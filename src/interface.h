/*
 * $Id: interface.h,v 1.4 2001/11/20 02:02:54 sean_stuckless Exp $
 */

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <gtk/gtk.h>

GtkWidget* create_mainWin (void);
GtkWidget* create_fileselection (void);
GtkWidget* create_aboutBox (void);
GtkWidget *xmlwindow_new(GtkWidget *parent);

#endif
