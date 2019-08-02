/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_PREFERENCES_H
#define __GTK_PREFERENCES_H

#include "gtk_2_3_compat.h"
#include "gtk_s9x.h"
#include "gtk_builder_window.h"

gboolean snes9x_preferences_open (GtkWidget *widget,
                                  gpointer data);

class Snes9xPreferences : public GtkBuilderWindow
{
    public:
        Snes9xPreferences (Snes9xConfig *config);
        ~Snes9xPreferences ();
        void show ();
        void bindings_to_dialog (int joypad);
        int get_focused_binding ();
        void store_binding (const char *string, Binding binding);
        void browse_folder_dialog ();
        int hw_accel_value (int combo_value);
        int combo_value (int hw_accel);
        void focus_next ();
        void swap_with ();
        void clear_binding (const char *name);
        void reset_current_joypad ();
        void load_ntsc_settings ();
        void store_ntsc_settings ();
        void calibration_dialog ();

        Snes9xConfig    *config;
        GtkToggleButton *last_toggled;
        bool            awaiting_key;
        bool            polling_joystick;
        JoypadBinding   pad[NUM_JOYPADS];
        Binding         shortcut[NUM_EMU_LINKS];

    private:
        void get_settings_from_dialog ();
        void move_settings_to_dialog ();

        unsigned int *mode_indices;
};

#endif /* __GTK_PREFERENCES_H */
