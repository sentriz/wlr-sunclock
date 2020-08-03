#include "config.h"
#include "gui.h"
#include <argp.h>
#include <stdlib.h>
#include <string.h>

static enum sunclock_layer layer_from_str(char* in) {
    if (strcmp(in, "background") == 0)
        return SUNCLOCK_LAYER_BACKGROUND;
    if (strcmp(in, "bottom") == 0)
        return SUNCLOCK_LAYER_BOTTOM;
    if (strcmp(in, "top") == 0)
        return SUNCLOCK_LAYER_TOP;
    if (strcmp(in, "overlay") == 0)
        return SUNCLOCK_LAYER_OVERLAY;
    return -1;
}

// clang-format off
static char doc[] = "Displays a sunclock desktop widget using the layer shell protocol";
static struct argp_option options[] = {
    {"layer",         'l', "<background|bottom|top|overlay>", 0, "desktop layer to show the widget on",               1},
    {"width",         'w', "WIDTH",                           0, "width of the window",                               1},
    {"anchors",       'a', "ANCHORS",                         0, "string of window anchors (see readme)",             1},
    {"margins",       'm', "MARGINS",                         0, "comma seperated margins for window",                1},
    {"monitor-index", 'i', "MONITOR_INDEX",                   0, "monitor to show window on (starts at 0)",           1},
    {"border-width",  'd', "BORDER_WIDTH",                    0, "width of the window's border",                      2},
    {"border-colour", 'c', "BORDER_COLOUR",                   0, "colour of the window's border (to be implemented)", 2},
    {0},
};
// clang-format on

static error_t parse_option(int key, char* arg, struct argp_state* state) {
    struct sunclock_gui_settings* settings = state->input;
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
        for (int i = 0; i < 4; i++)
            settings->margins[i] = atoi(strsep(&arg, ","));
        break;
    case 'i': settings->monitor_index = atoi(arg); break;
    case 'd': settings->border.width = atoi(arg); break;
    case 'c': settings->border.colour = arg; break;
    case ARGP_KEY_ARG: break;
    default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    struct argp argp = {options, parse_option, NULL, doc, NULL, NULL, NULL};
    struct sunclock_gui_settings settings = {
        .title = "xyz.senan.wlr-sunclock",
        .layer = SUNCLOCK_LAYER_BOTTOM,
        .width = 300,
        .anchors = "",
        .monitor_index = 0,
    };
    argp_parse(&argp, argc, argv, 0, 0, &settings);
    return sunclock_gui_start(&settings);
}
