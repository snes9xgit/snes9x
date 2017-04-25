#include <string>
#include <stdlib.h>
#include <gdk/gdkkeysyms.h>
#ifdef USE_GTK3
#include <gdk/gdkkeysyms-compat.h>
#endif

#include "gtk_preferences.h"
#include "gtk_config.h"
#include "gtk_s9xcore.h"
#include "gtk_control.h"
#include "gtk_sound.h"
#include "gtk_display.h"
#include "gtk_binding.h"

#define SAME_GAME _("Same location as current game")

gboolean
snes9x_preferences_open (GtkWidget *widget,
                         gpointer  data)
{
    Snes9xWindow      *window = ((Snes9xWindow *) data);
    Snes9xConfig      *config = window->config;

    window->pause_from_focus_change ();

    Snes9xPreferences preferences (config);
    gtk_window_set_transient_for (preferences.get_window (),
                                  window->get_window ());

#ifdef USE_JOYSTICK
    config->set_joystick_mode (JOY_MODE_GLOBAL);
#endif

    preferences.show ();
    window->unpause_from_focus_change ();

#ifdef USE_JOYSTICK
    config->set_joystick_mode (JOY_MODE_INDIVIDUAL);
#endif

    config->reconfigure ();
    window->update_accels ();

    return TRUE;
}

static void
event_sram_folder_browse (GtkButton *widget, gpointer data)
{
    ((Snes9xPreferences *) data)->browse_folder_dialog ();

    return;
}

#ifdef USE_JOYSTICK
static void
event_calibrate (GtkButton *widget, gpointer data)
{
    ((Snes9xPreferences *) data)->calibration_dialog ();

    return;
}
#endif

static void
event_control_toggle (GtkToggleButton *widget, gpointer data)
{
    Snes9xPreferences    *window = (Snes9xPreferences *) data;
    static unsigned char toggle_lock = 0;
    const gchar          *name;
    bool                 state;

    if (toggle_lock)
    {
        return;
    }

    window->last_toggled = widget;
    name = gtk_buildable_get_name (GTK_BUILDABLE (widget));
    state = gtk_toggle_button_get_active (widget);

    toggle_lock = 1;

    for (int i = 0; b_links[i].button_name; i++)
    {
        if (strcasecmp (name, b_links[i].button_name))
        {
            gtk_toggle_button_set_active (
                GTK_TOGGLE_BUTTON (window->get_widget (b_links[i].button_name)),
                FALSE);
        }
    }

    gtk_toggle_button_set_active (widget, state);

    toggle_lock = 0;

    return;
}

static gboolean
event_key_press (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    Binding           key_binding;
    int               focus;
    GtkNotebook       *notebook;
    GtkToggleButton   *toggle;
    Snes9xPreferences *window = (Snes9xPreferences *) user_data;

    if ((focus = window->get_focused_binding ()) < 0)
    {
        return FALSE; /* Don't keep key for ourselves */
    }

    /* Allow modifier keys to be used if page is set to the joypad bindings. */
    notebook = GTK_NOTEBOOK (window->get_widget ("preferences_notebook"));
    toggle = GTK_TOGGLE_BUTTON (window->get_widget ("use_modifiers"));

    if (gtk_notebook_get_current_page (notebook) != 4 ||
        !gtk_toggle_button_get_active (toggle))
    {
        /* Don't allow modifiers that we track to be bound */
        if (event->keyval == GDK_Control_L ||
            event->keyval == GDK_Control_R ||
            event->keyval == GDK_Shift_L   ||
            event->keyval == GDK_Shift_R   ||
            event->keyval == GDK_Alt_L     ||
            event->keyval == GDK_Alt_R)
        {
            return FALSE;
        }
    }

    key_binding = Binding (event);

    /* Allows ESC key to clear the key binding */
    if (event->keyval == GDK_Escape)
    {
        if (event->state & GDK_SHIFT_MASK)
        {
            key_binding.clear ();
        }
        else
        {
            window->focus_next ();
            return TRUE;
        }
    }

    window->store_binding (b_links[focus].button_name, key_binding);

    return TRUE;
}

static void
event_ntsc_composite_preset (GtkButton *widget, gpointer data)
{
    Snes9xPreferences *window = (Snes9xPreferences *) data;
    window->config->ntsc_setup = snes_ntsc_composite;
    window->load_ntsc_settings ();

    return;
}

static void
event_ntsc_svideo_preset (GtkButton *widget, gpointer data)
{
    Snes9xPreferences *window = (Snes9xPreferences *) data;
    window->config->ntsc_setup = snes_ntsc_svideo;
    window->load_ntsc_settings ();

    return;
}

static void
event_ntsc_rgb_preset (GtkButton *widget, gpointer data)
{
    Snes9xPreferences *window = (Snes9xPreferences *) data;
    window->config->ntsc_setup = snes_ntsc_rgb;
    window->load_ntsc_settings ();

    return;
}

static void
event_ntsc_monochrome_preset (GtkButton *widget, gpointer data)
{
    Snes9xPreferences *window = (Snes9xPreferences *) data;
    window->config->ntsc_setup = snes_ntsc_monochrome;
    window->load_ntsc_settings ();

    return;
}


