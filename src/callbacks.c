#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "xmlparser.h"
#include "constants.h"


/* used for file open dialog box */
void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   GtkWidget *fileDialog = NULL;
   fileDialog = create_fileselection();
   gtk_widget_show(fileDialog);
}

void
on_file_select_cancel_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
   GtkWidget *fileDialog = lookup_widget(GTK_WIDGET(button), FILE_SELECTION);
   gtk_widget_destroy(fileDialog);
}

/* when the ok button is clicked in the file open dialog */
void
on_file_select_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
   GtkWidget *fileDialog = lookup_widget(GTK_WIDGET(button), FILE_SELECTION);
   if (fileDialog != NULL) {
      const gchar *filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(fileDialog));
      if (filename == NULL) {
         printf("file is null!!\n");
      } else {
         GtkWidget *xmltree = lookup_widget(GTK_WIDGET(gMainWindow), XMLTREE_WIDGET_NAME);
         if (xmltree != NULL) {
            gtk_tree_clear_items(GTK_TREE(xmltree), 0, 2);
            if (show_xmlfile(filename, xmltree)) {
		    /* unable to process file */
            }
         }
         else {
            printf("xmltree is null!!\n");
         }
      }
   }
   gtk_widget_destroy(fileDialog);
}

/* about box */
void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
   GtkWidget *aboutBox = create_aboutBox();
   gtk_widget_show(aboutBox);
}
