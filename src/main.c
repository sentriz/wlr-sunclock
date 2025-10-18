#include "astro.h"
#include "gtk4-layer-shell.h"
#include "map.h"
#include "version.h"
#include <argp.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <librsvg-2.0/librsvg/rsvg.h>
#include <stdlib.h>
#include <string.h>

static struct settings {
    int margins[4];

    char* title;
    int layer;
    char* anchors;
    int monitor_index;
    int width;

    char* colour_ocean;
    char* colour_land;
    char* colour_sun;
    gboolean show_sun;
} g_settings = {
    .title = "xyz.senan.wlr-sunclock",
    .layer = GTK_LAYER_SHELL_LAYER_BOTTOM,
    .width = 300,
    .anchors = "",
    .monitor_index = 0,
    .colour_ocean = "#4c446d",
    .colour_land = "#726f9e",
    .colour_sun = "#ffe135",
    .show_sun = FALSE,
};

static gboolean draw_shade_timeout(GtkWidget* widget) {
    gtk_widget_queue_draw(widget);
    return TRUE;
}

typedef struct {
    double x, y;
} Point;

static Point astro_to_point(AstroPoint astro_point, int width, int height,
                            int middle, gboolean reverse) {
    double screen_x = (astro_point.lon + 180.0) * width / 360.0;
    double screen_y = (90.0 - astro_point.lat) * height / 180.0;

    // apply GMT offset
    if (reverse)
        screen_x += middle;
    else
        screen_x = -screen_x + middle;

    while (screen_x > width)
        screen_x -= width;
    while (screen_x < 0)
        screen_x += width;

    Point p;
    p.x = screen_x;
    p.y = screen_y;

    return p;
}

int compare_point_x(const void* a, const void* b) {
    const Point* p1 = (const Point*)a;
    const Point* p2 = (const Point*)b;

    if (p1->x < p2->x)
        return -1;
    else if (p1->x > p2->x)
        return 1;
    else
        return 0;
}

static void draw_sun(cairo_t* cr, double x, double y, double radius,
                     const char* color_hex) {
    unsigned int r, g, b;
    if (sscanf(color_hex, "#%02x%02x%02x", &r, &g, &b) != 3)
        return;

    // Convert to 0-1 range
    double red = r / 255.0;
    double green = g / 255.0;
    double blue = b / 255.0;

    // circle
    cairo_new_path(cr);
    cairo_arc(cr, x, y, radius, 0, 2 * G_PI);
    cairo_set_source_rgba(cr, red, green, blue, 1.0);
    cairo_fill(cr);

    // rays - slightly darker
    cairo_set_line_width(cr, 2.0);
    cairo_set_source_rgba(cr, red * 0.9, green * 0.9, blue * 0.9, 1.0);

    int num_rays = 8;
    double ray_length = radius * 2;
    for (int i = 0; i < num_rays; i++) {
        double angle = (i * 2 * G_PI) / num_rays;
        double inner_x = x + cos(angle) * (radius + 2);
        double inner_y = y + sin(angle) * (radius + 2);
        double outer_x = x + cos(angle) * ray_length;
        double outer_y = y + sin(angle) * ray_length;

        cairo_move_to(cr, inner_x, inner_y);
        cairo_line_to(cr, outer_x, outer_y);
        cairo_stroke(cr);
    }
}

