#ifndef __GTK_S9XWINDOW_H
#define __GTK_S9XWINDOW_H

#include <gtk/gtk.h>

#ifdef USE_OPENGL
#include <GL/gl.h>
#endif

#include "gtk_s9x.h"
#include "gtk_builder_window.h"

class Snes9xWindow : public GtkBuilderWindow
{
    public:
        Snes9xWindow (Snes9xConfig *config);

        /* Pause related functions */
        void pause_from_focus_change (void);
        void unpause_from_focus_change (void);
        void focus_notify (int state);
        void pause_from_user (void);
        void unpause_from_user (void);
        unsigned char is_paused (void);
        void propagate_pause_state (void);

        /* Fullscreen functions */
        void enter_fullscreen_mode (void);
        void leave_fullscreen_mode (void);
        void toggle_fullscreen_mode (void);
        void finish_fullscreen (void);

        /* Cursor modifying functions */
        void show_mouse_cursor (void);
        void hide_mouse_cursor (void);

        /* Rom-related functions */
        void open_rom_dialog (void);
        void save_state_dialog (void);
        void load_state_dialog (void);
        void configure_widgets (void);
        void save_spc_dialog (void);
        int try_open_rom (const char *filename);
        const char *open_movie_dialog (bool readonly);
        void movie_seek_dialog (void);
        void open_multicart_dialog (void);
        void show_rom_info (void);

        /* GTK-base-related functions */
        void show (void);
        void show_status_message (const char *message);
        void update_statusbar (void);
        void toggle_statusbar (void);
        void set_menu_item_selected (const char *name);
        void set_mouseable_area (int x, int y, int width, int height);
        void set_menu_item_accel_to_binding (const char *name,
                                             const char *binding);
        void reset_screensaver (void);
        void update_accels (void);
        void toggle_ui (void);
        void resize_to_multiple (int factor);
        void resize_viewport (int width, int height);
        void expose (void);

        Snes9xConfig   *config;
        int            user_pause, sys_pause;
        int            user_rewind;
        int            last_width, last_height;
        int            mouse_region_x, mouse_region_y;
        int            mouse_region_width, mouse_region_height;
        int            nfs_width, nfs_height, nfs_x, nfs_y;
        int            fullscreen_state;
        int            maximized_state;
        int            focused;
        int            paused_from_focus_loss;
        uint16         mouse_loc_x, mouse_loc_y;
        GdkPixbuf      *icon, *splash;
        GdkCursor      *default_cursor, *empty_cursor;
        GtkDrawingArea *drawing_area;
        GtkWidget      *recent_menu;
};

typedef struct gtk_splash_t {
    unsigned int  width;
    unsigned int  height;
    unsigned int  bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
    unsigned char pixel_data[256 * 224 * 3 + 1];
} gtk_splash_t;

#endif /* __GTK_S9XWINDOW_H */
