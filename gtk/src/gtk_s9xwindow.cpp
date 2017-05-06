#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#ifdef USE_GTK3
#include <gdk/gdkkeysyms-compat.h>
#endif
#include <cairo.h>

#ifdef USE_XV
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>
#endif

#include "gtk_s9x.h"
#include "gtk_preferences.h"
#include "gtk_icon.h"
#include "gtk_display.h"
#include "gtk_file.h"
#include "gtk_sound.h"
#include "gtk_control.h"
#include "gtk_cheat.h"
#ifdef NETPLAY_SUPPORT
#include "gtk_netplay.h"
#endif

static gboolean
event_main_window_delete (GtkWidget *widget,
                          GdkEvent  *event,
                          gpointer  data)
{
    S9xExit ();

    return TRUE;
}

static gboolean
event_main_window_state_event (GtkWidget           *widget,
                               GdkEventWindowState *event,
                               gpointer            data)
{
    Snes9xWindow *window = (Snes9xWindow *) data;
    window->fullscreen_state = event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN;
    window->maximized_state  = event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED;

    return FALSE;
}

static gboolean
event_continue_item_activate (GtkWidget *widget, gpointer data)
{
    Snes9xWindow *window = (Snes9xWindow *) data;

    window->unpause_from_user ();

    return TRUE;
}

static gboolean
event_open_cheats (GtkWidget *widget, gpointer data)
{
    Snes9xCheats *cheats;
    cheats = new Snes9xCheats ();

    cheats->show ();
    delete cheats;

    return TRUE;
}

static gboolean
event_open_multicart (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->open_multicart_dialog ();

    return TRUE;
}

static gboolean
event_rom_info (GtkWidget *widget, gpointer data)
{
    Snes9xWindow *window = (Snes9xWindow *) data;

    window->show_rom_info ();

    return TRUE;
}

static gboolean
event_toggle_interface (GtkWidget *widget, gpointer data)
{
    Snes9xWindow *window = (Snes9xWindow *) data;

    window->toggle_ui ();

    return TRUE;
}

static gboolean
event_show_statusbar (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->toggle_statusbar ();

    return TRUE;
}

static gboolean
event_sync_clients (GtkWidget *widget, gpointer data)
{
#ifdef NETPLAY_SUPPORT
    S9xNetplaySyncClients ();
#endif

    return TRUE;
}

static gboolean
event_pause_item_activate (GtkWidget *widget, gpointer data)
{
    Snes9xWindow *window = (Snes9xWindow *) data;

    window->pause_from_user ();

    return TRUE;
}

static gboolean
event_open_netplay (GtkWidget *widget, gpointer data)
{
#ifdef NETPLAY_SUPPORT
    S9xNetplayDialogOpen ();
#endif
    return TRUE;
}

#ifdef USE_GTK3
static gboolean
event_drawingarea_draw (GtkWidget *widget,
                        cairo_t   *cr,
                        gpointer  data)
{
    ((Snes9xWindow *) data)->expose ();

    return FALSE;
}

#endif

#ifndef USE_GTK3
static gboolean
event_drawingarea_expose (GtkWidget      *widget,
                          GdkEventExpose *event,
                          gpointer       data)
{
    ((Snes9xWindow *) data)->expose ();

    return FALSE;
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
        return TRUE;
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

        return TRUE;
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
        return TRUE;
    }

    return FALSE; /* Pass the key to GTK */
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
        return FALSE;
    }

    window->mouse_loc_x = (uint16)
        ((int) (event->x) - window->mouse_region_x) * 256 /
        (window->mouse_region_width <= 0 ? 1 : window->mouse_region_width);

    window->mouse_loc_y = (uint16)
        ((int) (event->y) - window->mouse_region_y) * SNES_HEIGHT_EXTENDED /
        (window->mouse_region_height <= 0 ? 1 : window->mouse_region_height);

    if (!window->config->pointer_is_visible)
    {
        if (!S9xIsMousePluggedIn ())
            window->show_mouse_cursor ();
    }

    gettimeofday (&(window->config->pointer_timestamp), NULL);

    return FALSE;
}