static void
event_swap_with (GtkButton *widget, gpointer data)
{
    ((Snes9xPreferences *) data)->swap_with ();

    return;
}

static void
event_reset_current_joypad (GtkButton *widget, gpointer data)
{
    ((Snes9xPreferences *) data)->reset_current_joypad ();

    return;
}

static void
event_shader_select (GtkButton *widget, gpointer data)
{
#ifdef USE_OPENGL
    Snes9xPreferences *window = (Snes9xPreferences *) data;
    GtkWidget     *dialog;
    char          *filename = NULL;
    gint          result;
    GtkEntry      *entry;

    entry = GTK_ENTRY (window->get_widget ("fragment_shader"));

    dialog = gtk_file_chooser_dialog_new ("Select Shader File",
                                          window->get_window (),
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                          NULL);

    if (strcmp (gui_config->last_directory, ""))
    {
        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog),
                                             gui_config->last_directory);
    }

    if (strlen (gtk_entry_get_text (entry)))
        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog),
                                       gtk_entry_get_text (entry));


    result = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_hide (dialog);

    if (result == GTK_RESPONSE_ACCEPT)
    {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        if (filename != NULL)
        {
            gtk_entry_set_text (entry, filename);
            g_free (filename);
        }
    }

    gtk_widget_destroy (dialog);

    return;
#endif
}

static void
event_game_data_clear (GtkEntry *entry,
                       GtkEntryIconPosition icon_pos,
                       GdkEvent *event,
                       gpointer  user_data)
{
    gtk_entry_set_text (entry, SAME_GAME);

    return;
}

static void
event_game_data_browse (GtkButton *widget, gpointer data)
{
    Snes9xPreferences *window = (Snes9xPreferences *) data;
    GtkWidget     *dialog;
    char          *filename = NULL;
    gint          result;
    GtkEntry      *entry;
    char          entry_name[256];

    strcpy (entry_name, gtk_buildable_get_name (GTK_BUILDABLE (widget)));

    sprintf (strstr (entry_name, "_browse"), "_directory");
    entry = GTK_ENTRY (window->get_widget (entry_name));

    dialog = gtk_file_chooser_dialog_new ("Select directory",
                                          window->get_window (),
                                          GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                          NULL);

    if (strcmp (gui_config->last_directory, ""))
    {
        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog),
                                             gui_config->last_directory);
    }

    if (strcmp (gtk_entry_get_text (entry), SAME_GAME))
        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog),
                                       gtk_entry_get_text (entry));

    result = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_hide (dialog);

    if (result == GTK_RESPONSE_ACCEPT)
    {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        if (filename != NULL)
        {
            gtk_entry_set_text (entry, filename);
            g_free (filename);
        }
    }

    gtk_widget_destroy (dialog);

    return;
}

static void
event_hw_accel_changed (GtkComboBox *widget, gpointer data)
{
    Snes9xPreferences *window = (Snes9xPreferences *) data;
    GtkComboBox       *combo =
                           GTK_COMBO_BOX (window->get_widget ("hw_accel"));
    int               value = gtk_combo_box_get_active (combo);

    value = window->hw_accel_value (value);

    switch (value)
    {
        case HWA_NONE:
            gtk_widget_show (window->get_widget ("bilinear_filter"));
            gtk_widget_hide (window->get_widget ("opengl_frame"));
            gtk_widget_hide (window->get_widget ("xv_frame"));
            break;
        case HWA_OPENGL:
            gtk_widget_show (window->get_widget ("bilinear_filter"));
            gtk_widget_show (window->get_widget ("opengl_frame"));
            gtk_widget_hide (window->get_widget ("xv_frame"));
            break;
        case HWA_XV:
            gtk_widget_hide (window->get_widget ("bilinear_filter"));
            gtk_widget_show (window->get_widget ("xv_frame"));
            gtk_widget_hide (window->get_widget ("opengl_frame"));
            break;
    }

    return;
}

static void
event_scale_method_changed (GtkComboBox *widget, gpointer user_data)
{
    Snes9xPreferences *window = (Snes9xPreferences *) user_data;
    GtkComboBox       *combo =
        GTK_COMBO_BOX (window->get_widget ("scale_method_combo"));

    if (gtk_combo_box_get_active (combo) == FILTER_NTSC)
    {
        gtk_widget_show (window->get_widget ("ntsc_frame"));
    }
    else
    {
        gtk_widget_hide (window->get_widget ("ntsc_frame"));
    }

    if (gtk_combo_box_get_active (combo) == FILTER_SCANLINES)
    {
        gtk_widget_show (window->get_widget ("scanline_filter_frame"));
    }
    else
    {
        gtk_widget_hide (window->get_widget ("scanline_filter_frame"));
    }


    return;
}

static void
event_control_combo_changed (GtkComboBox *widget, gpointer user_data)
{
    Snes9xPreferences *window = (Snes9xPreferences *) user_data;

    window->bindings_to_dialog (gtk_combo_box_get_active (widget));

    return;
}

#ifdef USE_JOYSTICK

