#ifndef SUNCLOCK_CONFIG_H
#define SUNCLOCK_CONFIG_H

enum sunclock_layer {
    SUNCLOCK_LAYER_BACKGROUND,
    SUNCLOCK_LAYER_BOTTOM,
    SUNCLOCK_LAYER_TOP,
    SUNCLOCK_LAYER_OVERLAY,
};

struct sunclock_gui_settings {
    int margins[4];
    int border_width;

    char* title;
    enum sunclock_layer layer;
    char* anchors;
    int monitor_index;
    int width;

    char* colour_ocean;
    char* colour_land;
};

#endif