gboolean
event_button_press (GtkWidget      *widget,
                    GdkEventButton *event,
                    gpointer       user_data)
{
    switch (event->button)
    {
        case 1:
            S9xReportButton (BINDING_MOUSE_BUTTON0, 1);
            break;
        case 2:
            S9xReportButton (BINDING_MOUSE_BUTTON1, 1);
            break;
        case 3:
            S9xReportButton (BINDING_MOUSE_BUTTON2, 1);
            break;
    }

    return FALSE;
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

    return FALSE;
}

static void
event_fullscreen (GtkWidget *widget, gpointer data)
{
    Snes9xWindow *window = (Snes9xWindow *) data;

    if (!window->config->fullscreen)
        window->enter_fullscreen_mode ();
    else
        window->leave_fullscreen_mode ();

    return;
}


static void
event_exact_pixels_1x (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->resize_to_multiple (1);

    return;
}

static void
event_exact_pixels_2x (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->resize_to_multiple (2);

    return;
}

static void
event_exact_pixels_3x (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->resize_to_multiple (3);

    return;
}

static void
event_exact_pixels_4x (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->resize_to_multiple (4);

    return;
}

static void
event_exact_pixels_5x (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->resize_to_multiple (5);

    return;
}

static void
event_record_movie (GtkWidget *widget, gpointer data)
{
    if (S9xMovieActive ())
        S9xMovieStop (FALSE);

    S9xMovieCreate (S9xChooseMovieFilename (FALSE),
                    0xFF,
                    MOVIE_OPT_FROM_RESET,
                    NULL,
                    0);
    return;
}

static void
event_open_movie (GtkWidget *widget, gpointer data)
{
    if (S9xMovieActive ())
        S9xMovieStop (FALSE);

    S9xMovieOpen (S9xChooseMovieFilename (TRUE), FALSE);

    return;
}

static void
event_stop_recording (GtkWidget *widget, gpointer data)
{
    if (S9xMovieActive ())
        S9xMovieStop (FALSE);

    return;
}

static void
event_jump_to_frame (GtkWidget *widget, gpointer data)
{
    Snes9xWindow *window = (Snes9xWindow *) data;

    window->movie_seek_dialog ();

    return;
}

static void
event_reset (GtkWidget *widget, gpointer data)
{
    S9xSoftReset ();

    return;
}

static void
event_hard_reset (GtkWidget *widget, gpointer data)
{
    S9xReset ();

    return;
}

static void
event_save_state (GtkWidget *widget, gpointer data)
{
    int  slot;
    char *name = (char *) gtk_buildable_get_name (GTK_BUILDABLE (widget));

    slot = atoi (&(name[11]));

    S9xQuickSaveSlot (slot);

    return;
}

static void
event_save_state_file (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->save_state_dialog ();

    return;
}

static void
event_load_state (GtkWidget *widget, gpointer data)
{
    int  slot;
    char *name = (char *) gtk_buildable_get_name (GTK_BUILDABLE (widget));

    slot = atoi (&(name[11]));

    S9xQuickLoadSlot (slot);

    return;
}

static void
event_load_state_file (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->load_state_dialog ();

    return;
}

static void
event_open_rom (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->open_rom_dialog ();

    return;
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

    return;
}

static void
event_save_spc (GtkWidget *widget, gpointer data)
{
    ((Snes9xWindow *) data)->save_spc_dialog ();

    return;
}

static gboolean
event_focus_in (GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
    ((Snes9xWindow *) data)->focus_notify (TRUE);

    return FALSE;
}