static gboolean
poll_joystick (gpointer data)
{
    Snes9xPreferences *window = (Snes9xPreferences *) data;
    JoyEvent          event;
    Binding           binding;
    int               i, focus;

    for (i = 0; window->config->joystick[i]; i++)
    {
        while (window->config->joystick[i]->get_event (&event))
        {
            if (event.state == JOY_PRESSED)
            {
                if ((focus = window->get_focused_binding ()) >= 0)
                {
                    binding = Binding (i,
                                       event.parameter,
                                       window->config->joystick_threshold);

                    window->store_binding (b_links[focus].button_name,
                                           binding);

                    window->config->flush_joysticks ();
                    return TRUE;
                }

            }
        }
    }

    return TRUE;
}

void
Snes9xPreferences::calibration_dialog (void)
{
    GtkWidget *dialog;

    config->joystick_register_centers ();
    dialog = gtk_message_dialog_new (NULL,
                                     (GtkDialogFlags) 0,
                                     GTK_MESSAGE_INFO,
                                     GTK_BUTTONS_OK,
                                     _("Current joystick centers have been saved."));
    gtk_window_set_title (GTK_WINDOW (dialog), _("Calibration Complete"));

    gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_widget_destroy (dialog);

    return;
}

#endif

static void
event_input_rate_changed (GtkRange *range, gpointer data)
{
    char text[256];
    GtkLabel *label = GTK_LABEL (data);
    double value = gtk_range_get_value (range) / 32040.0 * 60.09881389744051;

    snprintf (text, 256, "%.4f hz", value);

    gtk_label_set_text (label, text);

    return;
}

static void
event_about_clicked (GtkButton *widget, gpointer data)
{
    std::string version_string;
    GtkBuilderWindow *about_dialog = new GtkBuilderWindow ("about_dialog");
    Snes9xPreferences *preferences = (Snes9xPreferences *) data;
    PangoFontDescription *monospace;

    ((version_string += _("Snes9x version: ")) += VERSION) += ", ";
    ((version_string += _("GTK+ port version: ")) += SNES9X_GTK_VERSION) += "\n";
    (version_string += SNES9X_GTK_AUTHORS) += "\n";
    (version_string += _("English localization by Brandon Wright")) += "\n";

#if defined(USE_OPENGL) || defined(USE_XV) || defined(USE_XRANDR) \
    || defined(USE_JOYSTICK) || defined(NETPLAY_SUPPORT)
    version_string += _("\nFeatures enabled:<i>");
#else
    version_string += _(" Only barebones features enabled<i>");
#endif
#ifdef USE_OPENGL
    version_string += _(" OpenGL");
#endif
#ifdef USE_XV
    version_string += _(" XVideo");
#endif
#ifdef USE_XRANDR
    version_string += _(" XRandR");
#endif
#ifdef USE_JOYSTICK
    version_string += _(" Joystick");
#endif
#ifdef NETPLAY_SUPPORT
    version_string += _(" NetPlay");
#endif
    version_string += "</i>";

    GtkLabel *version_string_label = GTK_LABEL (about_dialog->get_widget ("version_string_label"));
    gtk_label_set_label (version_string_label, version_string.c_str ());
    gtk_label_set_justify (version_string_label, GTK_JUSTIFY_LEFT);

    gtk_widget_hide (about_dialog->get_widget ("preferences_splash"));

    monospace = pango_font_description_from_string ("Monospace 7");
#ifdef USE_GTK3
    gtk_widget_override_font (about_dialog->get_widget ("about_text_view"),
                              monospace);
#else
    gtk_widget_modify_font (about_dialog->get_widget ("about_text_view"),
                            monospace);
#endif
    pango_font_description_free (monospace);

    gtk_window_set_transient_for (about_dialog->get_window (),
                                  preferences->get_window ());

    gtk_dialog_run (GTK_DIALOG (about_dialog->get_window ()));

    delete about_dialog;

    return;
}

Snes9xPreferences::Snes9xPreferences (Snes9xConfig *config) :
    GtkBuilderWindow ("preferences_window")
{
    GtkBuilderWindowCallbacks callbacks[] =
    {
        { "control_toggle", G_CALLBACK (event_control_toggle) },
        { "on_key_press", G_CALLBACK (event_key_press) },
        { "control_combo_changed", G_CALLBACK (event_control_combo_changed) },
        { "sram_folder_browse", G_CALLBACK (event_sram_folder_browse) },
        { "scale_method_changed", G_CALLBACK (event_scale_method_changed) },
        { "hw_accel_changed", G_CALLBACK (event_hw_accel_changed) },
        { "reset_current_joypad", G_CALLBACK (event_reset_current_joypad) },
        { "swap_with", G_CALLBACK (event_swap_with) },
        { "ntsc_composite_preset", G_CALLBACK (event_ntsc_composite_preset) },
        { "ntsc_svideo_preset", G_CALLBACK (event_ntsc_svideo_preset) },
        { "ntsc_rgb_preset", G_CALLBACK (event_ntsc_rgb_preset) },
        { "ntsc_monochrome_preset", G_CALLBACK (event_ntsc_monochrome_preset) },
        { "shader_select", G_CALLBACK (event_shader_select) },
        { "game_data_browse", G_CALLBACK (event_game_data_browse) },
        { "game_data_clear", G_CALLBACK (event_game_data_clear) },
        { "about_clicked", G_CALLBACK (event_about_clicked) },
#ifdef USE_JOYSTICK
        { "calibrate", G_CALLBACK (event_calibrate) },
#endif
        { NULL, NULL }
    };

    last_toggled = NULL;
    this->config = config;

    gtk_widget_realize (window);

    signal_connect (callbacks);

    g_signal_connect_data (get_widget ("sound_input_rate"),
                           "value-changed",
                           G_CALLBACK (event_input_rate_changed),
                           get_widget ("relative_video_rate"),
                           NULL,
                           (GConnectFlags) 0);

    return;
}

