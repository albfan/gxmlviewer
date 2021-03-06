/*
 * $Id: callbacks.h,v 1.5 2001/11/20 02:02:54 sean_stuckless Exp $
 */

#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_

#include <gtk/gtk.h>


void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_file_select_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_file_select_cancel_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);


void
on_mainWin_drag_data_received          (GtkWidget       *widget,
                                        GdkDragContext  *drag_context,
                                        gint             x,
                                        gint             y,
                                        GtkSelectionData *data,
                                        guint            info,
                                        guint            time,
                                        gpointer         user_data);

#endif
