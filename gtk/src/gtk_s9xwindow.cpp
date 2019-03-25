/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_2_3_compat.h"
#include "gtk_config.h"
#ifdef GDK_WINDOWING_X11
#include <X11/Xatom.h>
#endif
#include <gdk/gdkkeysyms.h>
#include <cairo.h>

#ifdef USE_XV
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>
#endif

#ifdef USE_OPENGL
#include "gtk_shader_parameters.h"
#endif

#include "gtk_s9x.h"
#include "gtk_preferences.h"
#include "gtk_icon.h"
#include "gtk_display.h"
#include "gtk_file.h"
#include "gtk_sound.h"
#include "gtk_control.h"
#include "gtk_cheat.h"
#include "gtk_netplay.h"

static gboolean
event_main_window_delete (GtkWidget *widget,
                          GdkEvent  *event,
                          gpointer  data)
{
    S9xExit ();

    return true;
}

static gboolean
event_main_window_state_event (GtkWidget           *widget,
                               GdkEventWindowState *event,
                               gpointer            data)
{
    Snes9xWindow *window = (Snes9xWindow *) data;
    window->fullscreen_state = event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN;
    window->maximized_state  = event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED;

    return false;
}

static gboolean
event_continue_item_activate (GtkWidget *widget, gpointer data)
{
    Snes9xWindow *window = (Snes9xWindow *) data;

    window->unpause_from_user ();

    return true;
}

static gboolean
event_open_cheats (GtkWidget *widget, gpointer data)
{
    Snes9xCheats *cheats;
    cheats = new Snes9xCheats ();

    cheats->show ();
    delete cheats;

    return true;
}

static gboolean
event_open_multicart (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->open_multicart_dialog ();

    return true;
}

static gboolean
event_rom_info (GtkWidget *widget, gpointer data)
{
    Snes9xWindow *window = (Snes9xWindow *) data;

    window->show_rom_info ();

    return true;
}

static gboolean
event_toggle_interface (GtkWidget *widget, gpointer data)
{
    Snes9xWindow *window = (Snes9xWindow *) data;

    window->toggle_ui ();

    return true;
}

static gboolean
event_sync_clients (GtkWidget *widget, gpointer data)
{
    S9xNetplaySyncClients ();

    return true;
}

static gboolean
event_pause_item_activate (GtkWidget *widget, gpointer data)
{
    Snes9xWindow *window = (Snes9xWindow *) data;

    window->pause_from_user ();

    return true;
}

static gboolean
event_open_netplay (GtkWidget *widget, gpointer data)
{
    S9xNetplayDialogOpen ();

    return true;
}

#if GTK_MAJOR_VERSION >= 3
static gboolean
event_drawingarea_draw (GtkWidget *widget,
                        cairo_t   *cr,
                        gpointer  data)
{

    Snes9xWindow *window = (Snes9xWindow *) data;
    window->cr = cr;
    window->cairo_owned = false;
    window->expose ();
    window->cr = NULL;

    return true;
}

#else

static gboolean
event_drawingarea_expose (GtkWidget      *widget,
                          GdkEventExpose *event,
                          gpointer       data)
{
    ((Snes9xWindow *) data)->expose ();

    return true;
}
#endif

static gboolean
event_key (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    Snes9xWindow        *window = (Snes9xWindow *) data;
    static unsigned int keyval  = 0;
    static GdkEventType type    = GDK_NOTHING;
    Binding             b;
    s9xcommand_t        cmd;

    /* Ignore multiple identical keypresses to discard repeating keys */
    if (event->keyval == keyval && event->type == type)
    {
        return true;
    }

    /* Provide escape key to get out of fullscreen */
    if (event->keyval == GDK_Escape)
    {
        if (event->type == GDK_KEY_RELEASE)
        {
            if (window->config->default_esc_behavior == ESC_EXIT_FULLSCREEN)
                window->leave_fullscreen_mode ();
            else if (window->config->default_esc_behavior == ESC_EXIT_SNES9X)
                S9xExit ();
            else
                window->toggle_ui ();
        }

        return true;
    }

    keyval = event->keyval;
    type = event->type;

    b = Binding (event);

    /* If no mapping for modifier version exists, try non-modifier */
    cmd = S9xGetMapping (b.hex ());
    if (cmd.type == S9xNoMapping)
    {
        b = Binding (event->keyval, false, false, false);
        cmd = S9xGetMapping (b.hex ());
    }

    if (cmd.type != S9xNoMapping)
    {
        S9xReportButton (b.hex (), (event->type == GDK_KEY_PRESS));
        return true;
    }

    return false; /* Pass the key to GTK */
}

gboolean
event_motion_notify (GtkWidget      *widget,
                     GdkEventMotion *event,
                     gpointer       user_data)
{
    Snes9xWindow *window = (Snes9xWindow *) user_data;

    if (!window->config->rom_loaded ||
        window->last_width <= 0     ||
        window->last_height <= 0)
    {
        return false;
    }

    if (window->mouse_grabbed)
    {
        if (event->x_root == window->gdk_mouse_x &&
            event->y_root == window->gdk_mouse_y)
            return false;

        window->snes_mouse_x += (event->x_root - window->gdk_mouse_x);
        window->snes_mouse_y += (event->y_root - window->gdk_mouse_y);
        window->center_mouse ();

        return false;
    }

#if GTK_CHECK_VERSION(3,10,0)
    int scale_factor = gdk_window_get_scale_factor (gtk_widget_get_window (GTK_WIDGET (window->get_window ())));
#else
    int scale_factor = 1;
#endif

    window->snes_mouse_x = (uint16)
        ((int) (event->x * scale_factor) - window->mouse_region_x) * 256 /
        (window->mouse_region_width <= 0 ? 1 : window->mouse_region_width);

    window->snes_mouse_y = (uint16)
        ((int) (event->y * scale_factor) - window->mouse_region_y) * (gui_config->overscan ? SNES_HEIGHT_EXTENDED : SNES_HEIGHT) /
        (window->mouse_region_height <= 0 ? 1 : window->mouse_region_height);

    if (!window->config->pointer_is_visible)
    {
        if (!S9xIsMousePluggedIn ())
            window->show_mouse_cursor ();
    }

    window->config->pointer_timestamp = g_get_monotonic_time ();

    return false;
}

gboolean
event_button_press (GtkWidget      *widget,
                    GdkEventButton *event,
                    gpointer       user_data)
{
    auto window = (Snes9xWindow *)user_data;

    if (S9xIsMousePluggedIn())
    {
        switch (event->button)
        {
        case 1:
            S9xReportButton(BINDING_MOUSE_BUTTON0, 1);
            break;
        case 2:
            S9xReportButton(BINDING_MOUSE_BUTTON2, 1);
            break;
        case 3:
            S9xReportButton(BINDING_MOUSE_BUTTON1, 1);
            break;
        }
    }
    else if (event->button == 3)
    {
#if GTK_MAJOR_VERSION >= 3
        gtk_menu_popup_at_pointer(GTK_MENU(window->get_widget("view_menu_menu")), NULL);
#else
        gtk_menu_popup(GTK_MENU(window->get_widget("view_menu_menu")), NULL,
                       NULL, NULL, NULL, 3, event->time);
#endif
    }
    return false;
}

gboolean
event_button_release (GtkWidget      *widget,
                      GdkEventButton *event,
                      gpointer       user_data)
{
    switch (event->button)
    {
        case 1:
            S9xReportButton (BINDING_MOUSE_BUTTON0, 0);
            break;
        case 2:
            S9xReportButton (BINDING_MOUSE_BUTTON1, 0);
            break;
        case 3:
            S9xReportButton (BINDING_MOUSE_BUTTON2, 0);
            break;
    }

    return false;
}

