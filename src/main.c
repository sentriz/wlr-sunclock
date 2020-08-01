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
    {"margin-right",  'r', "[MARGIN_RIGHT]",                  0, "window margin right",                 1},
    {"margin-bottom", 'b', "[MARGIN_BOTTOM]",                 0, "window margin bottom",                1},
    {"margin-top",    't', "[MARGIN_TOP]",                    0, "window margin top",                   1},
    {"margin-left",   'l', "[MARGIN_LEFT]",                   0, "window margin left",                  1},
    {"layer",         'y', "<background|bottom|top|overlay>", 0, "desktop layer to show the widget on", 2},
};
// clang-format on

struct arguments {
    struct sunclock_margins margins;
    enum sunclock_layer layer;
};

static error_t parse_option(int key, char* arg, struct argp_state* state) {
    struct arguments* arguments = state->input;
    switch (key) {
    case 'r': arguments->margins.right = atoi(arg); break;
    case 'b': arguments->margins.bottom = atoi(arg); break;
    case 't': arguments->margins.top = atoi(arg); break;
    case 'l': arguments->margins.left = atoi(arg); break;
    case 'y':
        arguments->layer = layer_from_str(arg);
        if (arguments->layer == -1) {
            fprintf(stderr, "invalid layer %s provided\n", arg);
            argp_usage(state);
        };
        break;
    case ARGP_KEY_ARG: break;
    default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    struct argp argp = {options, parse_option, NULL, doc};
    struct arguments arguments = {
        // default args
        .layer = SUNCLOCK_LAYER_BOTTOM,
        .margins = {},
    };
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    struct sunclock_gui_settings settings = {
        // TODO: not do this path thing
        .image_path = "/usr/local/share/wlr-sunclock/maps/800.jpg",
        .title = "xyz.senan.wlr-sunclock",
        .margins = arguments.margins,
        .layer = arguments.layer,
    };
    return sunclock_gui_start(&settings);
}