static gboolean
event_focus_out (GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
    ((Snes9xWindow *) data)->focus_notify (FALSE);

    return FALSE;
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

/*    else if (!strcasecmp (name, "multitap1"))
    {
        S9xSetController (0, CTL_MP5, 0, 1, 2, 3);
    } */

    else if (!strcasecmp (name, "multitap2"))
    {
        S9xSetController (1, CTL_MP5, 1, 2, 3, 4);
    }

    return;
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
        { "hard_reset", G_CALLBACK (event_hard_reset) },
        { "on_port_activate", G_CALLBACK (event_port) },
        { "load_save_state", G_CALLBACK (event_load_state) },
        { "load_state_file", G_CALLBACK (event_load_state_file) },
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
        { "show_statusbar", G_CALLBACK (event_show_statusbar) },
        { "exact_1x", G_CALLBACK (event_exact_pixels_1x) },
        { "exact_2x", G_CALLBACK (event_exact_pixels_2x) },
        { "exact_3x", G_CALLBACK (event_exact_pixels_3x) },
        { "exact_4x", G_CALLBACK (event_exact_pixels_4x) },
        { "exact_5x", G_CALLBACK (event_exact_pixels_5x) },
        { "open_multicart", G_CALLBACK (event_open_multicart) },

        { NULL, NULL }
    };

    user_pause             = 0;
    user_rewind            = 0;
    sys_pause              = 0;
    last_width             = -1;
    last_height            = -1;
    this->config           = config;
    empty_cursor           = NULL;
    default_cursor         = NULL;
    recent_menu            = NULL;
    fullscreen_state       = 0;
    maximized_state        = 0;
    focused                = 1;
    paused_from_focus_loss = 0;

    if (gtk_icon_theme_has_icon (gtk_icon_theme_get_default (), "snes9x"))
    {
        gtk_window_set_default_icon_name ("snes9x");
    }
    else
    {
        icon = gdk_pixbuf_new_from_inline (-1, app_icon, FALSE, NULL);
        gtk_window_set_default_icon (icon);
    }

    drawing_area = GTK_DRAWING_AREA (get_widget ("drawingarea"));
    gtk_widget_set_double_buffered (GTK_WIDGET (drawing_area), FALSE);

    gtk_widget_realize (window);
    gtk_widget_realize (GTK_WIDGET (drawing_area));
#ifndef USE_GTK3
    gdk_window_set_back_pixmap (gtk_widget_get_window (window), NULL, FALSE);
    gdk_window_set_back_pixmap (gtk_widget_get_window (GTK_WIDGET (drawing_area)), NULL, FALSE);
#endif

    gtk_check_menu_item_set_active (
        GTK_CHECK_MENU_ITEM (get_widget ("show_statusbar_item")),
        config->statusbar_visible ? 1 : 0);

#ifdef NETPLAY_SUPPORT
#else
    gtk_widget_hide (get_widget ("open_netplay_item"));
    gtk_widget_hide (get_widget ("netplay_separator"));
    gtk_widget_hide (get_widget ("sync_clients_item"));
    gtk_widget_hide (get_widget ("sync_clients_separator"));
#endif

#ifdef USE_GTK3
    g_signal_connect_data (drawing_area,
                           "draw",
                           G_CALLBACK (event_drawingarea_draw),
                           this,
                           NULL,
                           (GConnectFlags) 0);

    gtk_window_set_has_resize_grip (GTK_WINDOW (window), FALSE);

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

    default_cursor = gdk_cursor_new (GDK_LEFT_PTR);
    gdk_window_set_cursor (gtk_widget_get_window (window), default_cursor);

    resize (config->window_width, config->window_height);

    return;
}

extern const gtk_splash_t gtk_splash;

