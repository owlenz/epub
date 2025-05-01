#include <gtk/gtk.h>
#include "parser.h"
#include "window.h"
#include <stdio.h>
#include <string.h>

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *label;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "pubby");
  gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
  gtk_window_set_child(GTK_WINDOW(window),box);

  const char *text = parse_main();
  char *text_label = malloc(38 * sizeof(char));
  strncpy(text_label, text, 37);
  text_label[37] = '\0';
  g_print("%s\n",text_label);
  label = gtk_label_new(text_label);

  gtk_box_append(GTK_BOX(box),label);

  gtk_window_present(GTK_WINDOW(window));
}

GtkApplication *window_init() {
  GtkApplication *app;
  int status;

  app = gtk_application_new("org.owlenz.pubby", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  return app;
}
