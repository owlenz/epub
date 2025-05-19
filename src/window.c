#include "window.h"
#include "glib.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include "parser.h"
#include <adwaita.h>
#include <stdio.h>
#include <string.h>

struct toc_button_t {
  char *file_path;
  GtkWidget *container;
};

typedef struct {
  GtkWidget *container;
  char *file_path;
  GtkWidget *text_view;
} open_chapter_t;

GList *all_buttons = NULL;

static void toggle_toc(GtkStack *stack) {
  const char *visible = gtk_stack_get_visible_child_name(stack);

  if (strcmp(visible, "toc") != 0)
    gtk_stack_set_visible_child_name(stack, "toc");
  else
    gtk_stack_set_visible_child_name(stack, "text");
}

static gboolean key_pressed(GtkEventControllerKey *controller, guint keyval,
                            guint keycode, GdkModifierType state,
                            gpointer user_data) {

  if (keyval == GDK_KEY_Tab) {
    g_print("balls");
    toggle_toc(GTK_STACK(user_data));
    return TRUE;
  }
  return FALSE;
}

static void button_activate(GtkWidget *btn, gpointer user_data) {
  open_chapter_t *data = (open_chapter_t *)user_data;
  GtkTextBuffer *buffer;
  g_print("button: %s\n", data->file_path);
  char xdd[200] = "OEBPS/";
  strcat(xdd, data->file_path);

  struct chapter *chap = read_zip_file(xdd);

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->text_view));
  gtk_text_buffer_set_text(buffer, chap->buffer, -1);
  gtk_stack_set_visible_child_name(GTK_STACK(data->container), "text");
}

GtkWidget *create_scrollable(GtkWidget *child, int width, int height) {
  GtkWidget *scrolled;
  scrolled = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request(scrolled, width, height);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), child);
  return scrolled;
}

void toc_buttons(GtkWidget *stack, GtkWidget *text_view, GtkWidget *toc_box) {
  zip_init();
  struct toc *toc = read_zip_file("OEBPS/toc.ncx");
  g_print("xddmors: %s\n", toc[50].file);
  int i = 1240;
  for (int n = 0; n < i; n++) {
    GtkWidget *label;
    GtkWidget *button;

    char *text_label = malloc(101 * sizeof(char));
    strncpy(text_label, toc[n].entry, 100);

    label = gtk_label_new(text_label);
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    gtk_widget_set_halign(label, GTK_ALIGN_START);

    button = gtk_button_new();
    open_chapter_t *chapter = malloc(sizeof(open_chapter_t));
    chapter->file_path = toc[n].file;
    chapter->container = stack;
    chapter->text_view = text_view;
    g_signal_connect(button, "clicked", G_CALLBACK(button_activate), chapter);

    gtk_widget_set_size_request(button, 200, -1);
    gtk_button_set_child(GTK_BUTTON(button), label);

    all_buttons = g_list_append(all_buttons,button);
    gtk_box_append(GTK_BOX(toc_box), button);
  }
}

static void filter_buttons(GtkSearchEntry *entry, gpointer t) {
  const char *text = gtk_editable_get_text(GTK_EDITABLE(entry));

  /* if (data == NULL) { */
  /*   g_print("%s\n", data); */
  /* } */

  for (GList *l = all_buttons; l != NULL; l = l->next) {
    GtkWidget *btn = GTK_WIDGET(l->data);
    GtkWidget *label = gtk_button_get_child(GTK_BUTTON(btn));
    const char *label_txt = gtk_label_get_text(GTK_LABEL(label));

    char *lower_text = g_ascii_strdown(text, -1);
    char *lower_label = g_ascii_strdown(label_txt, -1);

    if (g_strstr_len(lower_label, -1, lower_text) != NULL || strlen(label_txt) == 0) {
      gtk_widget_set_visible(btn,true);
    } else {
      gtk_widget_set_visible(btn,false);
    }
  }
}

static void activate(GtkApplication *app, gpointer user_data) {
  adw_init();
  GtkWidget *window;
  GtkWidget *toc_scrolled;
  GtkWidget *toc_box;
  GtkWidget *toc_button;
  GtkWidget *text_view;
  GtkWidget *stack;

  GtkCssProvider *css_provider = gtk_css_provider_new();

  gtk_css_provider_load_from_string(
      css_provider, "textview {background-color: #242424; padding: 10px;}");

  gtk_style_context_add_provider_for_display(
      gdk_display_get_default(), GTK_STYLE_PROVIDER(css_provider), -1);

  window = adw_application_window_new(GTK_APPLICATION(app));
  gtk_window_set_title(GTK_WINDOW(window), "pubby");
  gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);

  toc_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_widget_set_halign(toc_box, GTK_ALIGN_CENTER);
  toc_scrolled = create_scrollable(toc_box, 300, 100);

  stack = gtk_stack_new();
  gtk_stack_add_named(GTK_STACK(stack), toc_scrolled, "toc");
  adw_application_window_set_content(ADW_APPLICATION_WINDOW(window), stack);

  GtkEventController *key_controller = gtk_event_controller_key_new();
  gtk_widget_add_controller(window, key_controller);
  g_signal_connect(key_controller, "key-pressed", G_CALLBACK(key_pressed),
                   stack);

  text_view = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
  gtk_widget_set_size_request(text_view, 1000, -1);
  gtk_widget_set_halign(text_view, GTK_ALIGN_CENTER);

  GtkWidget *text_scrollable = create_scrollable(text_view, 400, -1);
  gtk_stack_add_named(GTK_STACK(stack), text_scrollable, "text");

  /* gtk_widget_set_can_focus(text_view, false); */
  GtkEventController *text_cont = gtk_event_controller_key_new();
  gtk_widget_add_controller(text_view, text_cont);
  g_signal_connect(text_cont, "key-pressed", G_CALLBACK(key_pressed), stack);

  GtkWidget *search_bar = gtk_search_bar_new();
  gtk_box_append(GTK_BOX(toc_box), search_bar);
  gtk_search_bar_set_key_capture_widget(GTK_SEARCH_BAR(search_bar), window);
  GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_search_bar_set_child(GTK_SEARCH_BAR(search_bar), search_box);

  GtkWidget *search_entry = gtk_search_entry_new();
  gtk_box_append(GTK_BOX(search_box), search_entry);

  gtk_search_bar_connect_entry(GTK_SEARCH_BAR(search_bar),
                               GTK_EDITABLE(search_entry));


  toc_buttons(stack, text_view, toc_box);

  g_signal_connect(search_entry, "search-changed", G_CALLBACK(filter_buttons), NULL);
  
  gtk_stack_set_visible_child_name(GTK_STACK(stack), "toc");

  gtk_window_present(GTK_WINDOW(window));
}

AdwApplication *window_init() {
  AdwApplication *app;
  int status;

  app = adw_application_new("org.owlenz.pubby", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  return app;
}