void
Snes9xWindow::expose (void)
{
    if (last_width < 0)
    {
        if (!(config->fullscreen) && !(maximized_state))
        {
            config->window_width = get_width ();
            config->window_height = get_height ();
        }

        /* Load splash image (RGB24) into Snes9x buffer (RGB15) */
        last_width = 256;
        last_height = 224;

        uint16 *screen_ptr = GFX.Screen;
        const unsigned char *splash_ptr = gtk_splash.pixel_data;

        for (int y = 0; y < 224; y++, screen_ptr += (GFX.Pitch / 2))
        {
            for (int x = 0; x < 256; x++)
            {
                unsigned int red = *splash_ptr++;
                unsigned int green = *splash_ptr++;
                unsigned int blue = *splash_ptr++;

                screen_ptr[x] = ((red   & 0xF8) << 7) +
                                ((green & 0xF8) << 2) +
                                ((blue  & 0xF8) >> 3);
            }
        }
   }

    S9xDisplayRefresh (last_width, last_height);

    if (!(config->fullscreen))
    {
        config->window_width = get_width ();
        config->window_height = get_height ();
    }

    if (is_paused ()
#ifdef NETPLAY_SUPPORT
            || NetPlay.Paused
#endif
    )
    {
        S9xDeinitUpdate (last_width, last_height);
    }

    return;
}

void
Snes9xWindow::focus_notify (int state)
{
    focused = state ? 1 : 0;

    if (!state && config->pause_emulation_on_switch)
    {
        sys_pause++;
        propagate_pause_state ();
        paused_from_focus_loss = TRUE;
    }

    if (state && paused_from_focus_loss)
    {
        unpause_from_focus_change ();
        paused_from_focus_loss = FALSE;
    }

    return;
}

void
Snes9xWindow::open_multicart_dialog (void)
{
    int result;
    GtkBuilderWindow *dialog = new GtkBuilderWindow ("multicart_dialog");
    GtkFileChooser *slota, *slotb;
    GtkWidget *multicart_dialog = GTK_WIDGET (dialog->get_window ());

    gtk_window_set_transient_for (dialog->get_window (), get_window ());

    pause_from_focus_change ();

    slota = GTK_FILE_CHOOSER (dialog->get_widget ("multicart_slota"));
    slotb = GTK_FILE_CHOOSER (dialog->get_widget ("multicart_slotb"));

    gtk_file_chooser_set_current_folder (slota, config->last_directory);
    gtk_file_chooser_set_current_folder (slotb, config->last_directory);

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

        Settings.Multi = TRUE;

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

    return;
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
                                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                              GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
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

        sprintf (default_name, "%s.smv", def);

        dialog = gtk_file_chooser_dialog_new (_("New SNES Movie"),
                                              GTK_WINDOW (this->window),
                                              GTK_FILE_CHOOSER_ACTION_SAVE,
                                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                              GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
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
        Settings.Multi = FALSE;
        try_open_rom (filename);

        g_free (filename);
    }

    unpause_from_focus_change ();

    return;
}

int
Snes9xWindow::try_open_rom (const char *filename)
{
    pause_from_focus_change ();

    Settings.Multi = FALSE;

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

        return 1;
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
                FALSE
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

    return 0;
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
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
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

    return;
}