static void draw_shade(GtkDrawingArea* area, cairo_t* cr, int width, int height,
                       gpointer user_data) {
    (void)area;

    RsvgHandle* svg_handle = (RsvgHandle*)user_data;
    RsvgRectangle viewport = {.x = 0, .y = 0, .width = width, .height = height};

    cairo_save(cr);
    rsvg_handle_render_document(svg_handle, cr, &viewport, NULL);
    cairo_restore(cr);

    time_t raw_time;
    time(&raw_time);

    struct tm* gm_time = gmtime(&raw_time);

    double jt, sunra, sundec, sunrv, sunlong;
    jt = astro_gm_time_to_julian_astro(gm_time);
    astro_sun_position(jt, FALSE, &sunra, &sundec, &sunrv, &sunlong);

    int sec =
        gm_time->tm_hour * 60 * 60 + gm_time->tm_min * 60 + gm_time->tm_sec;
    int gmt_position = width * sec / 86400;
    int middle = width - gmt_position;

    static size_t max_coords = 1000;
    AstroPoint coords[max_coords];
    int num_coords = astro_terminator_points(
        coords, sizeof(coords) / sizeof(AstroPoint), sundec);
    if (num_coords == 0)
        return;

    size_t num_points = num_coords * 2;
    Point points[num_points];

    for (int i = 0; i < num_coords; i++) {
        points[2 * i] = astro_to_point(coords[i], width, height, middle, FALSE);
        points[2 * i + 1] =
            astro_to_point(coords[i], width, height, middle, TRUE);
    }
    qsort(points, num_points, sizeof(Point), compare_point_x);

    Point p0 = points[0];
    Point pN = points[num_points - 1];

    cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);

    cairo_new_path(cr);
    cairo_set_source_rgba(cr, 0.933333, 0.933333, 0.925490, 0.4);

    // fill pole
    sundec > 0 ? cairo_line_to(cr, 0, 0)       // northern summer
               : cairo_line_to(cr, 0, height); // southern summer

    cairo_line_to(cr, 0, p0.y);

    for (size_t i = 0; i < num_points; i++) {
        Point p = points[i];
        cairo_line_to(cr, p.x, p.y);
    }

    cairo_line_to(cr, width, pN.y);

    // fill pole
    sundec > 0 ? cairo_line_to(cr, width, 0)       // northern summer
               : cairo_line_to(cr, width, height); // southern summer

    cairo_fill(cr);

    if (g_settings.show_sun) {
        AstroPoint subsolar = {.lat = sundec, .lon = 180.0};
        Point p = astro_to_point(subsolar, width, height, middle, FALSE);

        draw_sun(cr, p.x, p.y, 10, g_settings.colour_sun);
    }
}

