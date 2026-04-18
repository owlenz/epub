#include "window.h"
#include "gdk/gdkkeysyms.h"
#include "gio/gio.h"
#include "glib-object.h"
#include "glib.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include "parser.h"
#include <adwaita.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct toc_button_t {
  char *file_path;
  GtkWidget *container;
};

typedef struct {
  GtkWidget *container;
  uint8_t *file_path;
  GtkWidget *text_view;
} open_chapter_t;

GList *all_buttons = NULL;
GtkBuilder *builder = NULL;

static void toggle_toc(GtkStack *stack) {
  const char *visible = gtk_stack_get_visible_child_name(stack);
  g_print("%s\n", visible);

  if (strcmp(visible, "toc-page") == 0)
    gtk_stack_set_visible_child_name(stack, "text-page");
  else if(strcmp(visible, "text-page") == 0)
    gtk_stack_set_visible_child_name(stack, "toc-page");
}

static gboolean key_pressed(GtkEventControllerKey *controller, guint keyval,
                            guint keycode, GdkModifierType state,
                            gpointer user_data) {
    g_print("balls\n");

  if (keyval == GDK_KEY_t) {
    g_print("balls\n");
    toggle_toc(GTK_STACK(user_data));
    return TRUE;
  }
  return FALSE;
}

static void button_activate(GtkWidget *btn, gpointer user_data) {
  open_chapter_t *data = (open_chapter_t *)user_data;
  GtkTextBuffer *buffer;
  g_print("button: %s\n", data->file_path);

  struct chapter *chap = read_html(data->file_path);

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->text_view));
  gtk_text_buffer_set_text(buffer, chap->buffer, -1);
  gtk_stack_set_visible_child_name(GTK_STACK(data->container), "text-page");
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

// table of contents
void epub_init(GtkWidget *stack, GtkWidget *text_view, GtkWidget *toc_box, const char *path) {
  zip_init(path);
  struct toc *toc = read_toc();
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

    all_buttons = g_list_append(all_buttons, button);
    gtk_box_append(GTK_BOX(toc_box), button);
  }
}

static void show_error_message(GtkWindow *window, const char *message) {

  GtkAlertDialog *alert = gtk_alert_dialog_new("Error");

  gtk_alert_dialog_set_detail(alert, message);
  gtk_alert_dialog_show(alert, window);

  g_object_unref(alert);
}

typedef struct {
    GtkWindow *window;
    GtkStack  *stack;
} FileOpenData;

G_MODULE_EXPORT void
on_file_open_response(GObject *dialog, GAsyncResult *result, gpointer data) {
  GError *err;
  GFile *file =
      gtk_file_dialog_open_finish(GTK_FILE_DIALOG(dialog), result, &err);

  FileOpenData *open_data = (FileOpenData*)data;
  
  if (file) {
    gtk_stack_set_visible_child_name(open_data->stack, "book-page");
    GtkWidget *text_view = GTK_WIDGET(gtk_builder_get_object(builder,"text_view"));
    GtkWidget *toc_box = GTK_WIDGET(gtk_builder_get_object(builder,"toc_box"));
    GtkWidget *epub_stack = GTK_WIDGET(gtk_builder_get_object(builder,"epub_stack"));
    g_print("%s", g_file_get_path(file));
    epub_init(epub_stack, text_view, toc_box,
              g_file_get_path(file));

  } else if (err) {
    show_error_message(open_data->window, err->message);
  }
}

G_MODULE_EXPORT void on_open_file_clicked(GtkStack *stack,GtkButton *button
                                          ) {

  GtkFileDialog *file_dialog = gtk_file_dialog_new();
  gtk_file_dialog_set_title(file_dialog, "open epub file");
  gtk_file_dialog_set_modal(file_dialog, TRUE);

  GtkFileFilter *file_filter = gtk_file_filter_new();
  gtk_file_filter_set_name(file_filter, "epub files");
  gtk_file_filter_add_pattern(file_filter, "*.epub");
  gtk_file_filter_add_pattern(file_filter, "*.EPUB");

  GListStore *filters = g_list_store_new(gtk_file_filter_get_type());
  g_list_store_append(filters, file_filter);
  gtk_file_dialog_set_filters(file_dialog, G_LIST_MODEL(filters));
  gtk_file_dialog_set_default_filter(file_dialog, file_filter);

  GtkWindow *window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));
  FileOpenData *data = malloc(sizeof(FileOpenData));
  data->window=window;  
  data->stack=stack;  

  gtk_file_dialog_open(file_dialog, window, NULL, on_file_open_response,
                       data);
}

G_MODULE_EXPORT void filter_buttons(GtkSearchEntry *entry, gpointer t) {
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
  builder = gtk_builder_new_from_file("pubby.ui");
  if (!builder) {
    g_print("Error: Could not load pubby.ui\n");
    return;
  }
  GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
  GtkWidget *search_bar = GTK_WIDGET(gtk_builder_get_object(builder, "search_bar"));
  GtkWidget *search_entry = GTK_WIDGET(gtk_builder_get_object(builder, "search_entry"));
  GtkWidget *main_stack = GTK_WIDGET(gtk_builder_get_object(builder,"window_stack"));
  GtkWidget *epub_stack = GTK_WIDGET(gtk_builder_get_object(builder,"epub_stack"));
  GtkWidget *toc_box;
  GtkWidget *toc_button;
  GtkWidget *text_view;

  // connect searchbar to entry
  gtk_search_bar_connect_entry(GTK_SEARCH_BAR(search_bar), GTK_EDITABLE(search_entry));

  gtk_application_add_window(app, GTK_WINDOW(window));
  gtk_window_present(GTK_WINDOW(window));

  GtkEventController *key_controller = gtk_event_controller_key_new();
  gtk_widget_add_controller(window, key_controller);
  g_signal_connect(key_controller, "key-pressed", G_CALLBACK(key_pressed),
                   epub_stack);

  

  /* toc_buttons(stack, */

}

AdwApplication *window_init() {
  AdwApplication *app;
  int status;

  app = adw_application_new("org.owlenz.pubby", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  return app;
}
