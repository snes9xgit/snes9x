#include <stdlib.h>
#include <string.h>
#include "gtk_builder_window.h"

extern const unsigned char snes9x_ui[];
extern const int           snes9x_ui_size;

GtkBuilderWindow::GtkBuilderWindow (const char *root)
{
    builder = gtk_builder_new ();
    gtk_builder_add_from_string (builder,
                                 (const gchar *) snes9x_ui,
                                 snes9x_ui_size,
                                 NULL);

    window = get_widget (root);

    return;
}

GtkBuilderWindow::~GtkBuilderWindow (void)
{
    gtk_widget_destroy (window);
    g_object_unref (builder);
}

GtkWidget *
GtkBuilderWindow::get_widget (const char *name)
{
    return GTK_WIDGET (gtk_builder_get_object (builder, name));
}

void
GtkBuilderWindow::signal_connection_func (GtkBuilder *builder,
                                          GObject *object,
                                          const gchar *signal_name,
                                          const char *handler_name,
                                          GObject *connect_object,
                                          GConnectFlags flags,
                                          gpointer data)
{
    GCallback function = NULL;
    GtkBuilderWindow *window = (GtkBuilderWindow *) data;
    GtkBuilderWindowCallbacks *callbacks = window->callbacks;

    for (int i = 0; callbacks[i].signal; i++)
    {
        if (!strcmp (handler_name, callbacks[i].signal))
        {
            function = callbacks[i].function;
            break;
        }
    }

    if (function)
    {
        if (connect_object)
        {
            fprintf (stderr, "Error: found a persistent object signal.\n");
            g_signal_connect_object (object,
                                     signal_name,
                                     function,
                                     connect_object,
                                     flags);
        }
        else
        {
            g_signal_connect_data (object,
                                   signal_name,
                                   function,
                                   data,
                                   NULL,
                                   flags);
        }
    }
    else
    {
    }

    return;
}

void
GtkBuilderWindow::signal_connect (GtkBuilderWindowCallbacks *callbacks)
{
    if (!callbacks)
        return;

    this->callbacks = callbacks;

    gtk_builder_connect_signals_full (builder,
                                      signal_connection_func,
                                      (gpointer) this);

    this->callbacks = NULL;

    return;
}

void
GtkBuilderWindow::enable_widget (const char *name, unsigned char state)
{
    gtk_widget_set_sensitive (get_widget (name), state);

    return;
}

void
GtkBuilderWindow::resize (int width, int height)
{
    if (width > 0 && height > 0)
        gtk_window_resize (GTK_WINDOW (window), width, height);

    return;
}

void
GtkBuilderWindow::refresh (void)
{
    gtk_widget_queue_draw (GTK_WIDGET (window));

    return;
}

int
GtkBuilderWindow::get_width (void)
{
    int width, height;

    gtk_window_get_size (GTK_WINDOW (window), &width, &height);

    return width;
}

int
GtkBuilderWindow::get_height (void)
{
    int width, height;

    gtk_window_get_size (GTK_WINDOW (window), &width, &height);

    return height;
}

void
GtkBuilderWindow::set_button_label (const char *name, const char *label)
{
    gtk_button_set_label (GTK_BUTTON (get_widget (name)), label);

    return;
}

unsigned char
GtkBuilderWindow::get_check (const char *name)
{
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (get_widget (name))))
        return 1;
    return 0;
}

unsigned int
GtkBuilderWindow::get_entry_value (const char *name)
{
    return atoi (gtk_entry_get_text (GTK_ENTRY (get_widget (name))));;
}

const char *
GtkBuilderWindow::get_entry_text (const char *name)
{
    return gtk_entry_get_text (GTK_ENTRY (get_widget (name)));
}

float
GtkBuilderWindow::get_slider (const char *name)
{
    return (float) gtk_range_get_value (GTK_RANGE (get_widget (name)));
}

unsigned char
GtkBuilderWindow::get_combo (const char *name)
{
    return gtk_combo_box_get_active (GTK_COMBO_BOX (get_widget (name)));
}

void
GtkBuilderWindow::set_slider (const char *name, float value)
{
    gtk_range_set_value (GTK_RANGE (get_widget (name)), (double) value);

    return;
}

void
GtkBuilderWindow::set_check (const char *name, unsigned char value)
{
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (get_widget (name)),
                                  value);
    return;
}

void
GtkBuilderWindow::set_entry_value (const char *name, unsigned int value)
{
    char text[80];

    snprintf (text, 80, "%u", value);
    gtk_entry_set_text (GTK_ENTRY (get_widget (name)), text);

    return;
}

void
GtkBuilderWindow::set_entry_text (const char *name, const char *text)
{
    gtk_entry_set_text (GTK_ENTRY (get_widget (name)), text);

    return;
}

void
GtkBuilderWindow::set_combo (const char *name, unsigned char value)
{
    gtk_combo_box_set_active (GTK_COMBO_BOX (get_widget (name)), value);

    return;
}

void
GtkBuilderWindow::set_spin (const char *name, unsigned int value)
{
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (get_widget (name)),
                               (double) value);

    return;
}

void
GtkBuilderWindow::combo_box_append (const char *name, const char *value)
{
    combo_box_append (GTK_COMBO_BOX (get_widget (name)), value);
}

void
GtkBuilderWindow::combo_box_append (GtkComboBox *combo, const char *value)
{
    GtkListStore *store;
    GtkTreeIter iter;

    store = GTK_LIST_STORE (gtk_combo_box_get_model (combo));

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter, 0, value, -1);

    return;
}

GtkWindow *
GtkBuilderWindow::get_window (void)
{
    return GTK_WINDOW (window);
}

unsigned int
GtkBuilderWindow::get_spin (const char *name)
{
    return (unsigned int)
        gtk_spin_button_get_value (GTK_SPIN_BUTTON (get_widget (name)));
}

int
GtkBuilderWindow::has_focus (const char *widget)
{
    return gtk_widget_is_focus (get_widget (widget));
}