Snes9xPreferences::~Snes9xPreferences (void)
{
    return;
}

void
Snes9xPreferences::load_ntsc_settings (void)
{
    set_slider ("ntsc_artifacts", config->ntsc_setup.artifacts);
    set_slider ("ntsc_bleed", config->ntsc_setup.bleed);
    set_slider ("ntsc_brightness", config->ntsc_setup.brightness);
    set_slider ("ntsc_contrast", config->ntsc_setup.contrast);
    set_slider ("ntsc_fringing", config->ntsc_setup.fringing);
    set_slider ("ntsc_gamma", config->ntsc_setup.gamma);
    set_slider ("ntsc_hue", config->ntsc_setup.hue);
    set_check  ("ntsc_merge_fields", config->ntsc_setup.merge_fields);
    set_slider ("ntsc_resolution", config->ntsc_setup.resolution);
    set_slider ("ntsc_saturation", config->ntsc_setup.saturation);
    set_slider ("ntsc_sharpness", config->ntsc_setup.sharpness);

    return;
}

void
Snes9xPreferences::store_ntsc_settings (void)
{
    config->ntsc_setup.artifacts    = get_slider ("ntsc_artifacts");
    config->ntsc_setup.bleed        = get_slider ("ntsc_bleed");
    config->ntsc_setup.brightness   = get_slider ("ntsc_brightness");
    config->ntsc_setup.contrast     = get_slider ("ntsc_contrast");
    config->ntsc_setup.fringing     = get_slider ("ntsc_fringing");
    config->ntsc_setup.gamma        = get_slider ("ntsc_gamma");
    config->ntsc_setup.hue          = get_slider ("ntsc_hue");
    config->ntsc_setup.merge_fields = get_check  ("ntsc_merge_fields");
    config->ntsc_setup.resolution   = get_slider ("ntsc_resolution");
    config->ntsc_setup.saturation   = get_slider ("ntsc_saturation");
    config->ntsc_setup.sharpness    = get_slider ("ntsc_sharpness");

    return;
}