static void
event_fullscreen (GtkWidget *widget, gpointer data)
{
    Snes9xWindow *window = (Snes9xWindow *) data;

    if (!window->config->fullscreen)
        window->enter_fullscreen_mode ();
    else
        window->leave_fullscreen_mode ();
}


static void
event_exact_pixels_1x (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->resize_to_multiple (1);
}

static void
event_exact_pixels_2x (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->resize_to_multiple (2);
}

static void
event_exact_pixels_3x (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->resize_to_multiple (3);
}

static void
event_exact_pixels_4x (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->resize_to_multiple (4);
}

static void
event_exact_pixels_5x (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->resize_to_multiple (5);
}

static void
event_record_movie (GtkWidget *widget, gpointer data)
{
    if (S9xMovieActive ())
        S9xMovieStop (false);

    S9xMovieCreate (S9xChooseMovieFilename (false),
                    0xFF,
                    MOVIE_OPT_FROM_RESET,
                    NULL,
                    0);
}

static void
event_open_movie (GtkWidget *widget, gpointer data)
{
    if (S9xMovieActive ())
        S9xMovieStop (false);

    S9xMovieOpen (S9xChooseMovieFilename (true), false);
}

static void
event_shader_parameters (GtkWidget *widget, gpointer data)
{
#ifdef USE_OPENGL
    Snes9xWindow *window = (Snes9xWindow *) data;

    gtk_shader_parameters_dialog (window->get_window ());
#endif
}

static void
event_stop_recording (GtkWidget *widget, gpointer data)
{
    if (S9xMovieActive ())
        S9xMovieStop (false);
}

static void
event_jump_to_frame (GtkWidget *widget, gpointer data)
{
    Snes9xWindow *window = (Snes9xWindow *) data;

    window->movie_seek_dialog ();
}

static void
event_reset (GtkWidget *widget, gpointer data)
{
    S9xSoftReset ();
}

static void
event_hard_reset (GtkWidget *widget, gpointer data)
{
    S9xReset ();
}

static void
event_save_state (GtkWidget *widget, gpointer data)
{
    int  slot;
    char *name = (char *) gtk_buildable_get_name (GTK_BUILDABLE (widget));

    slot = atoi (&(name[11]));

    S9xQuickSaveSlot (slot);
}

static void
event_save_state_file (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->save_state_dialog ();
}

static void
event_load_state (GtkWidget *widget, gpointer data)
{
    int  slot;
    char *name = (char *) gtk_buildable_get_name (GTK_BUILDABLE (widget));

    slot = atoi (&(name[11]));

    S9xQuickLoadSlot (slot);
}

static void
event_load_state_undo (GtkWidget *widget, gpointer data)
{
    S9xUnfreezeGame (S9xGetFilename (".undo", SNAPSHOT_DIR));
}


static void
event_load_state_file (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->load_state_dialog ();
}

static void
event_open_rom (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->open_rom_dialog ();
}

static void
event_recent_open (GtkRecentChooser *chooser, gpointer data)
{
    Snes9xWindow *window   = (Snes9xWindow *) data;
    gchar        *uri      = gtk_recent_chooser_get_current_uri (chooser);
    gchar        *filename = g_filename_from_uri (uri, NULL, NULL);

    window->try_open_rom (filename);

    g_free (filename);
    g_free (uri);
}

static void
event_save_spc (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->save_spc_dialog ();
}

static gboolean
event_focus_in (GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
    ((Snes9xWindow *) data)->focus_notify (true);

    return false;
}

static gboolean
event_focus_out (GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
    ((Snes9xWindow *) data)->focus_notify (false);

    return false;
}

static void
event_port (GtkWidget *widget, gpointer data)
{
    const gchar *name = gtk_buildable_get_name (GTK_BUILDABLE (widget));

    if (!(gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))))
        return;

    if (!strcasecmp (name, "joypad1"))
    {
        S9xSetController (0, CTL_JOYPAD, 0, 0, 0, 0);
    }

    else if (!strcasecmp (name, "joypad2"))
    {
        S9xSetController (1, CTL_JOYPAD, 1, 0, 0, 0);
    }

    else if (!strcasecmp (name, "mouse1"))
    {
        S9xSetController (0, CTL_MOUSE, 0, 0, 0, 0);
    }

    else if (!strcasecmp (name, "mouse2"))
    {
        S9xSetController (1, CTL_MOUSE, 0, 0, 0, 0);
    }

    else if (!strcasecmp (name, "superscope1"))
    {
        S9xSetController (0, CTL_SUPERSCOPE, 0, 0, 0, 0);
    }

    else if (!strcasecmp (name, "superscope2"))
    {
        S9xSetController (1, CTL_SUPERSCOPE, 0, 0, 0, 0);
    }

    else if (!strcasecmp (name, "multitap1"))
    {
        S9xSetController (0, CTL_MP5, 0, 1, 2, 3);
    }

    else if (!strcasecmp (name, "multitap2"))
    {
        S9xSetController (1, CTL_MP5, 1, 2, 3, 4);
    }

    else if (!strcasecmp (name, "nothingpluggedin2"))
    {
        S9xSetController (1, CTL_NONE, 0, 0, 0, 0);
    }
}

