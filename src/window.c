#include "window.h"
#include "glib-object.h"
#include "gtk/gtkshortcut.h"
#include "parser.h"
#include <adwaita.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

static void button(GtkWidget *btn) {
  /* g_print("%s", gtk_label_get_label( */
  /*                   GTK_LABEL(gtk_button_get_child(GTK_BUTTON(btn))))); */
}

bool toc_open = false;

static void toc_activate(GtkWidget *btn,GtkWidget *scrolled) {
  toc_open = !toc_open;
  gtk_widget_set_visible(scrolled,toc_open);
}


static void activate(GtkApplication *app, gpointer user_data) {
  adw_init();
  GtkWidget *window;
  GtkWidget *scrolled;
  GtkWidget *box;
  GtkWidget *toc_button;

  
  window = adw_application_window_new(GTK_APPLICATION(app));
  gtk_window_set_title(GTK_WINDOW(window), "pubby");
  gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);

  scrolled = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request(scrolled, 300, 200);

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), box);


  toc_button = gtk_button_new_with_label("toc");
  gtk_widget_set_size_request(toc_button, 20, 20);
  g_signal_connect(toc_button, "clicked", G_CALLBACK(toc_activate), scrolled);

  GtkWidget *main_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_box_append(GTK_BOX(main_box),toc_button);
  gtk_box_append(GTK_BOX(main_box),scrolled);
  
  adw_application_window_set_content(ADW_APPLICATION_WINDOW(window), main_box);

  zip_init();
  char **toc = read_zip_file("OEBPS/toc.ncx");

  int n = 50;
  while (n--) {
    GtkWidget *label;
    GtkWidget *button;

    char *text_label = malloc(101 * sizeof(char));
    strncpy(text_label, toc[n], 100);

    label = gtk_label_new(text_label);
    gtk_label_set_wrap(GTK_LABEL(label),TRUE);
    gtk_widget_set_halign(label,GTK_ALIGN_START);
    
    button = gtk_button_new();
    g_signal_connect(button, "clicked", G_CALLBACK(button), NULL);

    gtk_widget_set_size_request(button, 200, -1);
    gtk_button_set_child(GTK_BUTTON(button), label);

    gtk_box_append(GTK_BOX(box), button);
  }

  gtk_widget_set_visible(scrolled, false);

  gtk_window_present(GTK_WINDOW(window));
}

AdwApplication *window_init() {
  AdwApplication *app;
  int status;

  app = adw_application_new("org.owlenz.pubby", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  return app;
}