void
Snes9xPreferences::move_settings_to_dialog (void)
{
    set_check ("full_screen_on_open",       config->full_screen_on_open);
    set_check ("show_frame_rate",           Settings.DisplayFrameRate);
    set_check ("change_display_resolution", config->change_display_resolution);
    set_check ("scale_to_fit",              config->scale_to_fit);
    set_check ("overscan",                  config->overscan);
    set_check ("multithreading",            config->multithreading);
    set_combo ("hires_effect",              config->hires_effect);
    set_check ("maintain_aspect_ratio",     config->maintain_aspect_ratio);
    set_combo ("aspect_ratio",              config->aspect_ratio);
    if (config->sram_directory[0] == '\0')
        set_entry_text ("sram_directory", SAME_GAME);
    else
        set_entry_text ("sram_directory", config->sram_directory);
    if (config->savestate_directory[0] == '\0')
        set_entry_text ("savestate_directory", SAME_GAME);
    else
        set_entry_text ("savestate_directory", config->savestate_directory);
    if (config->patch_directory[0] == '\0')
        set_entry_text ("patch_directory", SAME_GAME);
    else
        set_entry_text ("patch_directory", config->patch_directory);
    if (config->cheat_directory[0] == '\0')
        set_entry_text ("cheat_directory", SAME_GAME);
    else
        set_entry_text ("cheat_directory", config->cheat_directory);
    if (config->export_directory[0] == '\0')
        set_entry_text ("export_directory", SAME_GAME);
    else
        set_entry_text ("export_directory", config->export_directory);

    set_combo ("resolution_combo",          config->xrr_index);
    set_combo ("scale_method_combo",        config->scale_method);
    set_entry_value ("save_sram_after_sec", Settings.AutoSaveDelay);
    set_check ("block_invalid_vram_access", Settings.BlockInvalidVRAMAccessMaster);
    set_check ("upanddown",                 Settings.UpAndDown);
    set_combo ("default_esc_behavior",      config->default_esc_behavior);
    set_check ("prevent_screensaver",       config->prevent_screensaver);
    set_check ("force_inverted_byte_order", config->force_inverted_byte_order);
    set_check ("stereo_check",              Settings.Stereo);
    set_combo ("playback_combo",            7 - config->sound_playback_rate);
    set_combo ("hw_accel",                  combo_value (config->hw_accel));
    set_check ("pause_emulation_on_switch", config->pause_emulation_on_switch);
    set_spin  ("num_threads",               config->num_threads);
    set_check ("mute_sound_check",          config->mute_sound);
    set_check ("mute_sound_turbo_check",    config->mute_sound_turbo);
    set_spin  ("sound_buffer_size",         config->sound_buffer_size);
    set_slider ("sound_input_rate",         config->sound_input_rate);
    set_check ("sync_sound",                Settings.SoundSync);
    set_spin  ("rewind_buffer_size",        config->rewind_buffer_size);
    set_spin  ("rewind_granularity",        config->rewind_granularity);

    int num_sound_drivers = 0;
#ifdef USE_PORTAUDIO
    num_sound_drivers++;
#endif
#ifdef USE_OSS
    num_sound_drivers++;
#endif
#ifdef USE_JOYSTICK
    num_sound_drivers++;
#endif
#ifdef USE_ALSA
    num_sound_drivers++;
#endif
#ifdef USE_PULSEAUDIO
    num_sound_drivers++;
#endif

    if (config->sound_driver >= num_sound_drivers)
        config->sound_driver = 0;

    set_combo ("sound_driver",              config->sound_driver);

    if (config->scale_method == FILTER_NTSC)
    {
        gtk_widget_show (get_widget ("ntsc_frame"));
    }
    else
    {
        gtk_widget_hide (get_widget ("ntsc_frame"));
    }

    if (config->scale_method == FILTER_SCANLINES)
    {
        gtk_widget_show (get_widget ("scanline_filter_frame"));
    }
    else
    {
        gtk_widget_hide (get_widget ("scanline_filter_frame"));
    }

    load_ntsc_settings ();
    set_combo ("ntsc_scanline_intensity",   config->ntsc_scanline_intensity);
    set_combo ("scanline_filter_intensity", config->scanline_filter_intensity);

    set_combo ("frameskip_combo",
               Settings.SkipFrames == AUTO_FRAMERATE ?
                   0 : Settings.SkipFrames + 1);
    set_check ("bilinear_filter",           config->bilinear_filter);

#ifdef USE_OPENGL
    set_check ("sync_to_vblank",            config->sync_to_vblank);
    set_check ("sync_every_frame",          config->sync_every_frame);
    set_check ("use_pbos",                  config->use_pbos);
    set_combo ("pixel_format",              config->pbo_format);
    set_check ("npot_textures",             config->npot_textures);
    set_check ("use_shaders",               config->use_shaders);
    set_entry_text ("fragment_shader",      config->fragment_shader);
#endif

#ifdef USE_JOYSTICK
    set_spin ("joystick_threshold",         config->joystick_threshold);
#else
    gtk_widget_set_sensitive (get_widget ("joystick_box"), FALSE);
#endif

    /* Control bindings */
    memcpy (pad, config->pad, (sizeof (JoypadBinding)) * NUM_JOYPADS);
    memcpy (shortcut, config->shortcut, (sizeof (Binding)) * NUM_EMU_LINKS);
    bindings_to_dialog (0);

    set_combo ("joypad_to_swap_with", 0);

    return;
}

