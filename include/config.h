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
    struct {
        int width;
        char* colour;
    } border;
    enum sunclock_layer layer;
    int width;
    char* anchors;
    char* title;
    int monitor_index;
};

#endif
