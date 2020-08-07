#include "astro.h"
#include "config.h"
#include "gtk-layer-shell.h"
#include "map.h"
#include <gtk/gtk.h>

static void sunclock_gui_fill_wtab(short* wtab, int width, int height,
                                   int* gmt_position) {
    time_t raw_time;
    time(&raw_time);
    struct tm* gm_time = gmtime(&raw_time);

    double jt, sunra, sundec, sunrv, sunlong;
    jt = astro_gm_time_to_julian_astro(gm_time);
    astro_sun_position(jt, FALSE, &sunra, &sundec, &sunrv, &sunlong);
    astro_project_illum(wtab, width, height, sundec);

    // note: greenwich is in the middle
    int sec =
        gm_time->tm_hour * 60 * 60 + gm_time->tm_min * 60 + gm_time->tm_sec;
    *gmt_position = width * sec / 86400;
}

static void sunclock_gui_draw_line(cairo_t* cr, short x1, short y1, short x2,
                                   short y2) {
    cairo_move_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    cairo_stroke(cr);
}

static gboolean sunclock_gui_draw_shade_timeout(GtkWidget* widget) {
    gtk_widget_queue_draw(widget);
    return TRUE;
}

static gboolean sunclock_gui_draw_shade(GtkWidget* widget, cairo_t* cr,
                                        gpointer image) {
    int height = gtk_widget_get_allocated_height(widget);
    int width = gtk_widget_get_allocated_width(widget);

    GdkPixbuf* image_scaled = gdk_pixbuf_scale_simple(
        (GdkPixbuf*)image, width, height, GDK_INTERP_BILINEAR);
    gdk_cairo_set_source_pixbuf(cr, image_scaled, 0, 0);
    cairo_paint(cr);
    g_object_unref(image_scaled);

    GtkStyleContext* context = gtk_widget_get_style_context(widget);
    GtkStateFlags state = gtk_style_context_get_state(context);
    GdkRGBA colour;
    gtk_style_context_get_color(context, state, &colour);
    gdk_cairo_set_source_rgba(cr, &colour);

    // calculate the illuminated area
    int gmt_position;
    short wtab[height];
    sunclock_gui_fill_wtab(wtab, width, height, &gmt_position);

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
            sunclock_gui_draw_line(cr, 0, y, stop, y);
            sunclock_gui_draw_line(cr, width + start, y, width, y);
        } else if (stop > width) {
            sunclock_gui_draw_line(cr, start, y, width, y);
            sunclock_gui_draw_line(cr, 0, y, stop - width, y);
        } else
            sunclock_gui_draw_line(cr, start, y, stop, y);
    }

    return FALSE;
}

void sunclock_gui_activate(GtkApplication* app, gpointer psettings) {
    GtkWindow* gtk_window = GTK_WINDOW(gtk_application_window_new(app));
    gtk_layer_init_for_window(gtk_window);

    struct sunclock_gui_settings* settings = psettings;

    // user setting border, layer
    gtk_container_set_border_width(GTK_CONTAINER(gtk_window),
                                   settings->border.width);
    gtk_layer_set_layer(gtk_window, (int)settings->layer);

    // user setting margin
    for (int i = 0; i < 4; i++)
        gtk_layer_set_margin(gtk_window, i, settings->margins[i]);

    // user setting anchor
    for (const char* c = settings->anchors; *c; c++)
        switch (*c) {
        case 'l': gtk_layer_set_anchor(gtk_window, 0, TRUE); break;
        case 'r': gtk_layer_set_anchor(gtk_window, 1, TRUE); break;
        case 't': gtk_layer_set_anchor(gtk_window, 2, TRUE); break;
        case 'b': gtk_layer_set_anchor(gtk_window, 3, TRUE); break;
        }

    // user setting monitor index
    GdkDisplay* display = gdk_display_get_default();
    GdkMonitor* monitor =
        gdk_display_get_monitor(display, settings->monitor_index);
    gtk_layer_set_monitor(gtk_window, monitor);

    GtkWidget* canvas = gtk_drawing_area_new();
    gtk_widget_set_hexpand(canvas, TRUE);
    gtk_widget_set_vexpand(canvas, TRUE);

    int image_width = settings->width;
    int image_height = settings->width / 2;
    GdkPixbuf* map_image =
        gdk_pixbuf_new_from_inline(-1, sunclock_map, FALSE, NULL);
    gtk_widget_set_size_request(canvas, image_width, image_height);

    // draw canvas now and every 30s
    g_signal_connect(canvas, "draw", G_CALLBACK(sunclock_gui_draw_shade),
                     map_image);
    g_timeout_add_seconds(30, G_SOURCE_FUNC(sunclock_gui_draw_shade_timeout),
                          canvas);

    gtk_container_add(GTK_CONTAINER(gtk_window), canvas);
    gtk_widget_show_all(GTK_WIDGET(gtk_window));
}

int sunclock_gui_start(struct sunclock_gui_settings* settings) {
    GtkApplication* app = gtk_application_new(settings->title, 0);
    g_signal_connect(app, "activate", G_CALLBACK(sunclock_gui_activate),
                     settings);

    int status = g_application_run(G_APPLICATION(app), 0, 0);
    g_object_unref(app);
    return status;
}
