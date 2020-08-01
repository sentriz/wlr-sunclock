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
static char doc[] = "A program which accepts an input and output file as arguments";
static struct argp_option options[] = {
    {"layer",         'y', "<background|bottom|top|overlay>", 0, "desktop layer to show the widget on", 1},

    {"margin-right",  'r', "[MARGIN_RIGHT]",                  0, "window margin right",                 2},
    {"margin-bottom", 'b', "[MARGIN_BOTTOM]",                 0, "window margin bottom",                2},
    {"margin-top",    't', "[MARGIN_TOP]",                    0, "window margin top",                   2},
    {"margin-left",   'l', "[MARGIN_LEFT]",                   0, "window margin left",                  2},

    {"border-width",  'd', "[BORDER_WIDTH]",                  0, "window border width",                 3},
    {"border-colour", 'c', "[BORDER_COLOUR]",                 0, "window border colour (unused)",       3},
};
// clang-format on

static error_t parse_option(int key, char* arg, struct argp_state* state) {
    struct sunclock_gui_settings* settings = state->input;
    switch (key) {
    case 'r': settings->margins.right = atoi(arg); break;
    case 'b': settings->margins.bottom = atoi(arg); break;
    case 't': settings->margins.top = atoi(arg); break;
    case 'l': settings->margins.left = atoi(arg); break;
    case 'y':
        settings->layer = layer_from_str(arg);
        if (settings->layer == -1) {
            fprintf(stderr, "invalid layer %s provided\n", arg);
            argp_usage(state);
        };
        break;
    case 'd': settings->border.width = atoi(arg); break;
    case 'c': settings->border.colour = arg; break;
    case ARGP_KEY_ARG: break;
    default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    struct argp argp = {options, parse_option, NULL, doc};
    struct sunclock_gui_settings settings = {
        // TODO: not do this path thing
        .image_path = "/usr/local/share/wlr-sunclock/maps/800.jpg",
        .title = "xyz.senan.wlr-sunclock",
        .layer = SUNCLOCK_LAYER_BOTTOM,
    };
    argp_parse(&argp, argc, argv, 0, 0, &settings);
    return sunclock_gui_start(&settings);
}