Snes9xWindow::Snes9xWindow (Snes9xConfig *config) :
    GtkBuilderWindow ("main_window")
{
    GtkBuilderWindowCallbacks callbacks[] =
    {
        { "main_window_delete_event", G_CALLBACK (event_main_window_delete) },
        { "main_window_state_event", G_CALLBACK (event_main_window_state_event) },
        { "on_continue_item_activate", G_CALLBACK (event_continue_item_activate) },
        { "on_pause_item_activate", G_CALLBACK (event_pause_item_activate) },
        { "main_window_key_press_event", G_CALLBACK (event_key) },
        { "main_window_key_release_event", G_CALLBACK (event_key) },
        { "on_fullscreen_item_activate", G_CALLBACK (event_fullscreen) },
        { "on_open_rom_activate", G_CALLBACK (event_open_rom) },
        { "on_reset_item_activate", G_CALLBACK (event_reset) },
        { "on_shader_parameters_item_activate", G_CALLBACK (event_shader_parameters) },
        { "hard_reset", G_CALLBACK (event_hard_reset) },
        { "on_port_activate", G_CALLBACK (event_port) },
        { "load_save_state", G_CALLBACK (event_load_state) },
        { "load_state_file", G_CALLBACK (event_load_state_file) },
        { "load_state_undo", G_CALLBACK (event_load_state_undo) },
        { "save_save_state", G_CALLBACK (event_save_state) },
        { "save_state_file", G_CALLBACK (event_save_state_file) },
        { "drawingarea_button_press", G_CALLBACK (event_button_press) },
        { "drawingarea_button_release", G_CALLBACK (event_button_release) },
        { "drawingarea_motion_notify", G_CALLBACK (event_motion_notify) },
        { "save_spc", G_CALLBACK (event_save_spc) },
        { "open_movie", G_CALLBACK (event_open_movie) },
        { "stop_recording", G_CALLBACK (event_stop_recording) },
        { "jump_to_frame", G_CALLBACK (event_jump_to_frame) },
        { "record_movie", G_CALLBACK (event_record_movie) },
        { "open_cheats", G_CALLBACK (event_open_cheats) },
        { "on_preferences_item_activate", G_CALLBACK (snes9x_preferences_open) },
        { "focus_in_event", G_CALLBACK (event_focus_in) },
        { "focus_out_event", G_CALLBACK (event_focus_out) },
        { "open_netplay", G_CALLBACK (event_open_netplay) },
        { "rom_info", G_CALLBACK (event_rom_info) },
        { "sync_clients", G_CALLBACK (event_sync_clients) },
        { "toggle_interface", G_CALLBACK (event_toggle_interface) },
        { "exact_1x", G_CALLBACK (event_exact_pixels_1x) },
        { "exact_2x", G_CALLBACK (event_exact_pixels_2x) },
        { "exact_3x", G_CALLBACK (event_exact_pixels_3x) },
        { "exact_4x", G_CALLBACK (event_exact_pixels_4x) },
        { "exact_5x", G_CALLBACK (event_exact_pixels_5x) },
        { "open_multicart", G_CALLBACK (event_open_multicart) },

        { NULL, NULL }
    };

    user_pause             = 0;
    sys_pause              = 0;
    last_width             = -1;
    last_height            = -1;
    this->config           = config;
    empty_cursor           = NULL;
    default_cursor         = NULL;
    recent_menu            = NULL;
    fullscreen_state       = 0;
    maximized_state        = 0;
    focused                = true;
    paused_from_focus_loss = false;
    cr                     = NULL;
    cairo_owned            = false;
    mouse_grabbed          = false;

    if (gtk_icon_theme_has_icon (gtk_icon_theme_get_default (), "snes9x"))
    {
        gtk_window_set_default_icon_name ("snes9x");
    }
    else
    {
        GdkPixbufLoader *loader = gdk_pixbuf_loader_new ();
        if (gdk_pixbuf_loader_write (loader, (const guchar *)app_icon, sizeof (app_icon), NULL) &&
            gdk_pixbuf_loader_close (loader, NULL) &&
            (icon = gdk_pixbuf_loader_get_pixbuf (loader)))
        {
            gtk_window_set_default_icon (icon);
        }
        g_object_unref(loader);
    }

    drawing_area = GTK_DRAWING_AREA (get_widget ("drawingarea"));
#if GTK_MAJOR_VERSION < 3
    gtk_widget_set_double_buffered (GTK_WIDGET (drawing_area), false);
    gtk_widget_set_app_paintable (GTK_WIDGET (drawing_area), true);

#endif

    gtk_widget_realize (window);
    gtk_widget_realize (GTK_WIDGET (drawing_area));
#if GTK_MAJOR_VERSION < 3
    gdk_window_set_back_pixmap (gtk_widget_get_window (window), NULL, false);
    gdk_window_set_back_pixmap (gtk_widget_get_window (GTK_WIDGET (drawing_area)), NULL, false);
#endif

#ifndef USE_OPENGL
    gtk_widget_hide (get_widget ("shader_parameters_separator"));
    gtk_widget_hide (get_widget ("shader_parameters_item"));
#else
    enable_widget ("shader_parameters_item", false);
#endif

#if GTK_MAJOR_VERSION >= 3
    g_signal_connect_data (drawing_area,
                           "draw",
                           G_CALLBACK (event_drawingarea_draw),
                           this,
                           NULL,
                           (GConnectFlags) 0);
#else
    g_signal_connect_data (drawing_area,
                           "expose-event",
                           G_CALLBACK (event_drawingarea_expose),
                           this,
                           NULL,
                           (GConnectFlags) 0);
#endif
    signal_connect (callbacks);

    if (config->window_width < 100 || config->window_height < 100)
    {
        config->window_width = 256;
        config->window_height = 224;
    }

    default_cursor = gdk_cursor_new_for_display (gdk_display_get_default (),GDK_LEFT_PTR);
    gdk_window_set_cursor (gtk_widget_get_window (window), default_cursor);

    resize (config->window_width, config->window_height);
}

extern int gtk_splash_smtpe_size;
extern unsigned char gtk_splash_smtpe[];
extern int gtk_splash_combo_size;
extern unsigned char gtk_splash_combo[];
extern int gtk_splash_pattern_size;
extern unsigned char gtk_splash_pattern[];

void Snes9xWindow::setup_splash()
{
    uint16 *screen_ptr = GFX.Screen;

    /* Load splash image (RGB24) into Snes9x buffer (RGB15) */
    last_width = 256;
    last_height = 224;

    if (config->splash_image == SPLASH_IMAGE_PATTERN ||
        config->splash_image == SPLASH_IMAGE_SMTPE   ||
        config->splash_image == SPLASH_IMAGE_COMBO) {
        unsigned char *pattern = NULL;
        int pattern_size = 0;

        if (config->splash_image == SPLASH_IMAGE_PATTERN) {
            pattern = gtk_splash_pattern;
            pattern_size = gtk_splash_pattern_size;
        } else if (config->splash_image == SPLASH_IMAGE_SMTPE) {
            pattern = gtk_splash_smtpe;
            pattern_size = gtk_splash_smtpe_size;
        } else {
            pattern = gtk_splash_combo;
            pattern_size = gtk_splash_combo_size;
        }

        auto pixbuf_loader = gdk_pixbuf_loader_new_with_type("png", NULL);
        gdk_pixbuf_loader_write(pixbuf_loader, pattern, pattern_size, NULL);
        gdk_pixbuf_loader_close(pixbuf_loader, NULL);
        auto pixbuf = gdk_pixbuf_loader_get_pixbuf(pixbuf_loader);
        const unsigned char *splash_ptr = gdk_pixbuf_get_pixels(pixbuf);
        const int channels = gdk_pixbuf_get_n_channels(pixbuf);

        for (int y = 0; y < 224; y++, screen_ptr += (GFX.Pitch / 2)) {
            for (int x = 0; x < 256; x++) {
                unsigned int red = splash_ptr[0];
                unsigned int green = splash_ptr[1];
                unsigned int blue = splash_ptr[2];

                screen_ptr[x] = ((red & 0xF8) << 8) +
                                ((green & 0xF8) << 3) +
                                ((green & 0x80) >> 2) +
                                ((blue & 0xF8) >> 3);

                splash_ptr += channels;
            }
        }

        g_object_unref(pixbuf_loader);

        return;
    }

    if (config->splash_image == SPLASH_IMAGE_BLUE) {
        for (int y = 0; y < 224; y++, screen_ptr += (GFX.Pitch / 2)) {
            uint16 colora = (uint16)y / 7;
            uint16 colorb = ((uint16)y - 3) / 7;
            if (colorb > 32)
                colorb = 0;

            for (int x = 0; x < 256; x++) {
                screen_ptr[x] = ((x ^ y) & 1) ? colorb : colora;
            }
        }

        return;
    }

    for (int y = 0; y < 224; y++, screen_ptr += (GFX.Pitch / 2)) {
        memset(screen_ptr, 0, 256 * sizeof(uint16));
    }
}

void
Snes9xWindow::expose ()
{
    if (!(config->fullscreen) && !(maximized_state))
    {
        config->window_width = get_width();
        config->window_height = get_height();
    }

    if (last_width < 0)
    {
        setup_splash();
    }

    S9xDisplayRefresh (last_width, last_height);

    if (!(config->fullscreen))
    {
        config->window_width = get_width ();
        config->window_height = get_height ();
    }

    if (is_paused () || NetPlay.Paused)
    {
        S9xDeinitUpdate (last_width, last_height);
    }
}

void
Snes9xWindow::focus_notify (bool state)
{
    focused = state;

    if (!state && config->pause_emulation_on_switch)
    {
        sys_pause++;
        propagate_pause_state ();
        paused_from_focus_loss = true;
    }

    if (state && paused_from_focus_loss)
    {
        unpause_from_focus_change ();
        paused_from_focus_loss = false;
    }
}

