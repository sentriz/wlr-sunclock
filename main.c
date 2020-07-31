#include "astro.h"
#include "gtk-layer-shell.h"
#include <gtk/gtk.h>

static void fill_wtab(short *wtab, int width, int height, int *gmt_position) {
  time_t t = time(NULL);
  struct tm *gm_time = gmtime(&t);

  double jt, sunra, sundec, sunrv, sunlong;
  jt = astro_gm_time_to_julian_astro(gm_time);
  astro_sun_position(jt, FALSE, &sunra, &sundec, &sunrv, &sunlong);
  astro_project_illum(wtab, width, height, sundec);

  // note: greenwich is in the middle
  int sec = gm_time->tm_hour * 60 * 60 + gm_time->tm_min * 60 + gm_time->tm_sec;
  *gmt_position = width * sec / 86400;
}

static void drawLine(cairo_t *cr, short x1, short y1, short x2, short y2) {
  cairo_move_to(cr, x1, y1);
  cairo_line_to(cr, x2, y2);
  cairo_stroke(cr);
}

static void fill_canvas(int width, int height, cairo_t *cr) {
  // calculate the illuminated area
  int gmt_position;
  short wtab[height];
  fill_wtab(wtab, width, height, &gmt_position);

  // draw illuminated area
  cairo_set_line_width(cr, 0.3);
  int start, stop;
  int middle = width - gmt_position;
  for (int y = 0; y < height; y++) {
    if (wtab[y] <= 0)
      continue;
    start = middle - wtab[y];
    stop = middle + wtab[y];
    if (start < 0) {
      drawLine(cr, 0, y, stop, y);
      drawLine(cr, width + start, y, width, y);
    } else if (stop > width) {
      drawLine(cr, start, y, width, y);
      drawLine(cr, 0, y, stop - width, y);
    } else
      drawLine(cr, start, y, stop, y);
  }
};

static gboolean draw_shade_timeout(GtkWidget *widget) {
  gtk_widget_queue_draw(widget);
  return TRUE;
};

static gboolean draw_shade(GtkWidget *widget, cairo_t *cr, gpointer _) {
  (void)_;

  GtkStyleContext *context = gtk_widget_get_style_context(widget);
  int width = gtk_widget_get_allocated_width(widget);
  int height = gtk_widget_get_allocated_height(widget);

  GtkStateFlags state = gtk_style_context_get_state(context);
  GdkRGBA colour;
  gtk_style_context_get_color(context, state, &colour);
  gdk_cairo_set_source_rgba(cr, &colour);

  fill_canvas(width, height, cr);

  return FALSE;
}

static void activate(GtkApplication *app, gpointer _) {
  (void)_;

  GtkWindow *gtk_window = GTK_WINDOW(gtk_application_window_new(app));
  gtk_layer_init_for_window(gtk_window);
  gtk_layer_set_layer(gtk_window, GTK_LAYER_SHELL_LAYER_TOP);
  gtk_layer_set_margin(gtk_window, GTK_LAYER_SHELL_EDGE_RIGHT, 0);
  gtk_layer_set_margin(gtk_window, GTK_LAYER_SHELL_EDGE_BOTTOM, 16);

  // anchor bottom right
  gtk_layer_set_anchor(gtk_window, 1, TRUE);
  gtk_layer_set_anchor(gtk_window, 3, TRUE);

  GtkWidget *image = gtk_image_new_from_file(
      "/home/senan/projects/kdetoys_3.5.10.orig/kdetoys-3.5.10/kworldwatch/"
      "maps/flatworld/800.jpg");
  const GdkPixbuf *pb = gtk_image_get_pixbuf(GTK_IMAGE(image));
  int width = gdk_pixbuf_get_width(pb);
  int height = gdk_pixbuf_get_height(pb);

  GtkWidget *overlay = gtk_overlay_new();
  gtk_widget_set_hexpand(overlay, TRUE);
  gtk_widget_set_vexpand(overlay, TRUE);
  gtk_widget_set_app_paintable(overlay, TRUE);
  gtk_widget_set_size_request(overlay, width, height);
  gtk_overlay_add_overlay(GTK_OVERLAY(overlay), image);
  gtk_overlay_set_overlay_pass_through(GTK_OVERLAY(overlay), image, TRUE);

  // draw shade now and every 30s
  g_signal_connect_after(overlay, "draw", G_CALLBACK(draw_shade), image);
  g_timeout_add_seconds(30, G_SOURCE_FUNC(draw_shade_timeout), image);

  gtk_container_set_border_width(GTK_CONTAINER(gtk_window), 12);
  gtk_container_add(GTK_CONTAINER(gtk_window), overlay);
  gtk_widget_show_all(GTK_WIDGET(gtk_window));
}

int main(int argc, char **argv) {
  GtkApplication *app =
      gtk_application_new("xyz.senan.sun-clock", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
