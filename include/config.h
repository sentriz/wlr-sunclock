#ifndef SUNCLOCK_CONFIG_H
#define SUNCLOCK_CONFIG_H

struct sunclock_margins {
    int top, right, bottom, left;
};

enum sunclock_layer {
    SUNCLOCK_LAYER_BACKGROUND,
    SUNCLOCK_LAYER_BOTTOM,
    SUNCLOCK_LAYER_TOP,
    SUNCLOCK_LAYER_OVERLAY,
};

struct sunclock_gui_settings {
    struct sunclock_margins margins;
    enum sunclock_layer layer;
    char* image_path;
    char* title;
};

#endif