void activate(GtkApplication* app, gpointer user_data) {
    RsvgHandle* map_handle = (RsvgHandle*)user_data;

    GtkWindow* gtk_window = GTK_WINDOW(gtk_application_window_new(app));
    gtk_layer_init_for_window(gtk_window);

    // user setting layer
    gtk_layer_set_layer(gtk_window, (int)g_settings.layer);

    // user setting margin
    for (int i = 0; i < 4; i++)
        gtk_layer_set_margin(gtk_window, i, g_settings.margins[i]);

    for (const char* c = g_settings.anchors; *c; c++)
        switch (*c) {
        case 'l':
            gtk_layer_set_anchor(gtk_window, GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
            break;
        case 'r':
            gtk_layer_set_anchor(gtk_window, GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
            break;
        case 't':
            gtk_layer_set_anchor(gtk_window, GTK_LAYER_SHELL_EDGE_TOP, TRUE);
            break;
        case 'b':
            gtk_layer_set_anchor(gtk_window, GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
            break;
        }

    // user setting monitor index
    if (g_settings.monitor_index > 0) {
        GdkDisplay* display = gdk_display_get_default();
        GListModel* monitors = gdk_display_get_monitors(display);
        GdkMonitor* monitor =
            g_list_model_get_item(monitors, g_settings.monitor_index);
        if (monitor) {
            gtk_layer_set_monitor(gtk_window, monitor);
            g_object_unref(monitor);
        }
    }

    GtkWidget* canvas = gtk_drawing_area_new();
    gtk_widget_set_hexpand(canvas, TRUE);
    gtk_widget_set_vexpand(canvas, TRUE);

    int image_height = g_settings.width / 2;
    int image_width = g_settings.width;
    gtk_widget_set_size_request(canvas, image_width, image_height);

    // draw canvas now and every 30s
    // pass the SVG handle directly (no pixbuf conversion)
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(canvas), draw_shade,
                                   map_handle, NULL);
    g_timeout_add_seconds(30, G_SOURCE_FUNC(draw_shade_timeout), canvas);

    gtk_window_set_child(gtk_window, canvas);
    gtk_window_present(gtk_window);
}

static int layer_from_str(char* in) {
    if (strcmp(in, "background") == 0)
        return GTK_LAYER_SHELL_LAYER_BACKGROUND;
    if (strcmp(in, "bottom") == 0)
        return GTK_LAYER_SHELL_LAYER_BOTTOM;
    if (strcmp(in, "top") == 0)
        return GTK_LAYER_SHELL_LAYER_TOP;
    if (strcmp(in, "overlay") == 0)
        return GTK_LAYER_SHELL_LAYER_OVERLAY;
    return -1;
}

// clang-format off
static char doc[] = "Displays a sunclock desktop widget using the layer shell protocol";
static struct argp_option options[] = {
    {"layer",         'l', "LAYER",         0, "layer (background, bottom, top, overlay)",                      0},
    {"monitor-index", 'i', "INDEX",         0, "monitor to show window on (starts at 0)",                       0},

    {"width",         'w', "SIZE",          0, "width of the window in pixels",                                 1},
    {"anchors",       'a', "ANCHORS",       0, "string of window anchors (see readme)",                         1},
    {"margins",       'm', "MARGINS",       0, "comma separated margins for window (left, right, top, bottom)", 1},

    {"colour-ocean",  'o', "COLOUR",        0, "colour of the ocean",                                           2},
    {"colour-land",   'n', "COLOUR",        0, "colour of the land",                                            2},
    {"colour-sun",    's', "COLOUR",        0, "colour of the sun (requires --show-sun)",                       2},

    {"show-sun",      'S', NULL,            0, "show sun at subsolar point",                                    3},

    {"version",       'v', NULL,            0, "print version",                                                 4},
    {0},
};
// clang-format on

static error_t parse_option(int key, char* arg, struct argp_state* state) {
    struct settings* settings = &g_settings;
    switch (key) {
    case 'l':
        settings->layer = layer_from_str(arg);
        if ((int)settings->layer == -1) {
            fprintf(stderr, "invalid layer %s provided\n", arg);
            argp_usage(state);
        };
        break;
    case 'w': settings->width = atoi(arg); break;
    case 'a': settings->anchors = arg; break;
    case 'm':
        sscanf(arg, "%d,%d,%d,%d", &settings->margins[0], &settings->margins[1],
               &settings->margins[2], &settings->margins[3]);
        break;
    case 'i': settings->monitor_index = atoi(arg); break;
    case 'o': settings->colour_ocean = arg; break;
    case 'n': settings->colour_land = arg; break;
    case 's': settings->colour_sun = arg; break;
    case 'S': settings->show_sun = TRUE; break;
    case 'v':
        fprintf(stderr, "sunclock version %s\n", VERSION);
        exit(0);
        break;
    case ARGP_KEY_ARG: break;
    default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    struct argp argp = {options, parse_option, NULL, doc, NULL, NULL, NULL};
    argp_parse(&argp, argc, argv, 0, 0, NULL);

    g_setenv("GDK_DISABLE", "vulkan", TRUE);

    const gchar* map = g_strdup_printf(MAP_SVG, g_settings.colour_ocean,
                                       g_settings.colour_land);
    glong map_len = g_utf8_strlen(map, -1);
    RsvgHandle* map_handle =
        rsvg_handle_new_from_data((guint8*)map, map_len, NULL);
    g_free((gpointer)map);

    GtkApplication* app = gtk_application_new(g_settings.title, 0);
    g_signal_connect(app, "activate", G_CALLBACK(activate), map_handle);

    int status = g_application_run(G_APPLICATION(app), 0, 0);
    g_object_unref(app);
    g_object_unref(map_handle);
    return status;
}