void
Snes9xWindow::open_multicart_dialog ()
{
    int result;
    GtkBuilderWindow *dialog = new GtkBuilderWindow ("multicart_dialog");
    GtkFileChooser *slota, *slotb;
    GtkWidget *multicart_dialog = GTK_WIDGET (dialog->get_window ());

    gtk_window_set_transient_for (dialog->get_window (), get_window ());

    pause_from_focus_change ();

    slota = GTK_FILE_CHOOSER (dialog->get_widget ("multicart_slota"));
    slotb = GTK_FILE_CHOOSER (dialog->get_widget ("multicart_slotb"));

    gtk_file_chooser_set_current_folder (slota, config->last_directory.c_str ());
    gtk_file_chooser_set_current_folder (slotb, config->last_directory.c_str ());

    result = gtk_dialog_run (GTK_DIALOG (multicart_dialog));

    gtk_widget_hide (multicart_dialog);

    if (result == GTK_RESPONSE_OK)
    {
        const gchar *filename;

        filename = gtk_file_chooser_get_filename (slota);
        if (filename)
            strncpy (Settings.CartAName, filename, PATH_MAX);
        else
            Settings.CartAName[0] = '\0';

        filename = gtk_file_chooser_get_filename (slotb);
        if (filename)
            strncpy (Settings.CartBName, filename, PATH_MAX);
        else
            Settings.CartBName[0] = '\0';

        Settings.Multi = true;

        if (S9xOpenROM (NULL))
        {
            GtkWidget *msg;

            msg = gtk_message_dialog_new (GTK_WINDOW (this->window),
                                          GTK_DIALOG_DESTROY_WITH_PARENT,
                                          GTK_MESSAGE_ERROR,
                                          GTK_BUTTONS_CLOSE,
                                          _("Couldn't load files."));
            gtk_window_set_title (GTK_WINDOW (msg), _("Error"));
            gtk_dialog_run (GTK_DIALOG (msg));
            gtk_widget_destroy (msg);
        }
    }

    delete dialog;

    unpause_from_focus_change ();
}

const char *
Snes9xWindow::open_movie_dialog (bool readonly)
{
    GtkWidget     *dialog;
    GtkFileFilter *filter;
    char          *filename;
    gint          result;
    const char    *extensions[] =
    {
            "*.smv", "*.SMV",
            NULL
    };

    this->pause_from_focus_change ();

    if (readonly)
    {
        dialog = gtk_file_chooser_dialog_new (_("Open SNES Movie"),
                                              GTK_WINDOW (this->window),
                                              GTK_FILE_CHOOSER_ACTION_OPEN,
                                              "gtk-cancel", GTK_RESPONSE_CANCEL,
                                              "gtk-open", GTK_RESPONSE_ACCEPT,
                                              NULL);
    }
    else
    {
        char def[PATH_MAX];
        char default_name[PATH_MAX];
        char drive[_MAX_DRIVE];
        char dir[_MAX_DIR];
        char ext[_MAX_EXT];

        _splitpath (Memory.ROMFilename, drive, dir, def, ext);

        snprintf (default_name, PATH_MAX, "%s.smv", def);

        dialog = gtk_file_chooser_dialog_new (_("New SNES Movie"),
                                              GTK_WINDOW (this->window),
                                              GTK_FILE_CHOOSER_ACTION_SAVE,
                                              "gtk-cancel", GTK_RESPONSE_CANCEL,
                                              "gtk-open", GTK_RESPONSE_ACCEPT,
                                              NULL);

        gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog),
                                           default_name);

    }

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("SNES Movies"));
    for (int i = 0; extensions[i]; i++)
    {
        gtk_file_filter_add_pattern (filter, extensions[i]);
    }
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("All Files"));
    gtk_file_filter_add_pattern (filter, "*.*");
    gtk_file_filter_add_pattern (filter, "*");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog),
                                         S9xGetDirectory (SRAM_DIR));

    result = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_hide (dialog);

    if (result == GTK_RESPONSE_ACCEPT)
    {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    }
    else
    {
        filename = strdup ("");
    }

    gtk_widget_destroy (dialog);

    this->unpause_from_focus_change ();

    return filename;
}

void
Snes9xWindow::open_rom_dialog ()
{
    char *filename;

    pause_from_focus_change ();

    filename = S9xOpenROMDialog ();

    if (filename)
    {
        Settings.Multi = false;
        try_open_rom (filename);

        g_free (filename);
    }

    unpause_from_focus_change ();
}

bool Snes9xWindow::try_open_rom(const char *filename)
{
    pause_from_focus_change ();

    Settings.Multi = false;

    if (S9xOpenROM (filename))
    {
        GtkWidget *msg;

        msg = gtk_message_dialog_new (GTK_WINDOW (this->window),
                                      GTK_DIALOG_DESTROY_WITH_PARENT,
                                      GTK_MESSAGE_ERROR,
                                      GTK_BUTTONS_CLOSE,
                                      _("Couldn't load file '%s'"),
                                      filename);
        gtk_window_set_title (GTK_WINDOW (msg), _("Error"));
        gtk_dialog_run (GTK_DIALOG (msg));
        gtk_widget_destroy (msg);

        unpause_from_focus_change ();

        return false;
    }
    else
    {
        const char *groups[] = { "cartridge", NULL };

        GtkRecentData recent_data =
        {
                NULL,
                (gchar *) "SNES ROM",
                (gchar *) "application/x-snes-rom",
                (gchar *) "Snes9x",
                NULL,
                (gchar **) groups,
                false
        };
        gchar *u_filename;

        u_filename = g_filename_to_uri (filename, NULL, NULL);

        recent_data.app_exec = g_strjoin (" ",
                                          g_get_prgname (),
                                          "%f",
                                          NULL);

        gtk_recent_manager_add_full (gtk_recent_manager_get_default (),
                                     u_filename,
                                     &recent_data);

        g_free (recent_data.app_exec);
        g_free (u_filename);

        this->unpause_from_user ();

    }

    this->unpause_from_focus_change ();

    return true;
}

void
Snes9xWindow::load_state_dialog ()
{
    GtkWidget     *dialog;
    GtkFileFilter *filter;
    char          *filename;
    gint          result;

    this->pause_from_focus_change ();

    dialog = gtk_file_chooser_dialog_new (_("Load Saved State"),
                                          GTK_WINDOW (this->window),
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          "gtk-cancel", GTK_RESPONSE_CANCEL,
                                          "gtk-open", GTK_RESPONSE_ACCEPT,
                                          NULL);

    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog),
                                         S9xGetDirectory (SNAPSHOT_DIR));

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("Save States"));
    gtk_file_filter_add_pattern (filter, "*.sst");
    gtk_file_filter_add_pattern (filter, "*.zst");
    gtk_file_filter_add_pattern (filter, "*.ZST");
    gtk_file_filter_add_pattern (filter, "*.000");
    gtk_file_filter_add_pattern (filter, "*.001");
    gtk_file_filter_add_pattern (filter, "*.002");
    gtk_file_filter_add_pattern (filter, "*.003");
    gtk_file_filter_add_pattern (filter, "*.004");
    gtk_file_filter_add_pattern (filter, "*.005");
    gtk_file_filter_add_pattern (filter, "*.006");
    gtk_file_filter_add_pattern (filter, "*.007");
    gtk_file_filter_add_pattern (filter, "*.008");
    gtk_file_filter_add_pattern (filter, "*.009");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("All Files"));
    gtk_file_filter_add_pattern (filter, "*.*");
    gtk_file_filter_add_pattern (filter, "*");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

    result = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_hide (dialog);

    if (result == GTK_RESPONSE_ACCEPT)
    {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

        S9xLoadState (filename);

        g_free (filename);
    }

    else
    {
    }

    gtk_widget_destroy (dialog);

    this->unpause_from_focus_change ();
}