void
Snes9xWindow::movie_seek_dialog (void)
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

    return;
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

    sprintf (default_name, "%s.sst", def);

    dialog = gtk_file_chooser_dialog_new (_("Save State"),
                                          GTK_WINDOW (this->window),
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
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

    return;
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

    sprintf (default_name, "%s.spc", def);

    dialog = gtk_file_chooser_dialog_new (_("Save SPC file..."),
                                          GTK_WINDOW (this->window),
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
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

    return;
}

void
Snes9xWindow::set_menu_item_selected (const char *name)
{
    GtkCheckMenuItem *item;

    item = GTK_CHECK_MENU_ITEM (get_widget (name));

    gtk_check_menu_item_set_active (item, 1);

    return;
}

static gboolean
statusbar_timeout (gpointer data)
{
    gtk_statusbar_pop (GTK_STATUSBAR (data),
                       gtk_statusbar_get_context_id (GTK_STATUSBAR (data),
                                                     "info"));

    return FALSE;
}

void
Snes9xWindow::show_status_message (const char *message)
{
    GtkStatusbar *statusbar = GTK_STATUSBAR (get_widget ("statusbar"));

    gtk_statusbar_pop (statusbar, gtk_statusbar_get_context_id (statusbar, "info"));
    gtk_statusbar_push (statusbar, gtk_statusbar_get_context_id (statusbar, "info"), message);

    g_timeout_add (2000, statusbar_timeout, statusbar);

    return;
}

void
Snes9xWindow::update_statusbar (void)
{
    GtkStatusbar *bar = GTK_STATUSBAR (get_widget ("statusbar"));
    char         status_string[256];
    char         title_string[1024];

    if (!config->rom_loaded)
    {
        snprintf (title_string, 1024, "Snes9x");
        status_string[0] = '\0';
    }
    else
    {
#ifdef NETPLAY_SUPPORT
        if (config->netplay_activated)
        {
            if (config->netplay_server_up)
            {
                snprintf (status_string,
                          256,
                          _("%sHosting NetPlay - %s"),
                          is_paused () || NetPlay.Paused ? _("Paused - ") : "",
                          S9xBasenameNoExt (Memory.ROMFilename));
            }
            else
            {
                snprintf (status_string,
                          256,
                          _("%s%s on NetPlay %s:%d - Player %d"),
                          is_paused () || NetPlay.Paused ? _("Paused - ") : "",
                          S9xBasenameNoExt (Memory.ROMFilename),
                          NetPlay.ServerHostName,
                          NetPlay.Port,
                          NetPlay.Player);
            }

        }
        else
#endif
        {
            snprintf (status_string,
                      256,
                      "%s%s",
                      is_paused () ? _("Paused - ") : "",
                      S9xBasenameNoExt (Memory.ROMFilename));
        }

        snprintf (title_string, 1024, "%s - Snes9x", S9xBasenameNoExt (Memory.ROMFilename));
    }

    gtk_window_set_title (GTK_WINDOW (window), title_string);
    gtk_statusbar_pop (bar, gtk_statusbar_get_context_id (bar, "none"));
    gtk_statusbar_push (bar, gtk_statusbar_get_context_id (bar, "none"), status_string);

    return;
}

void
Snes9xWindow::show_rom_info (void)
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

    return;
}

void
Snes9xWindow::configure_widgets (void)
{
    enable_widget ("continue_item", config->rom_loaded);
    enable_widget ("pause_item", config->rom_loaded);
    enable_widget ("reset_item", config->rom_loaded);
    enable_widget ("controller_ports_item", config->rom_loaded);
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

#ifdef NETPLAY_SUPPORT
    enable_widget ("sync_clients_item",
                   config->rom_loaded &&
                   Settings.NetPlay   &&
                   Settings.NetPlayServer);
#endif

    if (config->default_esc_behavior != ESC_TOGGLE_MENUBAR)
    {
        enable_widget ("fullscreen_item", config->rom_loaded);

        config->ui_visible = TRUE;

        if (!config->fullscreen)
        {
            gtk_widget_show (get_widget ("menubar"));

            if (config->statusbar_visible)
                gtk_widget_show (get_widget ("statusbar"));
            else
                gtk_widget_hide (get_widget ("statusbar"));
        }
        else
        {
            gtk_widget_hide (get_widget ("menubar"));
            gtk_widget_hide (get_widget ("statusbar"));
        }

        gtk_widget_hide (get_widget ("hide_ui"));
    }
    else
    {
        enable_widget ("fullscreen_item", TRUE);

        gtk_widget_show (get_widget ("hide_ui"));

        if (config->ui_visible)
        {
            gtk_widget_show (get_widget ("menubar"));
            if (config->statusbar_visible)
                gtk_widget_show (get_widget ("statusbar"));
            else
                gtk_widget_hide (get_widget ("statusbar"));
        }
        else
        {
            gtk_widget_hide (get_widget ("menubar"));
            gtk_widget_hide (get_widget ("statusbar"));
        }
    }

    propagate_pause_state ();

    if (config->rom_loaded && !Settings.Paused)
        hide_mouse_cursor ();
    else
        show_mouse_cursor ();

    return;
}

