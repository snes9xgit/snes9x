/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_BUILDER_WINDOW_H
#define __GTK_BUILDER_WINDOW_H

#include "gtk_compat.h"

extern Glib::RefPtr<Gtk::Builder> global_builder;

class GtkBuilderWindow
{
  public:
    GtkBuilderWindow(const char *root);
    ~GtkBuilderWindow();

    template <typename T>
    Glib::RefPtr<T> get_object(const char *name)
    {
        auto object = global_builder->get_object(name);
        if (!object)
        {
            printf("No object named %s\n", name);
            exit(1);
        }
        return Glib::RefPtr<T>::cast_static(object);
    }

    void resize(int width, int height);
    GtkWindow *get_window();
    void refresh();
    int get_width();
    int get_height();

    void enable_widget(const char *name, bool state);
    void show_widget(const char *name, bool state);
    void set_button_label(const char *name, const char *label);
    bool get_check(const char *name);
    int get_entry_value(const char *name);
    std::string get_entry_text(const char *name);
    int get_combo(const char *name);
    void combo_box_append(const char *name, const char *value);
    void combo_box_append(Gtk::ComboBox *combo, const char *value);
    double get_spin(const char *name);
    float get_slider(const char *name);
    void set_check(const char *name, bool value);
    void set_entry_value(const char *name, unsigned int value);
    void set_entry_text(const char *name, const char *text);
    void set_combo(const char *name, unsigned char value);
    void set_spin(const char *name, double value);
    void set_slider(const char *name, float value);
    bool has_focus(const char *widget);

    Glib::RefPtr<Gtk::Window> window;
};

#endif /* __GTK_BUILDER_WINDOW_H */
