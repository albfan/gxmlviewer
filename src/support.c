/*
 * $Id: support.c,v 1.4 2001/11/29 01:28:46 sean_stuckless Exp $
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/stat.h>
#include <string.h>

#include "support.h"

/* This is an internally used function to create pixmaps. */
static GtkWidget* create_dummy_pixmap  (GtkWidget       *widget
#ifdef NEED_GNOMESUPPORT_H
                                        ,gboolean         gnome_pixmap
#endif
					);

GtkWidget*
lookup_widget                          (GtkWidget       *widget,
                                        const gchar     *widget_name)
{
  GtkWidget *parent, *found_widget;

  for (;;)
    {
      if (GTK_IS_MENU (widget))
        parent = gtk_menu_get_attach_widget (GTK_MENU (widget));
      else
        parent = widget->parent;
      if (parent == NULL)
        break;
      widget = parent;
    }

  found_widget = (GtkWidget*) gtk_object_get_data (GTK_OBJECT (widget),
                                                   widget_name);
  if (!found_widget)
    g_warning ("Widget not found: %s", widget_name);
  return found_widget;
}

/* This is a dummy pixmap we use when a pixmap can't be found. */
static char *dummy_pixmap_xpm[] = {
/* columns rows colors chars-per-pixel */
"1 1 1 1",
"  c None",
/* pixels */
" ",
" "
};

/* This is an internally used function to create pixmaps. */
static GtkWidget*
create_dummy_pixmap                    (GtkWidget       *widget
#ifdef NEED_GNOMESUPPORT_H
                                        ,gboolean         gnome_pixmap
#endif
					)
{
  GdkColormap *colormap;
  GdkPixmap *gdkpixmap;
  GdkBitmap *mask;
  GtkWidget *pixmap;

#ifdef NEED_GNOMESUPPORT_H
  if (gnome_pixmap)
    {
      return gnome_pixmap_new_from_xpm_d (dummy_pixmap_xpm);
    }
#endif

  colormap = gtk_widget_get_colormap (widget);
  gdkpixmap = gdk_pixmap_colormap_create_from_xpm_d (NULL, colormap, &mask,
                                                     NULL, dummy_pixmap_xpm);
  if (gdkpixmap == NULL)
    g_error ("Couldn't create replacement pixmap.");
  pixmap = gtk_pixmap_new (gdkpixmap, mask);
  gdk_pixmap_unref (gdkpixmap);
  gdk_bitmap_unref (mask);
  return pixmap;
}

/* This is an internally used function to create pixmaps. */
#ifdef NEED_GNOMESUPPORT_H
GtkWidget*
create_pixmap                          (GtkWidget       *widget,
                                        const gchar     *filename,
                                        gboolean         gnome_pixmap)
{
  GtkWidget *pixmap;
  GdkColormap *colormap;
  GdkPixmap *gdkpixmap;
  GdkBitmap *mask;
  gchar *pathname;

  if (!filename || !filename[0])
      return create_dummy_pixmap (widget, gnome_pixmap);

  pathname = gnome_pixmap_file (filename);
  if (!pathname)
    {
      g_warning (_("Couldn't find pixmap file: %s"), filename);
      return create_dummy_pixmap (widget, gnome_pixmap);
    }

  if (gnome_pixmap)
    {
      pixmap = gnome_pixmap_new_from_file (pathname);
      g_free (pathname);
      return pixmap;
    }

  colormap = gtk_widget_get_colormap (widget);
  gdkpixmap = gdk_pixmap_colormap_create_from_xpm (NULL, colormap, &mask,
                                                   NULL, pathname);
  if (gdkpixmap == NULL)
    {
      g_warning (_("Couldn't create pixmap from file: %s"), pathname);
      g_free (pathname);
      return create_dummy_pixmap (widget, gnome_pixmap);
    }
  g_free (pathname);

  pixmap = gtk_pixmap_new (gdkpixmap, mask);
  gdk_pixmap_unref (gdkpixmap);
  gdk_bitmap_unref (mask);
  return pixmap;
}

/* This is an internally used function to create imlib images. */
GdkImlibImage*
create_image                           (const gchar     *filename)
{
  GdkImlibImage *image;
  gchar *pathname;

  pathname = gnome_pixmap_file (filename);
  if (!pathname)
    {
      g_warning (_("Couldn't find pixmap file: %s"), filename);
      return NULL;
    }

  image = gdk_imlib_load_image (pathname);
  g_free (pathname);
  return image;
}

#else /* NEED_GNOMESUPPORT_H */

/* This is an internally used function to check if a pixmap file exists. */
static gchar* check_file_exists        (const gchar     *directory,
                                        const gchar     *filename);

static GList *pixmaps_directories = NULL;

/* Use this function to set the directory containing installed pixmaps. */
void
add_pixmap_directory                   (const gchar     *directory)
{
  pixmaps_directories = g_list_prepend (pixmaps_directories,
                                        g_strdup (directory));
}


/*
 * Private Functions.
 */

/* This is an internally used function to create pixmaps. */
GtkWidget*
create_pixmap                          (GtkWidget       *widget,
                                        const gchar     *filename)
{
  gchar *found_filename = NULL;
  GdkColormap *colormap;
  GdkPixmap *gdkpixmap;
  GdkBitmap *mask;
  GtkWidget *pixmap;
  GList *elem;

  /* We first try any pixmaps directories set by the application. */
  elem = pixmaps_directories;
  while (elem)
    {
      found_filename = check_file_exists ((gchar*)elem->data, filename);
      if (found_filename)
        break;
      elem = elem->next;
    }

  /* If we haven't found the pixmap, try the source directory. */
  if (!found_filename)
    {
      found_filename = check_file_exists ("../pixmaps", filename);
    }

  if (!found_filename)
    {
      g_warning (_("Couldn't find pixmap file: %s"), filename);
      return create_dummy_pixmap (widget);
    }

  colormap = gtk_widget_get_colormap (widget);
  gdkpixmap = gdk_pixmap_colormap_create_from_xpm (NULL, colormap, &mask,
                                                   NULL, found_filename);
  if (gdkpixmap == NULL)
    {
      g_warning (_("Error loading pixmap file: %s"), found_filename);
      g_free (found_filename);
      return create_dummy_pixmap (widget);
    }
  g_free (found_filename);
  pixmap = gtk_pixmap_new (gdkpixmap, mask);
  gdk_pixmap_unref (gdkpixmap);
  gdk_bitmap_unref (mask);
  return pixmap;
}

/* This is an internally used function to check if a pixmap file exists. */
gchar*
check_file_exists                      (const gchar     *directory,
                                        const gchar     *filename)
{
  gchar *full_filename;
  struct stat s;
  gint status;

  full_filename = (gchar*) g_malloc (strlen (directory) + 1
                                     + strlen (filename) + 1);
  strcpy (full_filename, directory);
  strcat (full_filename, G_DIR_SEPARATOR_S);
  strcat (full_filename, filename);

  status = stat (full_filename, &s);
#ifndef WIN32
  if (status == 0 && S_ISREG (s.st_mode))
#else
   if (status == 0 && s.st_mode == _S_IFREG)
#endif
    return full_filename;
  g_free (full_filename);
  return NULL;
}

#endif /* NEED_GNOMESUPPORT_H */