void
Snes9xWindow::set_mouseable_area (int x, int y, int width, int height)
{
    mouse_region_x      = x;
    mouse_region_y      = y;
    mouse_region_width  = width;
    mouse_region_height = height;

    return;
}

void
Snes9xWindow::reset_screensaver (void)
{
    if (!focused)
        return;

    XResetScreenSaver (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()));

    config->screensaver_needs_reset = FALSE;

    return;
}

void
Snes9xWindow::toggle_fullscreen_mode (void)
{
    if (config->fullscreen)
        leave_fullscreen_mode ();
    else
        enter_fullscreen_mode ();
}

void
Snes9xWindow::enter_fullscreen_mode (void)
{
    int rom_loaded = config->rom_loaded;

    if (config->fullscreen)
        return;

    config->rom_loaded = 0;

    nfs_width = config->window_width;
    nfs_height = config->window_height;

    gtk_window_get_position (GTK_WINDOW (window), &nfs_x, &nfs_y);

    /* Make sure everything is done synchronously */
    gdk_display_sync (gdk_display_get_default ());
    gtk_window_fullscreen (GTK_WINDOW (window));

#ifdef USE_XRANDR
    if (config->change_display_resolution)
    {
        int mode = -1;

        for (int i = 0; i < config->xrr_num_sizes; i++)
        {
            if (config->xrr_sizes[i].width == config->xrr_width &&
                config->xrr_sizes[i].height == config->xrr_height)
            {
                mode = i;
            }
        }

        if (mode < 0)
        {
            config->change_display_resolution = 0;
        }
        else
        {
            GdkDisplay *gdk_display = gtk_widget_get_display (window);
            Display *display = gdk_x11_display_get_xdisplay (gdk_display);
            GdkScreen *screen = gtk_widget_get_screen (window);
            GdkWindow *root = gdk_screen_get_root_window (screen);

            gdk_display_sync (gdk_display_get_default ());
            XRRSetScreenConfig (display,
                                config->xrr_config,
                                GDK_COMPAT_WINDOW_XID (root),
                                (SizeID) mode,
                                config->xrr_rotation,
                                CurrentTime);
        }
    }
#endif

    gdk_display_sync (gdk_display_get_default ());
    gtk_window_present (GTK_WINDOW (window));

    config->fullscreen = 1;
    config->rom_loaded = rom_loaded;

    /* If we're running a game, disable ui when entering fullscreen */
    if (!Settings.Paused && config->rom_loaded)
        config->ui_visible = FALSE;

    configure_widgets ();

    return;
}

void
Snes9xWindow::leave_fullscreen_mode (void)
{
    int rom_loaded = config->rom_loaded;

    if (!config->fullscreen)
        return;

    config->rom_loaded = 0;

#ifdef USE_XRANDR
    if (config->change_display_resolution)
    {
        gtk_widget_hide (window);

        GdkDisplay *gdk_display = gtk_widget_get_display (window);
        Display *display = gdk_x11_display_get_xdisplay (gdk_display);
        GdkScreen *screen = gtk_widget_get_screen (window);
        GdkWindow *root = gdk_screen_get_root_window (screen);

        XRRSetScreenConfig (display,
                            config->xrr_config,
                            GDK_COMPAT_WINDOW_XID (root),
                            (SizeID) config->xrr_original_size,
                            config->xrr_rotation,
                            CurrentTime);
    }
#endif

    gtk_window_unfullscreen (GTK_WINDOW (window));

#ifdef USE_XRANDR
    if (config->change_display_resolution)
    {
        gtk_widget_show (window);
    }
#endif

    resize (nfs_width, nfs_height);
    gtk_window_move (GTK_WINDOW (window), nfs_x, nfs_y);

    config->rom_loaded = rom_loaded;

    config->fullscreen = 0;

    configure_widgets ();

    return;
}

