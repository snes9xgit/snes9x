#include <stdlib.h>
#include "gtk_glade_window.h"

GladeWindow::GladeWindow (const char *buffer, int size, const char *root)
{
    glade = glade_xml_new_from_buffer (buffer, size, root, NULL);

    window = get_widget (root);

    return;
}

GtkWidget *
GladeWindow::get_widget (const char *name)
{
    return glade_xml_get_widget (glade, name);
}

void
GladeWindow::signal_connect (const char *name, GCallback func)
{
    glade_xml_signal_connect_data (glade, name, func, (gpointer) this);

    return;
}

void
GladeWindow::signal_connect (const char *name, GCallback func, gpointer data)
{
    glade_xml_signal_connect_data (glade, name, func, (gpointer) data);

    return;
}

void
GladeWindow::signal_connect (GladeWindowCallbacks *callbacks)
{
    if (!callbacks)
        return;

    for (int i = 0; callbacks[i].signal; i++)
    {
        signal_connect (callbacks[i].signal, callbacks[i].function);
    }

    return;
}

void
GladeWindow::enable_widget (const char *name, unsigned char state)
{
    gtk_widget_set_sensitive (get_widget (name), state);

    return;
}

void
GladeWindow::resize (int width, int height)
{
    gtk_window_resize (GTK_WINDOW (window), width, height);

    return;
}

void
GladeWindow::refresh (void)
{
    gtk_widget_queue_draw (GTK_WIDGET (window));

    return;
}

int
GladeWindow::get_width (void)
{
    return window->allocation.width;
}

int
GladeWindow::get_height (void)
{
    return window->allocation.height;
}

void
GladeWindow::set_button_label (const char *name, const char *label)
{
    gtk_button_set_label (GTK_BUTTON (get_widget (name)), label);

    return;
}

unsigned char
GladeWindow::get_check (const char *name)
{
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (get_widget (name))))
        return 1;
    return 0;
}

unsigned int
GladeWindow::get_entry_value (const char *name)
{
    return atoi (gtk_entry_get_text (GTK_ENTRY (get_widget (name))));;
}

const char *
GladeWindow::get_entry_text (const char *name)
{
    return gtk_entry_get_text (GTK_ENTRY (get_widget (name)));
}

float
GladeWindow::get_slider (const char *name)
{
    return (float) gtk_range_get_value (GTK_RANGE (get_widget (name)));
}

unsigned char
GladeWindow::get_combo (const char *name)
{
    return gtk_combo_box_get_active (GTK_COMBO_BOX (get_widget (name)));
}

void
GladeWindow::set_slider (const char *name, float value)
{
    gtk_range_set_value (GTK_RANGE (get_widget (name)), (double) value);

    return;
}

void
GladeWindow::set_check (const char *name, unsigned char value)
{
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (get_widget (name)),
                                  value);
    return;
}

void
GladeWindow::set_entry_value (const char *name, unsigned int value)
{
    char text[80];

    snprintf (text, 80, "%u", value);
    gtk_entry_set_text (GTK_ENTRY (get_widget (name)), text);

    return;
}

void
GladeWindow::set_entry_text (const char *name, const char *text)
{
    gtk_entry_set_text (GTK_ENTRY (get_widget (name)), text);

    return;
}

void
GladeWindow::set_combo (const char *name, unsigned char value)
{
    gtk_combo_box_set_active (GTK_COMBO_BOX (get_widget (name)), value);

    return;
}

void
GladeWindow::set_spin (const char *name, unsigned int value)
{
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (get_widget (name)),
                               (double) value);

    return;
}

GtkWindow *
GladeWindow::get_window (void)
{
    return GTK_WINDOW (window);
}

unsigned int
GladeWindow::get_spin (const char *name)
{
    return (unsigned int)
        gtk_spin_button_get_value (GTK_SPIN_BUTTON (get_widget (name)));
}

int
GladeWindow::has_focus (const char *widget)
{
    return gtk_widget_is_focus (get_widget (widget));
}