void
Snes9xPreferences::get_settings_from_dialog (void)
{
    int sound_needs_restart = 0;
    int gfx_needs_restart = 0;

    if ((config->sound_driver       != get_combo ("sound_driver"))          ||
        (config->mute_sound         != get_check ("mute_sound_check"))      ||
        (config->sound_buffer_size  != (int) get_spin ("sound_buffer_size"))||
        (Settings.Stereo            != get_check ("stereo_check"))          ||
        (config->sound_playback_rate !=
                     (7 - (get_combo ("playback_combo"))))                  ||
        (config->sound_input_rate   != get_slider ("sound_input_rate"))     ||
        (Settings.SoundSync         != get_check ("sync_sound"))
        )
    {
        sound_needs_restart = 1;
    }

#ifdef USE_XRANDR
    if ((config->change_display_resolution != get_check ("change_display_resolution") ||
            (config->change_display_resolution &&
                    (config->xrr_index != get_combo ("resolution_combo")))) &&
        config->fullscreen)
    {
        top_level->leave_fullscreen_mode ();
        config->xrr_index = get_combo ("resolution_combo");
        config->xrr_width = config->xrr_sizes[config->xrr_index].width;
        config->xrr_height = config->xrr_sizes[config->xrr_index].height;
        config->change_display_resolution = get_check ("change_display_resolution");
        top_level->enter_fullscreen_mode ();
    }
    else
    {
        config->xrr_index = get_combo ("resolution_combo");
        config->xrr_width = config->xrr_sizes[config->xrr_index].width;
        config->xrr_height = config->xrr_sizes[config->xrr_index].height;
    }
#endif

    config->change_display_resolution = get_check ("change_display_resolution");

    if (config->multithreading != get_check ("multithreading"))
        gfx_needs_restart = 1;

    if (config->hw_accel != hw_accel_value (get_combo ("hw_accel")))
        gfx_needs_restart = 1;

    if (config->force_inverted_byte_order != get_check ("force_inverted_byte_order"))
        gfx_needs_restart = 1;

#ifdef USE_OPENGL
#endif

    config->full_screen_on_open       = get_check ("full_screen_on_open");
    Settings.DisplayFrameRate         = get_check ("show_frame_rate");
    config->scale_to_fit              = get_check ("scale_to_fit");
    config->overscan                  = get_check ("overscan");
    config->maintain_aspect_ratio     = get_check ("maintain_aspect_ratio");
    config->aspect_ratio              = get_combo ("aspect_ratio");
    config->scale_method              = get_combo ("scale_method_combo");
    config->hires_effect              = get_combo ("hires_effect");
    config->force_inverted_byte_order = get_check ("force_inverted_byte_order");
    Settings.AutoSaveDelay            = get_entry_value ("save_sram_after_sec");
    config->multithreading            = get_check ("multithreading");
    config->pause_emulation_on_switch = get_check ("pause_emulation_on_switch");
    Settings.SkipFrames               = get_combo ("frameskip_combo");
    Settings.BlockInvalidVRAMAccessMaster   = get_check ("block_invalid_vram_access");
    Settings.UpAndDown                = get_check ("upanddown");
    config->sound_driver              = get_combo ("sound_driver");
    Settings.Stereo                   = get_check ("stereo_check");
    config->sound_playback_rate       = 7 - (get_combo ("playback_combo"));
    config->sound_buffer_size         = get_spin ("sound_buffer_size");
    config->sound_input_rate          = get_slider ("sound_input_rate");
    Settings.SoundSync                = get_check ("sync_sound");
    config->mute_sound                = get_check ("mute_sound_check");
    config->mute_sound_turbo          = get_check ("mute_sound_turbo_check");

    store_ntsc_settings ();
    config->ntsc_scanline_intensity   = get_combo ("ntsc_scanline_intensity");
    config->scanline_filter_intensity = get_combo ("scanline_filter_intensity");
    config->hw_accel                  = hw_accel_value (get_combo ("hw_accel"));
    config->bilinear_filter           = get_check ("bilinear_filter");
    config->num_threads               = get_spin ("num_threads");
    config->default_esc_behavior      = get_combo ("default_esc_behavior");
    config->prevent_screensaver       = get_check ("prevent_screensaver");
    config->rewind_buffer_size        = get_spin ("rewind_buffer_size");
    config->rewind_granularity        = get_spin ("rewind_granularity");

#ifdef USE_JOYSTICK
    config->joystick_threshold        = get_spin ("joystick_threshold");
#endif

#ifdef USE_OPENGL
    if (config->sync_to_vblank != get_check ("sync_to_vblank") ||
        config->npot_textures != get_check ("npot_textures") ||
        config->use_pbos != get_check ("use_pbos") ||
        config->pbo_format != get_combo ("pixel_format") ||
        config->use_shaders != get_check ("use_shaders") ||
        get_check ("use_shaders"))
    {
        gfx_needs_restart = 1;
    }

    config->sync_to_vblank            = get_check ("sync_to_vblank");
    config->use_pbos                  = get_check ("use_pbos");
    config->npot_textures             = get_check ("npot_textures");
    config->use_shaders               = get_check ("use_shaders");
    config->sync_every_frame          = get_check ("sync_every_frame");

    strncpy (config->fragment_shader, get_entry_text ("fragment_shader"), PATH_MAX);

    config->pbo_format = get_combo ("pixel_format");
#endif
    char safety_sram_directory [PATH_MAX];

    strncpy (safety_sram_directory, get_entry_text ("sram_directory"), PATH_MAX);
    strncpy (config->savestate_directory, get_entry_text ("savestate_directory"), PATH_MAX);
    strncpy (config->patch_directory, get_entry_text ("patch_directory"), PATH_MAX);
    strncpy (config->cheat_directory, get_entry_text ("cheat_directory"), PATH_MAX);
    strncpy (config->export_directory, get_entry_text ("export_directory"), PATH_MAX);

    if (!strcmp (safety_sram_directory, SAME_GAME))
        safety_sram_directory[0] = '\0';
    if (!strcmp (config->savestate_directory, SAME_GAME))
        config->savestate_directory[0] = '\0';
    if (!strcmp (config->patch_directory, SAME_GAME))
        config->patch_directory[0] = '\0';
    if (!strcmp (config->cheat_directory, SAME_GAME))
        config->cheat_directory[0] = '\0';
    if (!strcmp (config->export_directory, SAME_GAME))
        config->export_directory[0] = '\0';

    if (strcmp (safety_sram_directory, config->sram_directory) && config->rom_loaded)
    {
        GtkWidget *msg;
        int responseid;

        msg = gtk_message_dialog_new (GTK_WINDOW (this->window),
                                      GTK_DIALOG_DESTROY_WITH_PARENT,
                                      GTK_MESSAGE_WARNING,
                                      GTK_BUTTONS_OK_CANCEL,
                                      _("Changing the SRAM directory with a game loaded will replace the .srm file in the selected directory with the SRAM from the running game. If this is not what you want, click 'cancel'."));
        gtk_window_set_title (GTK_WINDOW (msg), _("Warning: Possible File Overwrite"));

        responseid = gtk_dialog_run (GTK_DIALOG (msg));

        if (responseid == GTK_RESPONSE_OK)
        {
            strncpy (config->sram_directory, safety_sram_directory, PATH_MAX);
        }
        else
        {
            if (config->sram_directory[0] == '\0')
                set_entry_text ("sram_directory", SAME_GAME);
            else
                set_entry_text ("sram_directory", config->sram_directory);
        }

        gtk_widget_destroy (msg);
    }
    else
    {
        strncpy (config->sram_directory, safety_sram_directory, PATH_MAX);
    }

    if (Settings.SkipFrames == 0)
        Settings.SkipFrames = AUTO_FRAMERATE;
    else
        Settings.SkipFrames--;

    memcpy (config->pad, pad, (sizeof (JoypadBinding)) * NUM_JOYPADS);
    memcpy (config->shortcut, shortcut, (sizeof (Binding)) * NUM_EMU_LINKS);

    if (sound_needs_restart)
    {
        S9xPortSoundReinit ();
    }

    if (gfx_needs_restart)
    {
        S9xReinitDisplay ();
    }

    S9xDisplayReconfigure ();
    S9xDisplayRefresh (top_level->last_width, top_level->last_height);

    S9xDeinitUpdate (top_level->last_width, top_level->last_height);

    top_level->configure_widgets ();

    if (config->default_esc_behavior != ESC_TOGGLE_MENUBAR)
        top_level->leave_fullscreen_mode ();

    return;
}

