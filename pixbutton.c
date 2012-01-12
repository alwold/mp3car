#include <gtk/gtk.h>
GtkWidget *gtk_button_new_with_pixmap( GtkWidget *parent, gchar *xpm_filename ) {
   GdkPixmap *pixmap;
   GtkStyle *style;
   GdkBitmap *mask;
   GtkWidget *pix, *button;
   style = gtk_widget_get_style(parent);
   pixmap = gdk_pixmap_create_from_xpm(parent->window, &mask,
                                       &style->bg[GTK_STATE_NORMAL],
                                       xpm_filename);
   pix = gtk_pixmap_new(pixmap, mask);
   button = gtk_button_new();
   gtk_container_add( GTK_CONTAINER(button), pix);
   gtk_widget_show(pix);
   return button;
} 
