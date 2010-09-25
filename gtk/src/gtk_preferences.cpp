#include <string>
#include <stdlib.h>
#include <gdk/gdkkeysyms.h>

#include "gtk_preferences.h"
#include "gtk_config.h"
#include "gtk_s9xcore.h"
#include "gtk_control.h"
#include "gtk_sound.h"
#include "gtk_display.h"
#include "gtk_binding.h"

gboolean
snes9x_preferences_open (GtkWidget *widget,
                         gpointer  data)
{
    Snes9xPreferences *preferences;
    Snes9xWindow      *window = ((Snes9xWindow *) data);
    Snes9xConfig      *config = window->config;

    window->pause_from_focus_change ();

    preferences = new Snes9xPreferences (config);
    gtk_window_set_transient_for (preferences->get_window (),
                                  window->get_window ());

#ifdef USE_JOYSTICK
    config->set_joystick_mode (JOY_MODE_GLOBAL);
#endif

    preferences->show ();
    window->unpause_from_focus_change ();

#ifdef USE_JOYSTICK
    config->set_joystick_mode (JOY_MODE_INDIVIDUAL);
#endif

    config->reconfigure ();
    window->update_accels ();

    delete preferences;

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
event_style_set (GtkWidget *widget, GtkStyle *previous_style, gpointer data)
{
    ((Snes9xPreferences *) data)->fix_style ();

    return;
}

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
    name = gtk_widget_get_name (GTK_WIDGET (widget));
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
            gtk_widget_hide (window->get_widget ("opengl_frame"));
            gtk_widget_hide (window->get_widget ("xv_frame"));
            break;
        case HWA_OPENGL:
            gtk_widget_show (window->get_widget ("opengl_frame"));
            gtk_widget_hide (window->get_widget ("xv_frame"));
            break;
        case HWA_XV:
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

static void
event_switch_page (GtkNotebook     *notebook,
                   GtkNotebookPage *page,
                   guint           page_num,
                   gpointer        data)
{
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

    gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_widget_destroy (dialog);

    return;
}

#endif

Snes9xPreferences::Snes9xPreferences (Snes9xConfig *config) :
    GladeWindow (snes9x_glade, snes9x_glade_size, "preferences_window")
{
    GladeWindowCallbacks callbacks[] =
    {
        { "control_toggle", G_CALLBACK (event_control_toggle) },
        { "on_key_press", G_CALLBACK (event_key_press) },
        { "control_combo_changed", G_CALLBACK (event_control_combo_changed) },
        { "change_current_page", G_CALLBACK (event_switch_page) },
        { "sram_folder_browse", G_CALLBACK (event_sram_folder_browse) },
        { "scale_method_changed", G_CALLBACK (event_scale_method_changed) },
        { "hw_accel_changed", G_CALLBACK (event_hw_accel_changed) },
        { "reset_current_joypad", G_CALLBACK (event_reset_current_joypad) },
        { "swap_with", G_CALLBACK (event_swap_with) },
        { "style_set", G_CALLBACK (event_style_set) },
        { "ntsc_composite_preset", G_CALLBACK (event_ntsc_composite_preset) },
        { "ntsc_svideo_preset", G_CALLBACK (event_ntsc_svideo_preset) },
        { "ntsc_rgb_preset", G_CALLBACK (event_ntsc_rgb_preset) },
        { "ntsc_monochrome_preset", G_CALLBACK (event_ntsc_monochrome_preset) },
#ifdef USE_JOYSTICK
        { "calibrate", G_CALLBACK (event_calibrate) },
#endif
        { NULL, NULL }
    };

    std::string version_string;

    ((version_string += _("GTK port version: ")) += SNES9X_GTK_VERSION) += "\n";
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
    (version_string += _("</i>\n\nSnes9x version: ")) += VERSION;

    gtk_label_set_label (GTK_LABEL (get_widget ("version_string_label")),
                         version_string.c_str ());

    last_toggled = NULL;
    this->config = config;

    size_group[0] = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
    gtk_size_group_add_widget (size_group[0], get_widget ("resolution_combo"));
    gtk_size_group_add_widget (size_group[0], get_widget ("scale_method_combo"));
    size_group[1] = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
    gtk_size_group_add_widget (size_group[1], get_widget ("change_display_resolution"));
    gtk_size_group_add_widget (size_group[1], get_widget ("scale_method_label"));

    gtk_image_set_from_pixbuf (GTK_IMAGE (get_widget ("preferences_splash")),
                               top_level->splash);

    fix_style ();

    gtk_widget_realize (window);

    signal_connect (callbacks);

    return;
}

Snes9xPreferences::~Snes9xPreferences (void)
{
    g_object_unref (glade);
    g_object_unref (size_group[0]);
    g_object_unref (size_group[1]);

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
Snes9xPreferences::fix_style (void)
{
    GtkStyle *style = gtk_rc_get_style (get_widget ("preferences_notebook"));

    gtk_widget_set_style (get_widget ("display_viewport"), style);
    gtk_widget_set_style (get_widget ("sound_viewport"), style);
    gtk_widget_set_style (get_widget ("emulation_viewport"), style);
    gtk_widget_set_style (get_widget ("shortcut_viewport1"), style);
    gtk_widget_set_style (get_widget ("shortcut_viewport2"), style);
    gtk_widget_set_style (get_widget ("shortcut_viewport3"), style);
    gtk_widget_set_style (get_widget ("shortcut_viewport4"), style);

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
    set_check ("force_hires",               config->force_hires);
    set_check ("maintain_aspect_ratio",     config->maintain_aspect_ratio);
    set_combo ("aspect_ratio",              config->aspect_ratio);
    set_check ("rom_folder_radio",          config->data_location==DIR_ROM);
    set_check ("config_folder_radio",       config->data_location==DIR_CONFIG);
    set_check ("custom_folder_radio",       config->data_location==DIR_CUSTOM);
    set_entry_text ("custom_folder_entry",  config->custom_sram_directory);
    set_combo ("resolution_combo",          config->xrr_index);
    set_combo ("scale_method_combo",        config->scale_method);
    set_entry_value ("save_sram_after_sec", Settings.AutoSaveDelay);
    set_check ("block_invalid_vram_access", Settings.BlockInvalidVRAMAccessMaster);
    set_check ("upanddown",                 Settings.UpAndDown);
    set_combo ("default_esc_behavior",      config->default_esc_behavior);
    set_check ("prevent_screensaver",       config->prevent_screensaver);
    set_check ("force_inverted_byte_order", config->force_inverted_byte_order);
    set_check ("hdma_check",                !(Settings.DisableHDMA));
    set_check ("stereo_check",              Settings.Stereo);
    set_combo ("playback_combo",            7 - config->sound_playback_rate);
    set_combo ("hw_accel",                  combo_value (config->hw_accel));
    set_check ("pause_emulation_on_switch", config->pause_emulation_on_switch);
    set_spin  ("num_threads",               config->num_threads);
    set_check ("mute_sound_check",          config->mute_sound);
    set_spin  ("sound_buffer_size",         config->sound_buffer_size);
    set_slider ("sound_input_rate",         config->sound_input_rate);
    set_check ("sync_sound",                Settings.SoundSync);

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
#ifdef USE_OPENGL
    set_check ("bilinear_filter",           config->bilinear_filter);
    set_check ("sync_to_vblank",            config->sync_to_vblank);
    set_check ("use_pbos",                  config->use_pbos);
    set_combo ("pixel_format",              config->pbo_format);
    set_check ("npot_textures",             config->npot_textures);
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
    config->force_hires               = get_check ("force_hires");
    config->force_inverted_byte_order = get_check ("force_inverted_byte_order");
    Settings.AutoSaveDelay            = get_entry_value ("save_sram_after_sec");
    config->multithreading            = get_check ("multithreading");
    config->data_location             = get_check ("rom_folder_radio") ?
                                            DIR_ROM : config->data_location;
    config->data_location             = get_check ("config_folder_radio") ?
                                            DIR_CONFIG : config->data_location;
    config->data_location             = get_check ("custom_folder_radio") ?
                                            DIR_CUSTOM : config->data_location;
    config->pause_emulation_on_switch = get_check ("pause_emulation_on_switch");
    Settings.SkipFrames               = get_combo ("frameskip_combo");
    Settings.DisableHDMA              = !(get_check ("hdma_check"));
    Settings.BlockInvalidVRAMAccessMaster   = get_check ("block_invalid_vram_access");
    Settings.UpAndDown                = get_check ("upanddown");
    config->sound_driver              = get_combo ("sound_driver");
    Settings.Stereo                   = get_check ("stereo_check");
    config->sound_playback_rate       = 7 - (get_combo ("playback_combo"));
    config->sound_buffer_size         = get_spin ("sound_buffer_size");
    config->sound_input_rate          = get_slider ("sound_input_rate");
    Settings.SoundSync                = get_check ("sync_sound");
    config->mute_sound                = get_check ("mute_sound_check");

    store_ntsc_settings ();
    config->ntsc_scanline_intensity   = get_combo ("ntsc_scanline_intensity");
    config->scanline_filter_intensity = get_combo ("scanline_filter_intensity");
    config->hw_accel                  = hw_accel_value (get_combo ("hw_accel"));
    config->num_threads               = get_spin ("num_threads");
    config->default_esc_behavior      = get_combo ("default_esc_behavior");
    config->prevent_screensaver       = get_check ("prevent_screensaver");

#ifdef USE_JOYSTICK
    config->joystick_threshold        = get_spin ("joystick_threshold");
#endif

#ifdef USE_OPENGL
    if (config->sync_to_vblank != get_check ("sync_to_vblank") ||
        config->npot_textures != get_check ("npot_textures") ||
        config->use_pbos != get_check ("use_pbos") ||
        config->pbo_format != get_combo ("pixel_format"))
    {
        gfx_needs_restart = 1;
    }

    config->bilinear_filter           = get_check ("bilinear_filter");
    config->sync_to_vblank            = get_check ("sync_to_vblank");
    config->use_pbos                  = get_check ("use_pbos");
    config->npot_textures             = get_check ("npot_textures");

    config->pbo_format = get_combo ("pixel_format");

#endif

    strncpy (config->custom_sram_directory,
             get_entry_text ("custom_folder_entry"),
             PATH_MAX);

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
    else
    {
        S9xDisplayReconfigure ();
        S9xDisplayRefresh (top_level->last_width, top_level->last_height);

        if (config->rom_loaded)
            S9xDeinitUpdate (top_level->last_width, top_level->last_height);
    }

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
    gtk_widget_hide_all (dialog);

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

            gtk_combo_box_append_text (GTK_COMBO_BOX (combo), size_string);
        }
#endif
    }
    else
    {
        gtk_widget_hide (get_widget ("resolution_box"));
    }

    combo = get_widget ("hw_accel");
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo),
                               _("None - Use software scaler"));

    if (config->allow_opengl)
        gtk_combo_box_append_text (GTK_COMBO_BOX (combo),
                                   _("OpenGL - Use 3D graphics hardware"));

    if (config->allow_xv)
        gtk_combo_box_append_text (GTK_COMBO_BOX (combo),
                                   _("XVideo - Use hardware video blitter"));

    combo = get_widget ("sound_driver");

#ifdef USE_PORTAUDIO
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo),
                               _("PortAudio"));
#endif
#ifdef USE_OSS
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo),
                               _("Open Sound System"));
#endif
#ifdef USE_JOYSTICK
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo),
                               _("SDL"));
#endif
#ifdef USE_ALSA
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo),
                               _("ALSA"));
#endif
#ifdef USE_PULSEAUDIO
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo),
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

            default:
                gtk_widget_hide (window);
                close_dialog = 1;
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