int
Snes9xPreferences::hw_accel_value (int combo_value)
{
    if (config->allow_opengl && config->allow_xv)
        return combo_value;
    else if (!config->allow_opengl && !config->allow_xv)
        return 0;
    else if (!config->allow_opengl && config->allow_xv)
        return combo_value ? 2 : 0;
    else
        return combo_value ? 1 : 0;
}

int
Snes9xPreferences::combo_value (int hw_accel)
{
    if (config->allow_opengl && config->allow_xv)
        return hw_accel;
    else if (!config->allow_opengl && !config->allow_xv)
        return 0;
    else if (!config->allow_opengl && config->allow_xv)
        return hw_accel == HWA_XV ? 1 : 0;
    else
        return hw_accel == HWA_OPENGL ? 1 : 0;
}


void
Snes9xPreferences::browse_folder_dialog (void)
{
    GtkWidget *dialog;
    char      *filename;
    gint      result;

    dialog = gtk_file_chooser_dialog_new (_("Select Folder"),
                                          GTK_WINDOW (this->window),
                                          GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                          NULL);

    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog),
                                         S9xGetDirectory (HOME_DIR));

    result = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_hide (dialog);

    if (result == GTK_RESPONSE_ACCEPT)
    {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

        gtk_entry_set_text (GTK_ENTRY (get_widget ("custom_folder_entry")),
                            filename);

        g_free (filename);
    }

    else
    {
    }

    gtk_widget_destroy (dialog);

    return;
}

void
Snes9xPreferences::show (void)
{
    gint      result;
    GtkWidget *combo;
    int       close_dialog;
#ifdef USE_JOYSTICK
    guint     source_id = -1;
#endif

    if (config->allow_xrandr)
    {
#ifdef USE_XRANDR
        char      size_string[256];

        combo = get_widget ("resolution_combo");

        config->xrr_index = 0;

        for (int i = 0; i < config->xrr_num_sizes; i++)
        {
            if (config->xrr_width == config->xrr_sizes[i].width &&
                config->xrr_height == config->xrr_sizes[i].height)
                config->xrr_index = i;

            snprintf (size_string,
                      256,
                      "%dx%d",
                      config->xrr_sizes[i].width,
                      config->xrr_sizes[i].height);

            combo_box_append (GTK_COMBO_BOX (combo), size_string);
        }
#endif
    }
    else
    {
        gtk_widget_hide (get_widget ("resolution_box"));
    }

#ifdef USE_HQ2X
    combo = get_widget ("scale_method_combo");
    combo_box_append (GTK_COMBO_BOX (combo), _("HQ2x"));
    combo_box_append (GTK_COMBO_BOX (combo), _("HQ3x"));
    combo_box_append (GTK_COMBO_BOX (combo), _("HQ4x"));
#endif

#ifdef USE_XBRZ
    combo = get_widget ("scale_method_combo");
    combo_box_append (GTK_COMBO_BOX (combo), _("2xBRZ"));
    combo_box_append (GTK_COMBO_BOX (combo), _("3xBRZ"));
    combo_box_append (GTK_COMBO_BOX (combo), _("4xBRZ"));
#endif

    combo = get_widget ("hw_accel");
    combo_box_append (GTK_COMBO_BOX (combo),
                      _("None - Use software scaler"));

    if (config->allow_opengl)
        combo_box_append (GTK_COMBO_BOX (combo),
                          _("OpenGL - Use 3D graphics hardware"));

    if (config->allow_xv)
        combo_box_append (GTK_COMBO_BOX (combo),
                          _("XVideo - Use hardware video blitter"));

    combo = get_widget ("sound_driver");

#ifdef USE_PORTAUDIO
    combo_box_append (GTK_COMBO_BOX (combo),
                      _("PortAudio"));
#endif
#ifdef USE_OSS
    combo_box_append (GTK_COMBO_BOX (combo),
                      _("Open Sound System"));
#endif
#ifdef USE_JOYSTICK
    combo_box_append (GTK_COMBO_BOX (combo),
                      _("SDL"));
#endif
#ifdef USE_ALSA
    combo_box_append (GTK_COMBO_BOX (combo),
                      _("ALSA"));
#endif
#ifdef USE_PULSEAUDIO
    combo_box_append (GTK_COMBO_BOX (combo),
                      _("PulseAudio"));
#endif

    move_settings_to_dialog ();

#ifdef USE_JOYSTICK
    S9xGrabJoysticks ();
    source_id = g_timeout_add (100, poll_joystick, (gpointer) this);
#endif

    if (config->preferences_width > 0 && config->preferences_height > 0)
        resize (config->preferences_width, config->preferences_height);

    for (close_dialog = 0; !close_dialog; )
    {
        gtk_widget_show (window);
        result = gtk_dialog_run (GTK_DIALOG (window));

        config->preferences_width = get_width ();
        config->preferences_height = get_height ();

        switch (result)
        {
            case GTK_RESPONSE_OK:
                get_settings_from_dialog ();
                config->save_config_file ();
                close_dialog = 1;
                gtk_widget_hide (window);
                break;

            case GTK_RESPONSE_APPLY:
                get_settings_from_dialog ();
                config->save_config_file ();
                break;

            case GTK_RESPONSE_CANCEL:
            case GTK_RESPONSE_CLOSE:
            case GTK_RESPONSE_DELETE_EVENT:
                gtk_widget_hide (window);
                close_dialog = 1;
                break;

            default:
                break;
        }
    }

#ifdef USE_JOYSTICK
    g_source_remove (source_id);
    S9xReleaseJoysticks ();
#endif

    gtk_widget_destroy (window);

    return;
}