void
Snes9xWindow::movie_seek_dialog ()
{
    char      str[1024];
    gint      result;

    if (!S9xMovieActive ())
        return;

    GtkBuilderWindow *seek_dialog = new GtkBuilderWindow ("frame_advance_dialog");
    GtkWindow *seek_window = seek_dialog->get_window ();

    pause_from_focus_change ();

    snprintf (str, 1024, _("The current frame in the movie is <b>%d</b>."), S9xMovieGetFrameCounter ());
    gtk_label_set_label (GTK_LABEL (seek_dialog->get_widget ("current_frame_label")), str);

    snprintf (str, 1024, "%d", S9xMovieGetFrameCounter ());
    seek_dialog->set_entry_text ("frame_entry", str);

    gtk_window_set_transient_for (seek_window, get_window ());

    result = gtk_dialog_run (GTK_DIALOG (seek_window));

    int entry_value = seek_dialog->get_entry_value ("frame_entry");

    switch (result)
    {
        case GTK_RESPONSE_OK:

            if (entry_value > 0 &&
                entry_value > (int) S9xMovieGetFrameCounter ())
            {
                Settings.HighSpeedSeek =
                    entry_value - S9xMovieGetFrameCounter ();
            }

            break;
    }

    delete seek_dialog;

    unpause_from_focus_change ();
}

void
Snes9xWindow::save_state_dialog ()
{
    GtkWidget     *dialog;
    GtkFileFilter *filter;
    char          *filename;
    gint          result;
    char          def[PATH_MAX];
    char          default_name[PATH_MAX];
    char          drive[_MAX_DRIVE];
    char          dir[_MAX_DIR];
    char          ext[_MAX_EXT];

    this->pause_from_focus_change ();

    _splitpath (Memory.ROMFilename, drive, dir, def, ext);

    snprintf (default_name, PATH_MAX, "%s.sst", def);

    dialog = gtk_file_chooser_dialog_new (_("Save State"),
                                          GTK_WINDOW (this->window),
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          "gtk-cancel", GTK_RESPONSE_CANCEL,
                                          "gtk-save", GTK_RESPONSE_ACCEPT,
                                          NULL);

    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog),
                                         S9xGetDirectory (SNAPSHOT_DIR));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog),
                                       default_name);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("Save States"));
    gtk_file_filter_add_pattern (filter, "*.sst");
    gtk_file_filter_add_pattern (filter, "*.zst");
    gtk_file_filter_add_pattern (filter, "*.ZST");
    gtk_file_filter_add_pattern (filter, "*.000");
    gtk_file_filter_add_pattern (filter, "*.001");
    gtk_file_filter_add_pattern (filter, "*.002");
    gtk_file_filter_add_pattern (filter, "*.003");
    gtk_file_filter_add_pattern (filter, "*.004");
    gtk_file_filter_add_pattern (filter, "*.005");
    gtk_file_filter_add_pattern (filter, "*.006");
    gtk_file_filter_add_pattern (filter, "*.007");
    gtk_file_filter_add_pattern (filter, "*.008");
    gtk_file_filter_add_pattern (filter, "*.009");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("All Files"));
    gtk_file_filter_add_pattern (filter, "*.*");
    gtk_file_filter_add_pattern (filter, "*");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

    result = gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_widget_hide (dialog);

    if (result == GTK_RESPONSE_ACCEPT)
    {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

        S9xSaveState (filename);

        g_free (filename);
    }

    else
    {
    }

    gtk_widget_destroy (dialog);

    this->unpause_from_focus_change ();
}

void
Snes9xWindow::save_spc_dialog ()
{
    GtkWidget     *dialog;
    GtkFileFilter *filter;
    char          *filename;
    gint          result;
    char          def[PATH_MAX];
    char          default_name[PATH_MAX];
    char          drive[_MAX_DRIVE];
    char          dir[_MAX_DIR];
    char          ext[_MAX_EXT];

    this->pause_from_focus_change ();

    _splitpath (Memory.ROMFilename, drive, dir, def, ext);

    snprintf (default_name, PATH_MAX, "%s.spc", def);

    dialog = gtk_file_chooser_dialog_new (_("Save SPC file..."),
                                          GTK_WINDOW (this->window),
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          "gtk-cancel", GTK_RESPONSE_CANCEL,
                                          "gtk-save", GTK_RESPONSE_ACCEPT,
                                          NULL);

    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog),
                                         S9xGetDirectory (SNAPSHOT_DIR));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog),
                                       default_name);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("SPC Files"));
    gtk_file_filter_add_pattern (filter, "*.spc");
    gtk_file_filter_add_pattern (filter, "*.SPC");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("All Files"));
    gtk_file_filter_add_pattern (filter, "*.*");
    gtk_file_filter_add_pattern (filter, "*");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

    result = gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_widget_hide (dialog);

    if (result == GTK_RESPONSE_ACCEPT)
    {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

        if (S9xSPCDump (filename))
        {
            /* Success ? */
        }
        else
        {
            GtkWidget *msg;

            msg = gtk_message_dialog_new (GTK_WINDOW (this->window),
                                          GTK_DIALOG_DESTROY_WITH_PARENT,
                                          GTK_MESSAGE_ERROR,
                                          GTK_BUTTONS_CLOSE,
                                          _("Couldn't save SPC file '%s'"),
                                          filename);
            gtk_window_set_title (GTK_WINDOW (msg), _("Error"));
            gtk_dialog_run (GTK_DIALOG (msg));
            gtk_widget_destroy (msg);
        }

        g_free (filename);
    }

    else
    {
    }

    gtk_widget_destroy (dialog);

    this->unpause_from_focus_change ();
}

void
Snes9xWindow::set_menu_item_selected (const char *name)
{
    GtkCheckMenuItem *item;

    item = GTK_CHECK_MENU_ITEM (get_widget (name));

    gtk_check_menu_item_set_active (item, 1);
}

void
Snes9xWindow::show_rom_info ()
{
    GtkWidget *msg;

    pause_from_focus_change ();

    msg = gtk_message_dialog_new_with_markup (GTK_WINDOW (window),
                                              GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_MESSAGE_OTHER,
                                              GTK_BUTTONS_CLOSE,
                                              _("<b>Information for %s</b>\n\n"
                                              "<i>Name:</i> %s\n"
                                              "<i>Speed:</i> %02X/%s\n"
                                              "<i>Map:</i> %s\n"
                                              "<i>Type:</i> %02x\n"
                                              "<i>Contents:</i> %s\n"
                                              "<i>ROM Size:</i> %s\n"
                                              "<i>Calculated Size:</i> %d\n"
                                              "<i>SRAM Size:</i> %s\n"
                                              "<i>Header Checksum:</i> %04X\n"
                                              "<i>Checksum Compliment:</i> %04X\n"
                                              "<i>Actual Checksum:</i> %04X\n"
                                              "<i>Video:</i> %s\n"
                                              "<i>CRC32:</i> %08X\n"
                                              "<i>Revision:</i> %s"
                                              "<b><i>%s</i></b>"),
                                              Memory.ROMFilename,
                                              Memory.ROMName,
                                              Memory.ROMSpeed,
                                              ((Memory.ROMSpeed & 0x10) != 0) ?
                                                      "FastROM" : "SlowROM",
                                              (Memory.HiROM) ?
                                                      "HiROM" : "LoROM",
                                              Memory.ROMType,
                                              Memory.KartContents (),
                                              Memory.Size (),
                                              Memory.CalculatedSize / 0x20000,
                                              Memory.StaticRAMSize (),
                                              Memory.ROMChecksum,
                                              Memory.ROMComplementChecksum,
                                              Memory.CalculatedChecksum,
                                              (Memory.ROMRegion > 12 ||
                                               Memory.ROMRegion < 2) ?
                                                  "NTSC 60Hz" : "PAL 50Hz",
                                              Memory.ROMCRC32,
                                              Memory.Revision (),
                                              (Memory.ROMChecksum !=
                                                  Memory.CalculatedChecksum) ?
                                               _("\n\nThis ROM has been modified or damaged")
                                               : "");
    gtk_window_set_title (GTK_WINDOW (msg), _("File Information"));

    gtk_dialog_run (GTK_DIALOG (msg));

    unpause_from_focus_change ();

    gtk_widget_destroy (msg);
}

