/*
 * $Id: callbacks.c,v 1.6 2001/11/29 01:28:46 sean_stuckless Exp $
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <string.h>

#include "interface.h"
#include "support.h"
#include "xmlparser.h"
#include "constants.h"
#include "callbacks.h"

/*
 * used for file open dialog box 
 */
void on_open1_activate(GtkMenuItem * menuitem, gpointer user_data)
{
    GtkWidget *fileDialog = NULL;
    fileDialog = create_fileselection();
    gtk_widget_show(fileDialog);
}

void
on_file_select_cancel_button_clicked(GtkButton * button,
                                     gpointer user_data)
{
    GtkWidget *fileDialog =
        lookup_widget(GTK_WIDGET(button), FILE_SELECTION);
    gtk_widget_destroy(fileDialog);
}

/*
 * when the ok button is clicked in the file open dialog 
 */
void on_file_select_button_clicked(GtkButton * button, gpointer user_data)
{
    GtkWidget *fileDialog =
        lookup_widget(GTK_WIDGET(button), FILE_SELECTION);
    if (fileDialog != NULL) {
        const gchar *filename =
            gtk_file_selection_get_filename(GTK_FILE_SELECTION
                                            (fileDialog));
        if (filename == NULL) {
            g_message("file is null!!");
        } else {
            GtkWidget *xmltree = lookup_widget(GTK_WIDGET(gMainWindow),
                                               XMLTREE_WIDGET_NAME);
            if (xmltree != NULL) {
                gtk_tree_clear_items(GTK_TREE(xmltree), 0, 99);
                if (show_xmlfile(filename, xmltree)) {
                    /*
                     * unable to process file 
                     */
                }
            } else {
                g_message("xmltree is null!!");
            }
        }
    }
    gtk_widget_destroy(fileDialog);
}

/*
 * about box 
 */
void on_about1_activate(GtkMenuItem * menuitem, gpointer user_data)
{
    GtkWidget *aboutBox = create_aboutBox();
    gtk_widget_show(aboutBox);
}



void
on_mainWin_drag_data_received(GtkWidget * widget,
                              GdkDragContext * drag_context,
                              gint x,
                              gint y,
                              GtkSelectionData * data,
                              guint info, guint time, gpointer user_data)
{
    GtkWidget *xmltree =
        lookup_widget(GTK_WIDGET(gMainWindow), XMLTREE_WIDGET_NAME);
    char *filename = data->data;
    char *fn = strchr(filename, ':');
    char *end = strchr(fn, '\r');
    if (fn != NULL) {
        fn++;
        *end = 0;
    }

    // now lets check for some possible ways that dnd can be passed...
    // file://hostname/path/file  (Rox default XDND compliant)
    // file:///path/file          (Rox with hostnames turned off)
    // file:/path/file            (Nautilus)
    g_message("Dragged Filename: [%s]\n", filename);
    if (strncmp(fn,"///",3) == 0) {
       g_message("Detected XDND style dnd without hostname");
       fn++;fn++;
    } else if (strncmp(fn, "//",2) == 0) {
       g_message("Detected XDND style dnd with hostname");
       fn++;fn++;
       fn = strchr(fn, '/');
    } else {
       g_message("Detected Nautilus style dnd.");
    }
    g_message("Parsed  Filename: [%s]\n", fn);

    if ((data->length >= 0) && (data->format == 8)) {
        gtk_tree_clear_items(GTK_TREE(xmltree), 0, 99);
        show_xmlfile(fn, xmltree);
        gtk_drag_finish(drag_context, TRUE, FALSE, time);
        return;
    }
}
