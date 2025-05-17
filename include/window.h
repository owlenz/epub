#ifndef WINDOW_H
#define WINDOW_H

#include <gtk/gtk.h>
#include <adwaita.h>

static void activate(GtkApplication *app, gpointer user_data);
AdwApplication *window_init();


#endif