void
Snes9xWindow::toggle_statusbar (void)
{
    GtkWidget     *item;
    GtkAllocation allocation;
    int           width = 0;
    int           height = 0;

    item = get_widget ("menubar");
    gtk_widget_get_allocation (item, &allocation);
    height += gtk_widget_get_visible (item) ? allocation.height : 0;

    item = get_widget ("drawingarea");
    gtk_widget_get_allocation (item, &allocation);
    height += allocation.height;
    width = allocation.width;

    config->statusbar_visible = !config->statusbar_visible;
    configure_widgets ();

    item = get_widget ("statusbar");
    gtk_widget_get_allocation (item, &allocation);
    height += gtk_widget_get_visible (item) ? allocation.height : 0;

    resize (width, height);

    return;
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

    item = get_widget ("statusbar");
    gtk_widget_get_allocation (item, &allocation);
    y_padding += gtk_widget_get_visible (item) ? allocation.height : 0;

    resize (width, height + y_padding);

    return;
}

void
Snes9xWindow::hide_mouse_cursor (void)
{
    if (!empty_cursor)
    {
        empty_cursor = gdk_cursor_new (GDK_BLANK_CURSOR);
    }

    gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (drawing_area)),
                           empty_cursor);
    config->pointer_is_visible = FALSE;

    return;
}

void
Snes9xWindow::show_mouse_cursor (void)
{
    gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (drawing_area)),
                           NULL);
    config->pointer_is_visible = TRUE;

    return;
}

void
Snes9xWindow::show (void)
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
        gtk_recent_chooser_set_local_only (chooser, TRUE);
        gtk_recent_chooser_set_show_icons (chooser, FALSE);
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

    return;
}

void
Snes9xWindow::propagate_pause_state (void)
{
    int oldpause = Settings.Paused;

    Settings.Paused = (sys_pause || user_pause || !(config->rom_loaded));

    if (Settings.Paused != oldpause)
    {
        if (!is_paused ())
        {
            S9xSoundStart ();
            if (config->rom_loaded)
                enable_widget ("pause_item", TRUE);

            S9xDisplayClearBuffers ();
        }
        else
        {
            S9xSoundStop ();
            enable_widget ("pause_item", FALSE);

        }

        configure_widgets ();
        update_statusbar ();
    }

    return;
}

void
Snes9xWindow::toggle_ui (void)
{
    config->ui_visible = !config->ui_visible;

    configure_widgets ();

    return;
}

/* gui_[un]pause Handles when system needs to pause the emulator */
void
Snes9xWindow::pause_from_focus_change (void)
{
    sys_pause += config->modal_dialogs;

    propagate_pause_state ();

    return;
}

void
Snes9xWindow::unpause_from_focus_change (void)
{
    if (--sys_pause < 0)
        sys_pause = 0;
    propagate_pause_state ();

    return;
}

/* client_[un]pause Handles when user manually chooses to pause */
void
Snes9xWindow::pause_from_user (void)
{
    user_pause = TRUE;
    propagate_pause_state ();

    return;
}

void
Snes9xWindow::unpause_from_user (void)
{
    user_pause = FALSE;
    propagate_pause_state ();

    return;
}

unsigned char
Snes9xWindow::is_paused (void)
{
    if (user_pause || sys_pause || Settings.Paused || !(config->rom_loaded))
        return TRUE;

    return FALSE;
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
                                    TRUE);
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
                                    TRUE);
    }

    return;
}

void
Snes9xWindow::update_accels (void)
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

    return;
}

void
Snes9xWindow::resize_to_multiple (int factor)
{
    int h = (config->overscan ? 239 : 224) * factor;
    int w = h * S9xGetAspect () + 0.5;

    resize_viewport (w, h);

    return;
}
