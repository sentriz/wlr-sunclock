#ifndef SUNCLOCK_GUI_H
#define SUNCLOCK_GUI_H

#include "config.h"
#include "gtk-layer-shell.h"
#include <gtk/gtk.h>

void sunclock_gui_activate(GtkApplication* app, gpointer psettings);
int sunclock_gui_start(struct sunclock_gui_settings* settings);

#endif
