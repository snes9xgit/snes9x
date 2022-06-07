/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_PREFERENCES_H
#define __GTK_PREFERENCES_H

#include "gtk_compat.h"
#include "gtk_s9x.h"
#include "gtk_builder_window.h"

void snes9x_preferences_create(Snes9xConfig *config);
void snes9x_preferences_open(Snes9xWindow *window);

class Snes9xPreferences : public GtkBuilderWindow
{
  public:
    Snes9xPreferences(Snes9xConfig *config);
    ~Snes9xPreferences();
    void show();
    void bindings_to_dialog(int joypad);
    int get_focused_binding();
    void store_binding(const char *string, Binding binding);
    int hw_accel_value(int combo_value);
    int combo_value(std::string driver_name);
    void focus_next();
    void swap_with();
    void clear_binding(const char *name);
    void reset_current_joypad();
    void load_ntsc_settings();
    void store_ntsc_settings();
    void calibration_dialog();
    void connect_signals();
    void input_rate_changed();
    bool key_pressed(GdkEventKey *event);
    void scale_method_changed();
    void shader_select();
    void game_data_browse(std::string folder);
    void about_dialog();

    Snes9xConfig *config;
    bool awaiting_key;
    bool polling_joystick;
    std::array<JoypadBinding, NUM_JOYPADS> pad;
    std::array<Binding, NUM_EMU_LINKS> shortcut;

  private:
    void get_settings_from_dialog();
    void move_settings_to_dialog();
};

#endif /* __GTK_PREFERENCES_H */