void
Snes9xWindow::configure_widgets ()
{
    enable_widget ("continue_item", config->rom_loaded);
    enable_widget ("pause_item", config->rom_loaded);
    enable_widget ("reset_item", config->rom_loaded);
    enable_widget ("load_state_item", config->rom_loaded);
    enable_widget ("save_state_item", config->rom_loaded);
    enable_widget ("save_spc_item", config->rom_loaded);
    enable_widget ("hard_reset_item", config->rom_loaded);
    enable_widget ("record_movie_item", config->rom_loaded);
    enable_widget ("stop_recording_item", config->rom_loaded);
    enable_widget ("open_movie_item", config->rom_loaded);
    enable_widget ("jump_to_frame_item", config->rom_loaded);
    enable_widget ("cheats_item", config->rom_loaded);
    enable_widget ("rom_info_item", config->rom_loaded);

    enable_widget ("sync_clients_item",
                   config->rom_loaded &&
                   Settings.NetPlay   &&
                   Settings.NetPlayServer);

    if (config->default_esc_behavior != ESC_TOGGLE_MENUBAR)
    {
        enable_widget ("fullscreen_item", config->rom_loaded);

        config->ui_visible = true;

        if (!config->fullscreen)
        {
            gtk_widget_show (get_widget ("menubar"));
        }
        else
        {
            gtk_widget_hide (get_widget ("menubar"));
        }

        gtk_widget_hide (get_widget ("hide_ui"));
        gtk_widget_hide (get_widget ("hide_ui_separator"));
    }
    else
    {
        enable_widget ("fullscreen_item", true);

        gtk_widget_show (get_widget ("hide_ui"));
        gtk_widget_show (get_widget ("hide_ui_separator"));

        if (config->ui_visible)
        {
            gtk_widget_show (get_widget ("menubar"));
        }
        else
        {
            gtk_widget_hide (get_widget ("menubar"));
        }
    }

    propagate_pause_state ();

    if (config->rom_loaded && !Settings.Paused)
        hide_mouse_cursor ();
    else
        show_mouse_cursor ();
}

void
Snes9xWindow::set_mouseable_area (int x, int y, int width, int height)
{
    mouse_region_x      = x;
    mouse_region_y      = y;
    mouse_region_width  = width;
    mouse_region_height = height;
}

void
Snes9xWindow::reset_screensaver ()
{
    if (!focused)
        return;

#ifdef GDK_WINDOWING_X11
    if (GDK_IS_X11_WINDOW (gtk_widget_get_window (GTK_WIDGET (window))))
    {
        XResetScreenSaver (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()));
    }
#endif
#ifdef GDK_WINDOWING_WAYLAND
    if (GDK_IS_WAYLAND_WINDOW (gtk_widget_get_window (GTK_WIDGET (window))))
    {
    // TODO screensaver for wayland
    }
#endif

    config->screensaver_needs_reset = false;
}

void
Snes9xWindow::toggle_fullscreen_mode ()
{
    if (config->fullscreen)
        leave_fullscreen_mode ();
    else
        enter_fullscreen_mode ();
}

static double XRRGetExactRefreshRate (Display *dpy, Window window)
{
    XRRScreenResources *resources = NULL;
    XRRCrtcInfo        *crtc_info = NULL;
    int event_base;
    int error_base;
    int version_major;
    int version_minor;
    double refresh_rate = 0.0;
    int i;

    if (!XRRQueryExtension (dpy, &event_base, &error_base) ||
        !XRRQueryVersion (dpy, &version_major, &version_minor))
    {
        return refresh_rate;
    }

    if (version_minor < 3)
        return refresh_rate;

    resources   = XRRGetScreenResourcesCurrent (dpy, window);
    crtc_info   = XRRGetCrtcInfo (dpy, resources, resources->crtcs[0]);

    for (i = 0; i < resources->nmode; i++)
    {
        if (resources->modes[i].id == crtc_info->mode)
        {
            XRRModeInfo *m = &resources->modes[i];

            refresh_rate = (double) m->dotClock / m->hTotal / m->vTotal;
            refresh_rate /= m->modeFlags & RR_DoubleScan     ? 2 : 1;
            refresh_rate /= m->modeFlags & RR_ClockDivideBy2 ? 2 : 1;
            refresh_rate *= m->modeFlags & RR_DoubleClock    ? 2 : 1;

            break;
        }
    }

    XRRFreeCrtcInfo (crtc_info);
    XRRFreeScreenResources (resources);

    return refresh_rate;
}

double
Snes9xWindow::get_refresh_rate ()
{
    double refresh_rate = 0.0;
    GdkDisplay *display = gtk_widget_get_display (window);
    GdkWindow *gdk_window =  gtk_widget_get_window (window);

#ifdef GDK_WINDOWING_X11
    if (GDK_IS_X11_DISPLAY (display))
    {
        Window xid = gdk_x11_window_get_xid (gtk_widget_get_window (window));
        Display *dpy = gdk_x11_display_get_xdisplay (gtk_widget_get_display (window));
        refresh_rate = XRRGetExactRefreshRate (dpy, xid);
    }
#endif

#ifdef GDK_WINDOWING_WAYLAND
    if (GDK_IS_WAYLAND_DISPLAY (display))
    {
        GdkMonitor *monitor = gdk_display_get_monitor_at_window(display, gdk_window);
        refresh_rate = (double) gdk_monitor_get_refresh_rate(monitor) / 1000.0;
    }
#endif

    if (refresh_rate < 10.0)
    {
        printf ("Warning: Couldn't read refresh rate.\n");
        refresh_rate = 60.0;
    }

    return refresh_rate;
}

int
Snes9xWindow::get_auto_input_rate ()
{
    double refresh_rate = get_refresh_rate ();

    if (refresh_rate == 0.0)
        return 0;

    // Try for a close multiple of 60hz
    if (refresh_rate > 119.0 && refresh_rate < 121.0)
        refresh_rate /= 2.0;
    if (refresh_rate > 179.0 && refresh_rate < 181.0)
        refresh_rate /= 3.0;
    if (refresh_rate > 239.0 && refresh_rate < 241.0)
        refresh_rate /= 4.0;

    double new_input_rate = refresh_rate * 32000.0 / 60.09881389744051 + 0.5;

    if (new_input_rate > 32000.0 * 1.05 || new_input_rate < 32000.0 * 0.95)
        new_input_rate = 0.0;

    return new_input_rate;
}

#ifdef GDK_WINDOWING_X11
static void set_bypass_compositor (Display *dpy, Window window, unsigned char bypass)
{
    uint32 value = bypass;
    Atom net_wm_bypass_compositor = XInternAtom (dpy, "_NET_WM_BYPASS_COMPOSITOR", False);
    XChangeProperty (dpy, window, net_wm_bypass_compositor, XA_CARDINAL, 32, PropModeReplace, (const unsigned char *) &value, 1);
}
#endif

