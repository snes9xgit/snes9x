#ifndef __GTK_PREFERENCES_H
#define __GTK_PREFERENCES_H

#include <gtk/gtk.h>
#include "gtk_s9x.h"
#include "gtk_builder_window.h"

gboolean snes9x_preferences_open (GtkWidget *widget,
                                  gpointer data);

class Snes9xPreferences : public GtkBuilderWindow
{
    public:
        Snes9xPreferences (Snes9xConfig *config);
        ~Snes9xPreferences (void);
        void show (void);
        void bindings_to_dialog (int joypad);
        int get_focused_binding (void);
        void store_binding (const char *string, Binding binding);
        void browse_folder_dialog (void);
        int hw_accel_value (int combo_value);
        int combo_value (int hw_accel);
        void focus_next (void);
        void swap_with (void);
        void reset_current_joypad (void);
        void load_ntsc_settings (void);
        void store_ntsc_settings (void);

#ifdef USE_JOYSTICK
        void calibration_dialog (void);
#endif

        Snes9xConfig    *config;
        GtkToggleButton *last_toggled;
        int             awaiting_key;
        int             polling_joystick;
        JoypadBinding   pad[NUM_JOYPADS];
        Binding         shortcut[NUM_EMU_LINKS];

    private:
        void get_settings_from_dialog (void);
        void move_settings_to_dialog (void);
};

#endif /* __GTK_PREFERENCES_H */
