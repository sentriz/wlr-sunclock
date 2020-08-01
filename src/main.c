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
    {"layer",         'l', "<background|bottom|top|overlay>", 0, "desktop layer to show the widget on", 1},
    {"width",         'w', "[WIDTH]",                         0, "window width",                        1},
    {"anchors",       'a', "[ANCHORS]",                       0, "window anchors",                      1},
    {"margins",       'm', "[MARGINS]",                       0, "window margins",                      1},
    {"border-width",  'd', "[BORDER_WIDTH]",                  0, "window border width",                 2},
    {"border-colour", 'c', "[BORDER_COLOUR]",                 0, "window border colour (unused)",       2},
};
// clang-format on

static error_t parse_option(int key, char* arg, struct argp_state* state) {
    struct sunclock_gui_settings* settings = state->input;
    switch (key) {
    case 'l':
        settings->layer = layer_from_str(arg);
        if (settings->layer == -1) {
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
        .image_path = "/usr/share/wlr-sunclock/maps/1200.jpg",
        .title = "xyz.senan.wlr-sunclock",
        .layer = SUNCLOCK_LAYER_BOTTOM,
        .width = 300,
        .anchors = "",
    };
    argp_parse(&argp, argc, argv, 0, 0, &settings);
    if (access(settings.image_path, F_OK) == -1) {
        fprintf(stderr, "couldn't locate image `%s`\n", settings.image_path);
        return 1;
    }
    return sunclock_gui_start(&settings);
}