void
Snes9xWindow::enter_fullscreen_mode ()
{
    int rom_loaded = config->rom_loaded;

    if (config->fullscreen)
        return;

    config->rom_loaded = 0;

    nfs_width = config->window_width;
    nfs_height = config->window_height;

    gtk_window_get_position (GTK_WINDOW (window), &nfs_x, &nfs_y);

    if (config->change_display_resolution)
    {
        GdkDisplay *gdk_display = gtk_widget_get_display (window);
        Display *dpy = gdk_x11_display_get_xdisplay (gdk_display);

        gdk_display_sync (gdk_display);
        if (XRRSetCrtcConfig (dpy,
                              config->xrr_screen_resources,
                              config->xrr_screen_resources->crtcs[0],
                              CurrentTime,
                              config->xrr_crtc_info->x,
                              config->xrr_crtc_info->y,
                              config->xrr_screen_resources->modes[config->xrr_index].id,
                              config->xrr_crtc_info->rotation,
                              &config->xrr_crtc_info->outputs[0],
                              1) != 0)
        {
            config->change_display_resolution = 0;
        }

        if (gui_config->auto_input_rate)
        {
            Settings.SoundInputRate = top_level->get_auto_input_rate ();
            S9xUpdateDynamicRate (1, 2);
        }
    }

    /* Make sure everything is done synchronously */
    gdk_display_sync (gdk_display_get_default ());
    gtk_window_fullscreen (GTK_WINDOW (window));

    gdk_display_sync (gdk_display_get_default ());
    gtk_window_present (GTK_WINDOW (window));
#ifdef GDK_WINDOWING_X11
    if (GDK_IS_X11_WINDOW (gtk_widget_get_window (GTK_WIDGET (window))) &&
        config->default_esc_behavior != ESC_TOGGLE_MENUBAR)
    {
        set_bypass_compositor (gdk_x11_display_get_xdisplay (gtk_widget_get_display (GTK_WIDGET (window))),
                               gdk_x11_window_get_xid (gtk_widget_get_window (GTK_WIDGET (window))),
                               1);
    }
#endif
    config->fullscreen = 1;
    config->rom_loaded = rom_loaded;

    /* If we're running a game, disable ui when entering fullscreen */
    if (!Settings.Paused && config->rom_loaded)
        config->ui_visible = false;

    configure_widgets ();
}

void
Snes9xWindow::leave_fullscreen_mode ()
{
    int rom_loaded = config->rom_loaded;

    if (!config->fullscreen)
        return;

    config->rom_loaded = 0;

    if (config->change_display_resolution)
    {
        GdkDisplay *gdk_display = gtk_widget_get_display (window);
        Display *dpy = gdk_x11_display_get_xdisplay (gdk_display);

        if (config->xrr_index > config->xrr_screen_resources->nmode)
            config->xrr_index = 0;

        gdk_display_sync (gdk_display);
        XRRSetCrtcConfig (dpy,
                          config->xrr_screen_resources,
                          config->xrr_screen_resources->crtcs[0],
                          CurrentTime,
                          config->xrr_crtc_info->x,
                          config->xrr_crtc_info->y,
                          config->xrr_crtc_info->mode,
                          config->xrr_crtc_info->rotation,
                          &config->xrr_crtc_info->outputs[0],
                          1);

        if (gui_config->auto_input_rate)
        {
            Settings.SoundInputRate = top_level->get_auto_input_rate ();
            S9xUpdateDynamicRate (1, 2);
        }
    }

    gtk_window_unfullscreen (GTK_WINDOW (window));

#ifdef GDK_WINDOWING_X11
    if (GDK_IS_X11_WINDOW (gtk_widget_get_window (GTK_WIDGET (window))))
    {
        set_bypass_compositor (gdk_x11_display_get_xdisplay (gtk_widget_get_display (GTK_WIDGET (window))),
                               gdk_x11_window_get_xid (gtk_widget_get_window (GTK_WIDGET (window))),
                               0);
    }
#endif

    resize (nfs_width, nfs_height);
    gtk_window_move (GTK_WINDOW (window), nfs_x, nfs_y);

    config->rom_loaded = rom_loaded;

    config->fullscreen = 0;

    configure_widgets ();
}

void
Snes9xWindow::resize_viewport (int width, int height)
{
    GtkWidget     *item;
    GtkAllocation allocation;
    int           y_padding = 0;

    item = get_widget ("menubar");
    gtk_widget_get_allocation (item, &allocation);
    y_padding += gtk_widget_get_visible (item) ? allocation.height : 0;

    resize (width, height + y_padding);
}

void
Snes9xWindow::hide_mouse_cursor ()
{
    if (!empty_cursor)
    {
        empty_cursor = gdk_cursor_new_for_display (gdk_display_get_default (), GDK_BLANK_CURSOR);
    }

    gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (drawing_area)),
                           empty_cursor);
    config->pointer_is_visible = false;
}

void
Snes9xWindow::show_mouse_cursor ()
{
    gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (drawing_area)),
                           NULL);
    config->pointer_is_visible = true;
}

void
Snes9xWindow::center_mouse ()
{
    GdkWindow *gdk_window = gtk_widget_get_window (window);
    GdkDisplay *gdk_display = gdk_window_get_display (gdk_window);
    GdkScreen *gdk_screen = gdk_window_get_screen (gdk_window);
    int x, y, w, h;

    gdk_window_get_origin (gdk_window, &x, &y);
    w = gdk_window_get_width (gdk_window);
    h = gdk_window_get_height (gdk_window);

    gdk_mouse_x = x + w / 2;
    gdk_mouse_y = y + h / 2;

#if GTK_MAJOR_VERSION < 3
    gdk_display_warp_pointer (gdk_display, gdk_screen, gdk_mouse_x,
                              gdk_mouse_y);
#else
    GdkSeat *seat = gdk_display_get_default_seat (gdk_display);
    GdkDevice *pointer = gdk_seat_get_pointer (seat);

    gdk_device_warp (pointer, gdk_screen, gdk_mouse_x, gdk_mouse_y);
#endif
}

void
Snes9xWindow::toggle_grab_mouse ()
{
    GdkWindow *gdk_window = gtk_widget_get_window (window);
    GdkDisplay *gdk_display = gdk_window_get_display (gdk_window);

    if ((!mouse_grabbed && !S9xIsMousePluggedIn ()) || !config->rom_loaded)
        return;

#if GTK_MAJOR_VERSION < 3
    if (!mouse_grabbed)
    {
        gdk_pointer_grab (gdk_window, true, (GdkEventMask) 1020, gdk_window, empty_cursor, GDK_CURRENT_TIME);
        center_mouse ();
    }
    else
    {
        gdk_pointer_ungrab (GDK_CURRENT_TIME);
        if (config->pointer_is_visible)
            show_mouse_cursor ();
    }
#else
    GdkSeat *seat = gdk_display_get_default_seat (gdk_display);

    if (!mouse_grabbed)
        gdk_seat_grab (seat, gdk_window, GDK_SEAT_CAPABILITY_ALL_POINTING, true,
                       empty_cursor, NULL, NULL, NULL);
    else
        gdk_seat_ungrab (seat);
#endif

    S9xReportPointer (BINDING_MOUSE_POINTER, 0, 0);
    snes_mouse_x = 0.0; snes_mouse_y = 0.0;
    mouse_grabbed = !mouse_grabbed;
    if (mouse_grabbed)
        center_mouse ();
}

void
Snes9xWindow::show ()
{
    gtk_widget_show (window);

    if (!recent_menu)
    {
        /* Add recent menu after showing window to avoid "No items" bug */
        recent_menu = gtk_recent_chooser_menu_new_for_manager (
                          gtk_recent_manager_get_default ());

        GtkRecentFilter *filter = gtk_recent_filter_new ();
        GtkRecentChooser *chooser = GTK_RECENT_CHOOSER (recent_menu);

        gtk_recent_filter_add_group (filter, "cartridge");
        gtk_recent_chooser_set_local_only (chooser, true);
        gtk_recent_chooser_set_show_icons (chooser, false);
        gtk_recent_chooser_set_sort_type (chooser, GTK_RECENT_SORT_MRU);
        gtk_recent_chooser_add_filter (chooser, filter);

        gtk_menu_item_set_submenu (
            GTK_MENU_ITEM (get_widget ("open_recent_item")),
            recent_menu);

        g_signal_connect (G_OBJECT (recent_menu),
                          "item-activated",
                          G_CALLBACK (event_recent_open),
                          (gpointer) this);

        gtk_widget_show (recent_menu);
    }
}

