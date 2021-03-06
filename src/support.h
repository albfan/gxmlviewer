/*
 * $Id: support.h,v 1.3 2001/11/20 02:02:54 sean_stuckless Exp $
 */

#ifndef _SUPPORT_H_
#define _SUPPORT_H_

#ifdef NEED_GNOMESUPPORT_H
#include <gnome.h>
#else
#include <gtk/gtk.h>

/*
 * Standard gettext macros.
 */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif
#endif

/*
 * Public Functions.
 */

/* If debug is enabled then a ton of output will be created */
/* #define DEBUG true */
#ifdef DEBUG
#  define DEBUG_CMD(cmd) cmd
#else
#  define DEBUG_CMD(cmd)
#endif

/*
 * This function returns a widget in a component created by Glade.
 * Call it with the toplevel widget in the component (i.e. a window/dialog),
 * or alternatively any widget in the component, and the name of the widget
 * you want returned.
 */
GtkWidget*  lookup_widget              (GtkWidget       *widget,
                                        const gchar     *widget_name);

/* get_widget() is deprecated. Use lookup_widget instead. */
#define get_widget lookup_widget

#ifdef NEED_GNOMESUPPORT_H

/*
 * Private Functions.
 */

/* This is used to create the pixmaps in the interface. */
GtkWidget*  create_pixmap              (GtkWidget       *widget,
                                        const gchar     *filename,
                                        gboolean         gnome_pixmap);

GdkImlibImage* create_image            (const gchar     *filename);

#else /* NEED_GNOMESUPPORT_H */

/* Use this function to set the directory containing installed pixmaps. */
void        add_pixmap_directory       (const gchar     *directory);


/*
 * Private Functions.
 */

/* This is used to create the pixmaps in the interface. */
GtkWidget*  create_pixmap              (GtkWidget       *widget,
                                        const gchar     *filename);

#endif /* NEED_GNOMESUPPORT_H */

#endif
