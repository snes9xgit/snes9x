/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <stdlib.h>
#include "gtk_builder_window.h"

extern const unsigned char snes9x_ui[];
extern const int snes9x_ui_size;

Glib::RefPtr<Gtk::Builder> global_builder;
bool global_builder_created = false;

GtkBuilderWindow::GtkBuilderWindow(const char *root)
{
    if (!global_builder_created)
    {
        global_builder = Gtk::Builder::create();
        global_builder->add_from_string((const gchar *)snes9x_ui, snes9x_ui_size);
        global_builder_created = true;
    }

    window = get_object<Gtk::Window>(root);
}

GtkBuilderWindow::~GtkBuilderWindow()
{
}

void GtkBuilderWindow::enable_widget(const char *name, bool state)
{
    auto widget = get_object<Gtk::Widget>(name);
    widget->set_sensitive(state);
}

void GtkBuilderWindow::show_widget(const char *name, bool state)
{
    auto widget = get_object<Gtk::Widget>(name);
    widget->set_visible(state);
}

void GtkBuilderWindow::resize(int width, int height)
{
    window->resize(width, height);
}

void GtkBuilderWindow::refresh()
{
    window->queue_draw();
}

int GtkBuilderWindow::get_width()
{
    return window->get_width();
}

int GtkBuilderWindow::get_height()
{
    return window->get_height();
}

void GtkBuilderWindow::set_label(const char * const name, const char * const label)
{
    get_object<Gtk::Label>(name)->set_label(label);
}

void GtkBuilderWindow::set_button_label(const char *name, const char *label)
{
    get_object<Gtk::Button>(name)->set_label(label);
}

bool GtkBuilderWindow::get_check(const char *name)
{
    return get_object<Gtk::ToggleButton>(name)->get_active();
}

int GtkBuilderWindow::get_entry_value(const char *name)
{
    auto text = get_object<Gtk::Entry>(name)->get_text();
    return std::stoi(text);
}

std::string GtkBuilderWindow::get_entry_text(const char *name)
{
    return get_object<Gtk::Entry>(name)->get_text();
}

void GtkBuilderWindow::set_entry_value(const char *name, unsigned int value)
{
    get_object<Gtk::Entry>(name)->set_text(std::to_string(value));
}

void GtkBuilderWindow::set_entry_text(const char *name, const char *text)
{
    get_object<Gtk::Entry>(name)->set_text(text);
}

void GtkBuilderWindow::set_entry_text(const char *name, const std::string &text)
{
    get_object<Gtk::Entry>(name)->set_text(text);
}

float GtkBuilderWindow::get_slider(const char *name)
{
    return get_object<Gtk::Range>(name)->get_value();
}

int GtkBuilderWindow::get_combo(const char *name)
{
    return get_object<Gtk::ComboBox>(name)->get_active_row_number();
}

void GtkBuilderWindow::set_slider(const char *name, float value)
{
    get_object<Gtk::Range>(name)->set_value(value);
}

void GtkBuilderWindow::set_check(const char *name, bool value)
{
    get_object<Gtk::ToggleButton>(name)->set_active(value);
}

void GtkBuilderWindow::set_combo(const char *name, unsigned char value)
{
    get_object<Gtk::ComboBox>(name)->set_active(value);
}

void GtkBuilderWindow::set_spin(const char *name, double value)
{
    get_object<Gtk::SpinButton>(name)->set_value(value);
}

double GtkBuilderWindow::get_spin(const char *name)
{
    return get_object<Gtk::SpinButton>(name)->get_value();
}

void GtkBuilderWindow::combo_box_append(const char *name, const char *value)
{
    return combo_box_append(get_object<Gtk::ComboBox>(name).get(), value);
}

void GtkBuilderWindow::combo_box_append(Gtk::ComboBox *combo, const char *value)
{
    GtkListStore *store;
    GtkTreeIter iter;

    store = GTK_LIST_STORE(combo->get_model()->gobj());

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, value, -1);
}

GtkWindow *GtkBuilderWindow::get_window()
{
    return window->gobj();
}

bool GtkBuilderWindow::has_focus(const char *widget)
{
    return get_object<Gtk::Widget>(widget)->is_focus();
}