void
Snes9xWindow::propagate_pause_state ()
{
    int oldpause = Settings.Paused;

    Settings.Paused = (sys_pause || user_pause || !(config->rom_loaded));

    if (Settings.Paused != oldpause)
    {
        if (!is_paused ())
        {
            S9xSoundStart ();
            if (config->rom_loaded)
                enable_widget ("pause_item", true);

            S9xDisplayClearBuffers ();
        }
        else
        {
            S9xSoundStop ();
            enable_widget ("pause_item", false);

        }

        configure_widgets ();
    }
}

void
Snes9xWindow::toggle_ui ()
{
    config->ui_visible = !config->ui_visible;

    configure_widgets ();
}

/* gui_[un]pause Handles when system needs to pause the emulator */
void
Snes9xWindow::pause_from_focus_change ()
{
    sys_pause += config->modal_dialogs;

    propagate_pause_state ();
}

void
Snes9xWindow::unpause_from_focus_change ()
{
    if (--sys_pause < 0)
        sys_pause = 0;
    propagate_pause_state ();
}

/* client_[un]pause Handles when user manually chooses to pause */
void
Snes9xWindow::pause_from_user ()
{
    user_pause = true;
    propagate_pause_state ();
}

void
Snes9xWindow::unpause_from_user ()
{
    user_pause = false;
    propagate_pause_state ();
}

bool Snes9xWindow::is_paused()
{
    if (user_pause || sys_pause || Settings.Paused || !(config->rom_loaded))
        return true;

    return false;
}

void
Snes9xWindow::set_menu_item_accel_to_binding (const char *name,
                                              const char *binding)
{
    Binding bin;
    char str[255];
    GtkAccelGroup *accel_group = NULL;

    if (!strcmp (binding, "Escape Key"))
    {
        bin = Binding (GDK_Escape, false, false, false);
    }
    else
    {
        bin = S9xGetBindingByName (binding);
    }

    snprintf (str, 255, "<Snes9x>/%s", name);
    if (!(bin.is_key ()))
    {
        gtk_accel_map_change_entry (str,
                                    0,
                                    (GdkModifierType) 0,
                                    true);
        return;
    }

    GSList *accel_group_list = gtk_accel_groups_from_object (G_OBJECT (window));

    if (accel_group_list)
    {
        accel_group = GTK_ACCEL_GROUP (accel_group_list->data);
    }
    else
    {
        accel_group = gtk_accel_group_new ();
        gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);
    }

    gtk_widget_set_accel_path (get_widget (name), str, accel_group);

    if (!gtk_accel_map_lookup_entry (str, NULL))
    {
        gtk_accel_map_add_entry (str,
                                 bin.get_key (),
                                 bin.get_gdk_modifiers ());

    }
    else
    {
        gtk_accel_map_change_entry (str,
                                    bin.get_key (),
                                    bin.get_gdk_modifiers (),
                                    true);
    }
}

void
Snes9xWindow::update_accels ()
{
    set_menu_item_accel_to_binding ("fullscreen_item", "GTK_fullscreen");
    set_menu_item_accel_to_binding ("reset_item", "SoftReset");
    set_menu_item_accel_to_binding ("save_state_0", "QuickSave000");
    set_menu_item_accel_to_binding ("save_state_1", "QuickSave001");
    set_menu_item_accel_to_binding ("save_state_2", "QuickSave002");
    set_menu_item_accel_to_binding ("save_state_3", "QuickSave003");
    set_menu_item_accel_to_binding ("save_state_4", "QuickSave004");
    set_menu_item_accel_to_binding ("save_state_5", "QuickSave005");
    set_menu_item_accel_to_binding ("save_state_6", "QuickSave006");
    set_menu_item_accel_to_binding ("save_state_7", "QuickSave007");
    set_menu_item_accel_to_binding ("save_state_8", "QuickSave008");
    set_menu_item_accel_to_binding ("save_state_9", "QuickSave009");
    set_menu_item_accel_to_binding ("load_state_0", "QuickLoad000");
    set_menu_item_accel_to_binding ("load_state_1", "QuickLoad001");
    set_menu_item_accel_to_binding ("load_state_2", "QuickLoad002");
    set_menu_item_accel_to_binding ("load_state_3", "QuickLoad003");
    set_menu_item_accel_to_binding ("load_state_4", "QuickLoad004");
    set_menu_item_accel_to_binding ("load_state_5", "QuickLoad005");
    set_menu_item_accel_to_binding ("load_state_6", "QuickLoad006");
    set_menu_item_accel_to_binding ("load_state_7", "QuickLoad007");
    set_menu_item_accel_to_binding ("load_state_8", "QuickLoad008");
    set_menu_item_accel_to_binding ("load_state_9", "QuickLoad009");
    set_menu_item_accel_to_binding ("pause_item", "GTK_pause");
    set_menu_item_accel_to_binding ("save_spc_item", "GTK_save_spc");
    set_menu_item_accel_to_binding ("open_rom_item", "GTK_open_rom");
    set_menu_item_accel_to_binding ("record_movie_item", "BeginRecordingMovie");
    set_menu_item_accel_to_binding ("open_movie_item", "LoadMovie");
    set_menu_item_accel_to_binding ("stop_recording_item", "EndRecordingMovie");
    set_menu_item_accel_to_binding ("jump_to_frame_item", "GTK_seek_to_frame");
    set_menu_item_accel_to_binding ("reset_item", "SoftReset");
    set_menu_item_accel_to_binding ("hard_reset_item", "Reset");
    set_menu_item_accel_to_binding ("exit_item", "GTK_quit");

    /* Special UI assignment */
    set_menu_item_accel_to_binding ("hide_ui", "Escape Key");
}

void
Snes9xWindow::resize_to_multiple (int factor)
{
    int h = (config->overscan ? 239 : 224) * factor;
    int w = h * S9xGetAspect () + 0.5;

    resize_viewport (w, h);
}

cairo_t *
Snes9xWindow::get_cairo ()
{
    if (cr)
        return cr;

    GtkWidget *drawing_area = GTK_WIDGET (this->drawing_area);

#if GTK_MAJOR_VERSION < 3
    cr = gdk_cairo_create (gtk_widget_get_window (drawing_area));
#else
    GtkAllocation allocation;
    gtk_widget_get_allocation (drawing_area, &allocation);

    cairo_rectangle_int_t rect = { 0, 0, allocation.width, allocation.height };
    cairo_region = cairo_region_create_rectangle (&rect);
    gdk_drawing_context = gdk_window_begin_draw_frame (gtk_widget_get_window (drawing_area),
                                                       cairo_region);
    cr = gdk_drawing_context_get_cairo_context (gdk_drawing_context);
#endif

    cairo_owned = true;
    return cr;
}

void
Snes9xWindow::release_cairo ()
{
    if (cairo_owned)
    {
#if GTK_MAJOR_VERSION < 3
        cairo_destroy (cr);
#else
        gdk_window_end_draw_frame (gtk_widget_get_window (GTK_WIDGET (drawing_area)), gdk_drawing_context);
        cairo_region_destroy (cairo_region);
#endif
        cairo_owned = false;
        cr = NULL;
    }
}
