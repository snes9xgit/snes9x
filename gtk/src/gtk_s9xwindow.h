/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_S9XWINDOW_H
#define __GTK_S9XWINDOW_H

#include "gtk_2_3_compat.h"

#include "port.h"
#include "gtk_builder_window.h"
#include "gtk_config.h"

class Snes9xWindow : public GtkBuilderWindow
{
    public:
        Snes9xWindow (Snes9xConfig *config);

        /* Pause related functions */
        void pause_from_focus_change ();
        void unpause_from_focus_change ();
        void focus_notify (bool state);
        void pause_from_user ();
        void unpause_from_user ();
        bool is_paused ();
        void propagate_pause_state ();

        /* Fullscreen functions */
        void enter_fullscreen_mode ();
        void leave_fullscreen_mode ();
        void toggle_fullscreen_mode ();
        void finish_fullscreen ();

        /* Cursor modifying functions */
        void show_mouse_cursor ();
        void hide_mouse_cursor ();
        void toggle_grab_mouse ();
        void center_mouse ();

        /* Rom-related functions */
        void open_rom_dialog ();
        void save_state_dialog ();
        void load_state_dialog ();
        void configure_widgets ();
        void save_spc_dialog ();
        bool try_open_rom (const char *filename);
        const char *open_movie_dialog (bool readonly);
        void movie_seek_dialog ();
        void open_multicart_dialog ();
        void show_rom_info ();

        /* GTK-base-related functions */
        void show ();
        void set_menu_item_selected (const char *name);
        void set_mouseable_area (int x, int y, int width, int height);
        void set_menu_item_accel_to_binding (const char *name,
                                             const char *binding);
        void reset_screensaver ();
        void update_accels ();
        void toggle_ui ();
        void resize_to_multiple (int factor);
        void resize_viewport (int width, int height);
        void expose ();
        void setup_splash();
        double get_refresh_rate ();
        int get_auto_input_rate ();

        cairo_t *get_cairo ();
        void release_cairo ();

        Snes9xConfig   *config;
        int            user_pause, sys_pause;
        int            last_width, last_height;
        int            mouse_region_x, mouse_region_y;
        int            mouse_region_width, mouse_region_height;
        int            nfs_width, nfs_height, nfs_x, nfs_y;
        int            fullscreen_state;
        int            maximized_state;
        bool           focused;
        bool           paused_from_focus_loss;
        double         snes_mouse_x, snes_mouse_y;
        double         gdk_mouse_x, gdk_mouse_y;
        bool           mouse_grabbed;
        GdkPixbuf      *icon, *splash;
        GdkCursor      *default_cursor, *empty_cursor;
        GtkDrawingArea *drawing_area;
        GtkWidget      *recent_menu;
        cairo_t        *cr;
        bool           cairo_owned;
#if GTK_MAJOR_VERSION >= 3
        GdkDrawingContext *gdk_drawing_context;
        cairo_region_t    *cairo_region;
#endif
};

typedef struct gtk_splash_t {
    unsigned int  width;
    unsigned int  height;
    unsigned int  bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
    unsigned char pixel_data[256 * 224 * 3 + 1];
} gtk_splash_t;

#endif /* __GTK_S9XWINDOW_H */