void
Snes9xPreferences::focus_next (void)
{
    int next = get_focused_binding () + 1;

    for (int i = 0; b_breaks [i] != -1; i++)
    {
        if (b_breaks[i] == next)
            next = -1;
    }

    if (next > 0)
        gtk_widget_grab_focus (get_widget (b_links [next].button_name));
    else
        gtk_widget_grab_focus (get_widget ("cancel_button"));

    return;
}

void
Snes9xPreferences::swap_with (void)
{
    JoypadBinding mediator;
    int           source_joypad = get_combo ("control_combo");
    int           dest_joypad   = get_combo ("joypad_to_swap_with");

    memcpy (&mediator, &pad[source_joypad], sizeof (JoypadBinding));
    memcpy (&pad[source_joypad], &pad[dest_joypad], sizeof (JoypadBinding));
    memcpy (&pad[dest_joypad], &mediator, sizeof (JoypadBinding));

    bindings_to_dialog (source_joypad);

    return;
}

void
Snes9xPreferences::reset_current_joypad (void)
{
    int joypad = get_combo ("control_combo");

    memset (&pad[joypad], 0, sizeof (JoypadBinding));

    bindings_to_dialog (joypad);

    return;
}

void
Snes9xPreferences::store_binding (const char *string, Binding binding)
{
    int     current_joypad = get_combo ("control_combo");
    Binding *pad_bindings  = (Binding *) (&pad[current_joypad]);

    for (int i = 0; i < NUM_JOYPAD_LINKS; i++)
    {
        if (!strcmp (b_links[i].button_name, string))
        {
            pad_bindings[i] = binding;
        }
        else
        {
            if (pad_bindings[i].matches (binding))
            {
                pad_bindings[i].clear ();
            }
        }
    }

    for (int i = NUM_JOYPAD_LINKS; b_links[i].button_name; i++)
    {
        if (!strcmp (b_links[i].button_name, string))
        {
            shortcut[i - NUM_JOYPAD_LINKS] = binding;
        }
        else
        {
            if (shortcut[i - NUM_JOYPAD_LINKS].matches (binding))
            {
                shortcut[i - NUM_JOYPAD_LINKS].clear ();
            }
        }
    }

    focus_next ();

    bindings_to_dialog (get_combo ("control_combo"));

    return;
}

int
Snes9xPreferences::get_focused_binding (void)
{
    for (int i = 0; b_links[i].button_name; i++)
    {
        if (has_focus (b_links[i].button_name))
            return i;
    }

    return -1;
}

void
Snes9xPreferences::bindings_to_dialog (int joypad)
{
    char    name[256];
    Binding *bindings = (Binding *) &pad[joypad];

    set_combo ("control_combo", joypad);

    for (int i = 0; i < NUM_JOYPAD_LINKS; i++)
    {
        bindings[i].to_string (name);
        set_entry_text (b_links[i].button_name, name);
    }

    for (int i = NUM_JOYPAD_LINKS; b_links[i].button_name; i++)
    {
        shortcut[i - NUM_JOYPAD_LINKS].to_string (name);
        set_entry_text (b_links[i].button_name, name);
    }

    return;
}
