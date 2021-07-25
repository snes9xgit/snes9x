/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <string>
#include <stdlib.h>
#include "gtk_compat.h"
#include "gtk_preferences.h"
#include "gtk_config.h"
#include "gtk_control.h"
#include "gtk_sound.h"
#include "gtk_display.h"
#include "gtk_binding.h"

#include "snes9x.h"
#include "gfx.h"
#include "display.h"

#define SAME_AS_GAME _("Same location as current game")

static Snes9xPreferences *preferences = nullptr;

void snes9x_preferences_open(Snes9xWindow *window)
{
    if (!preferences)
        preferences = new Snes9xPreferences(window->config);

    auto &config = preferences->config;

    window->pause_from_focus_change ();

    preferences->window->set_transient_for(*window->window.get());

    config->set_joystick_mode(JOY_MODE_GLOBAL);
    preferences->show();
    window->unpause_from_focus_change();

    config->set_joystick_mode(JOY_MODE_INDIVIDUAL);

    config->rebind_keys();
    window->update_accelerators();
}

gboolean poll_joystick(gpointer data)
{
    Snes9xPreferences *window = (Snes9xPreferences *)data;
    JoyEvent event;
    Binding binding;
    int focus;

    for (size_t i = 0; i < window->config->joystick.size(); i++)
    {
        while (window->config->joystick[i].get_event(&event))
        {
            if (event.state == JOY_PRESSED)
            {
                if ((focus = window->get_focused_binding()) >= 0)
                {
                    binding = Binding(i,
                                      event.parameter,
                                      window->config->joystick_threshold);

                    window->store_binding(b_links[focus].button_name,
                                          binding);

                    window->config->flush_joysticks();
                    return true;
                }
            }
        }
    }

    return true;
}

Snes9xPreferences::Snes9xPreferences(Snes9xConfig *config)
    : GtkBuilderWindow("preferences_window")
{
    this->config = config;

    gtk_widget_realize(GTK_WIDGET(window->gobj()));
    connect_signals();

    for (int i = 0; b_links[i].button_name; i++)
    {
        const BindingLink &link = b_links[i];
        auto entry = get_object<Gtk::Entry>(link.button_name);
        entry->set_icon_from_icon_name("edit-clear", Gtk::ENTRY_ICON_SECONDARY);
        entry->set_icon_activatable(true, Gtk::ENTRY_ICON_SECONDARY);
        entry->signal_icon_release().connect([i, this](Gtk::EntryIconPosition pos, const GdkEventButton *) {
            clear_binding(b_links[i].button_name);
        });
    }

    #ifdef GDK_WINDOWING_X11
    if (config->allow_xrandr)
    {
        char size_string[256];

        for (int i = 0; i < config->xrr_screen_resources->nmode; i++)
        {
            XRRModeInfo *m = &config->xrr_screen_resources->modes[i];
            unsigned long dotClock = m->dotClock;
            if (m->modeFlags & RR_ClockDivideBy2)
                dotClock /= 2;
            if (m->modeFlags & RR_DoubleScan)
                dotClock /= 2;
            if (m->modeFlags & RR_DoubleClock)
                dotClock *= 2;

            snprintf(size_string,
                     256,
                     "%dx%d @ %.3fHz",
                     m->width,
                     m->height,
                     (double)dotClock / m->hTotal / m->vTotal);

            combo_box_append("resolution_combo", size_string);
        }

        if (config->xrr_index > config->xrr_screen_resources->nmode)
            config->xrr_index = 0;
    }
    else
#endif
    {
        show_widget("resolution_box", false);
    }

#ifdef USE_HQ2X
    combo_box_append("scale_method_combo", _("HQ2x"));
    combo_box_append("scale_method_combo", _("HQ3x"));
    combo_box_append("scale_method_combo", _("HQ4x"));
#endif

#ifdef USE_XBRZ
    combo_box_append("scale_method_combo", _("2xBRZ"));
    combo_box_append("scale_method_combo", _("3xBRZ"));
    combo_box_append("scale_method_combo", _("4xBRZ"));
#endif

    combo_box_append("hw_accel", _("None - Use software scaler"));

    if (config->allow_opengl)
        combo_box_append("hw_accel",
                         _("OpenGL - Use 3D graphics hardware"));

    if (config->allow_xv)
        combo_box_append("hw_accel",
                         _("XVideo - Use hardware video blitter"));

    for (auto &name : config->sound_drivers)
    {
        combo_box_append("sound_driver", name.c_str());
    }
}

Snes9xPreferences::~Snes9xPreferences ()
{
}

void Snes9xPreferences::connect_signals()
{
    window->signal_key_press_event().connect(sigc::mem_fun(*this, &Snes9xPreferences::key_pressed), false);

    get_object<Gtk::ComboBox>("control_combo")->signal_changed().connect([&] {
        bindings_to_dialog(get_object<Gtk::ComboBox>("control_combo")->get_active_row_number());
    });
    get_object<Gtk::ComboBox>("scale_method_combo")->signal_changed().connect([&] {
        int id = get_combo("scale_method_combo");
        show_widget("ntsc_alignment", id == FILTER_NTSC);
        show_widget("scanline_alignment", id == FILTER_SCANLINES);
    });

    get_object<Gtk::ComboBox>("hw_accel")->signal_changed().connect([&] {
        int id = get_combo("hw_accel");
        show_widget("bilinear_filter", id != HWA_XV);
        show_widget("opengl_frame", id == HWA_OPENGL);
        show_widget("xv_frame", id == HWA_XV);
    });

    get_object<Gtk::Button>("reset_current_joypad")->signal_pressed().connect(sigc::mem_fun(*this, &Snes9xPreferences::reset_current_joypad));
    get_object<Gtk::Button>("swap_with")->signal_pressed().connect(sigc::mem_fun(*this, &Snes9xPreferences::swap_with));
    get_object<Gtk::Button>("ntsc_composite_preset")->signal_pressed().connect([&] {
        config->ntsc_setup = snes_ntsc_composite;
        load_ntsc_settings();
    });
    get_object<Gtk::Button>("ntsc_svideo_preset")->signal_pressed().connect([&] {
        config->ntsc_setup = snes_ntsc_svideo;
        load_ntsc_settings();
    });
    get_object<Gtk::Button>("ntsc_rgb_preset")->signal_pressed().connect([&] {
        config->ntsc_setup = snes_ntsc_rgb;
        load_ntsc_settings();
    });

    get_object<Gtk::Button>("ntsc_monochrome_preset")->signal_pressed().connect([&] {
        config->ntsc_setup = snes_ntsc_monochrome;
        load_ntsc_settings();
    });

    get_object<Gtk::Button>("fragment_shader_button")->signal_pressed().connect(sigc::mem_fun(*this, &Snes9xPreferences::shader_select));
    get_object<Gtk::Button>("calibrate_button")->signal_pressed().connect(sigc::mem_fun(*this, &Snes9xPreferences::calibration_dialog));
    get_object<Gtk::HScale>("sound_input_rate")->signal_value_changed().connect(sigc::mem_fun(*this, &Snes9xPreferences::input_rate_changed));
    get_object<Gtk::Button>("about_button")->signal_clicked().connect(sigc::mem_fun(*this, &Snes9xPreferences::about_dialog));
    get_object<Gtk::ToggleButton>("auto_input_rate")->signal_toggled().connect([&] {
        auto toggle_button = get_object<Gtk::ToggleButton>("auto_input_rate");
        enable_widget("sound_input_rate", toggle_button->get_active());
        if (toggle_button->get_active())
            set_slider("sound_input_rate", top_level->get_auto_input_rate());
    });

    std::array<std::string, 5> browse_buttons = { "sram", "savestate", "cheat", "patch", "export" };
    for (auto &name : browse_buttons)
    {
        get_object<Gtk::Button>((name + "_browse").c_str())->signal_clicked().connect([&, name] {
            game_data_browse(name);
        });

        auto entry = get_object<Gtk::Entry>((name + "_directory").c_str());
        entry->set_icon_activatable(true, Gtk::ENTRY_ICON_SECONDARY);
        entry->signal_icon_release().connect([&, name](Gtk::EntryIconPosition pos, const GdkEventButton *) {
            get_object<Gtk::Entry>((name + "_directory").c_str())->set_text(SAME_AS_GAME);
        });
    }
}

void Snes9xPreferences::about_dialog()
{
    std::string version_string;
    GtkBuilderWindow about_dialog("about_dialog");

    ((version_string += _("Snes9x version: ")) += VERSION) += ", ";
    ((version_string += _("GTK+ port version: ")) += SNES9X_GTK_VERSION) += "\n";
    (version_string += SNES9X_GTK_AUTHORS) += "\n";
    (version_string += _("English localization by Brandon Wright")) += "\n";

    auto label = get_object<Gtk::Label>("version_string_label");
    label->set_label(version_string);
    label->set_justify(Gtk::JUSTIFY_LEFT);
    about_dialog.show_widget("preferences_splash", false);

    auto provider = Gtk::CssProvider::create();
    provider->load_from_data("textview {"
                             " font-family: \"monospace\";"
                             " font-size: 8pt;"
                             "}");
    get_object<Gtk::TextView>("about_text_view")->reset_style();
    get_object<Gtk::TextView>("about_text_view")->get_style_context()->add_provider(provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    about_dialog.window->set_transient_for(*window.get());
    about_dialog.window->set_modal();
    Glib::RefPtr<Gtk::Dialog>::cast_static(about_dialog.window)->run();
    about_dialog.window->hide();
}

void Snes9xPreferences::game_data_browse(std::string folder)
{
    auto entry = get_object<Gtk::Entry>((folder + "_directory").c_str());
    auto dialog = Gtk::FileChooserDialog(*window.get(), _("Select directory"), Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    dialog.add_button(Gtk::StockID("gtk-cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::StockID("gtk-open"), Gtk::RESPONSE_ACCEPT);


    if (!config->last_directory.empty())
        dialog.set_current_folder(config->last_directory);

    if (entry->get_text().compare(SAME_AS_GAME) != 0)
        dialog.set_filename(entry->get_text());

    auto result = dialog.run();
    dialog.hide();

    if (result == Gtk::RESPONSE_ACCEPT)
        entry->set_text(dialog.get_filename());
}

void Snes9xPreferences::input_rate_changed()
{
    double value = get_object<Gtk::HScale>("sound_input_rate")->get_value();
    value = value / 32040.0 * 60.09881389744051;
    char text[256];
    snprintf(text, 256, "%.4f Hz", value);
    get_object<Gtk::Label>("relative_video_rate")->set_label(text);
}

bool Snes9xPreferences::key_pressed(GdkEventKey *event)
{
    int focus = get_focused_binding();

    if (focus < 0)
        return false; // Pass event on to Gtk

    // Disallow modifier keys if page isn't set to the joypad bindings.
    if (get_object<Gtk::Notebook>("preferences_notebook")->get_current_page() != 4 ||
        !get_object<Gtk::ToggleButton>("use_modifiers")->get_active())
    {
        // Don't allow modifiers that we track to be bound
        if (event->keyval == GDK_Control_L ||
            event->keyval == GDK_Control_R ||
            event->keyval == GDK_Shift_L ||
            event->keyval == GDK_Shift_R ||
            event->keyval == GDK_Alt_L ||
            event->keyval == GDK_Alt_R)
        {
            return false;
        }
    }

    Binding key_binding = Binding(event);

    // Allows ESC key to clear the key binding
    if (event->keyval == GDK_Escape)
    {
        if (event->state & GDK_SHIFT_MASK)
        {
            key_binding.clear();
        }
        else
        {
            focus_next();
            return true;
        }
    }

    store_binding(b_links[focus].button_name, key_binding);

    return true;
}

void Snes9xPreferences::shader_select()
{
#ifdef USE_OPENGL
    auto entry = get_object<Gtk::Entry>("fragment_shader");

    auto dialog = Gtk::FileChooserDialog(*window.get(), _("Select Shader File"));
    dialog.add_button(Gtk::StockID("gtk-cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::StockID("gtk-open"), Gtk::RESPONSE_ACCEPT);
    if (!config->last_shader_directory.empty())
        dialog.set_current_folder(config->last_shader_directory);
    else
        dialog.set_filename(entry->get_text());

    auto result = dialog.run();
    dialog.hide();

    if (result == Gtk::RESPONSE_ACCEPT)
    {
        auto folder = dialog.get_current_folder();
        auto filename = dialog.get_filename();

        if (!folder.empty())
            config->last_shader_directory = folder;
        if (!filename.empty())
            entry->set_text(filename);
    }
#endif
}

void Snes9xPreferences::load_ntsc_settings()
{
    set_slider("ntsc_artifacts", config->ntsc_setup.artifacts);
    set_slider("ntsc_bleed", config->ntsc_setup.bleed);
    set_slider("ntsc_brightness", config->ntsc_setup.brightness);
    set_slider("ntsc_contrast", config->ntsc_setup.contrast);
    set_slider("ntsc_fringing", config->ntsc_setup.fringing);
    set_slider("ntsc_gamma", config->ntsc_setup.gamma);
    set_slider("ntsc_hue", config->ntsc_setup.hue);
    set_check("ntsc_merge_fields", config->ntsc_setup.merge_fields);
    set_slider("ntsc_resolution", config->ntsc_setup.resolution);
    set_slider("ntsc_saturation", config->ntsc_setup.saturation);
    set_slider("ntsc_sharpness", config->ntsc_setup.sharpness);
}

void Snes9xPreferences::store_ntsc_settings()
{
    config->ntsc_setup.artifacts = get_slider("ntsc_artifacts");
    config->ntsc_setup.bleed = get_slider("ntsc_bleed");
    config->ntsc_setup.brightness = get_slider("ntsc_brightness");
    config->ntsc_setup.contrast = get_slider("ntsc_contrast");
    config->ntsc_setup.fringing = get_slider("ntsc_fringing");
    config->ntsc_setup.gamma = get_slider("ntsc_gamma");
    config->ntsc_setup.hue = get_slider("ntsc_hue");
    config->ntsc_setup.merge_fields = get_check("ntsc_merge_fields");
    config->ntsc_setup.resolution = get_slider("ntsc_resolution");
    config->ntsc_setup.saturation = get_slider("ntsc_saturation");
    config->ntsc_setup.sharpness = get_slider("ntsc_sharpness");
}

void Snes9xPreferences::move_settings_to_dialog()
{
    set_check("full_screen_on_open",       config->full_screen_on_open);
    set_check("show_time",                 Settings.DisplayTime);
    set_check("show_frame_rate",           Settings.DisplayFrameRate);
    set_check("show_pressed_keys",         Settings.DisplayPressedKeys);
    set_check("change_display_resolution", config->change_display_resolution);
    set_check("scale_to_fit",              config->scale_to_fit);
    set_check("overscan",                  config->overscan);
    set_check("multithreading",            config->multithreading);
    set_combo("hires_effect",              config->hires_effect);
    set_check("maintain_aspect_ratio",     config->maintain_aspect_ratio);
    set_combo("aspect_ratio",              config->aspect_ratio);
    if (config->sram_directory.empty())
        set_entry_text("sram_directory", SAME_AS_GAME);
    else
        set_entry_text("sram_directory", config->sram_directory.c_str());
    if (config->savestate_directory.empty())
        set_entry_text("savestate_directory", SAME_AS_GAME);
    else
        set_entry_text("savestate_directory", config->savestate_directory.c_str());
    if (config->patch_directory.empty())
        set_entry_text("patch_directory", SAME_AS_GAME);
    else
        set_entry_text("patch_directory", config->patch_directory.c_str());
    if (config->cheat_directory.empty())
        set_entry_text("cheat_directory", SAME_AS_GAME);
    else
        set_entry_text("cheat_directory", config->cheat_directory.c_str());
    if (config->export_directory.empty())
        set_entry_text("export_directory", SAME_AS_GAME);
    else
        set_entry_text("export_directory", config->export_directory.c_str());

    set_combo("resolution_combo",          config->xrr_index);
    set_combo("scale_method_combo",        config->scale_method);
    set_entry_value("save_sram_after_sec", Settings.AutoSaveDelay);
    set_check("allow_invalid_vram_access", !Settings.BlockInvalidVRAMAccessMaster);
    set_check("upanddown",                 Settings.UpAndDown);
    set_combo("default_esc_behavior",      config->default_esc_behavior);
    set_check("prevent_screensaver",       config->prevent_screensaver);
    set_check("force_inverted_byte_order", config->force_inverted_byte_order);
    set_combo("playback_combo",            7 - config->sound_playback_rate);
    set_combo("hw_accel",                  combo_value (config->hw_accel));
    set_check("pause_emulation_on_switch", config->pause_emulation_on_switch);
    set_spin ("num_threads",               config->num_threads);
    set_check("mute_sound_check",          config->mute_sound);
    set_check("mute_sound_turbo_check",    config->mute_sound_turbo);
    set_slider("sound_input_rate",         config->sound_input_rate);
    if (top_level->get_auto_input_rate() == 0)
    {
        config->auto_input_rate = 0;
        enable_widget("auto_input_rate", false);
    }
    set_check ("auto_input_rate",           config->auto_input_rate);
    enable_widget("sound_input_rate",       config->auto_input_rate ? false : true);
    set_spin  ("sound_buffer_size",         config->sound_buffer_size);
    set_check ("dynamic_rate_control",      Settings.DynamicRateControl);
    set_spin  ("dynamic_rate_limit",        Settings.DynamicRateLimit / 1000.0);
    set_spin  ("rewind_buffer_size",        config->rewind_buffer_size);
    set_spin  ("rewind_granularity",        config->rewind_granularity);
    set_spin  ("superfx_multiplier",        Settings.SuperFXClockMultiplier);
    set_combo ("splash_background",         config->splash_image);
    set_check ("force_enable_icons",        config->enable_icons);

    set_combo ("sound_driver",              config->sound_driver);

    show_widget("ntsc_alignment", config->scale_method == FILTER_NTSC);
    show_widget("scanline_alignment", config->scale_method == FILTER_SCANLINES);

    load_ntsc_settings();
    set_combo ("ntsc_scanline_intensity",   config->ntsc_scanline_intensity);
    set_combo ("scanline_filter_intensity", config->scanline_filter_intensity);

    set_combo ("frameskip_combo",           Settings.SkipFrames);
    set_check ("bilinear_filter",           Settings.BilinearFilter);

#ifdef USE_OPENGL
    set_check ("sync_to_vblank",            config->sync_to_vblank);
    set_check ("use_glfinish",              config->use_glfinish);
    set_check ("use_sync_control",          config->use_sync_control);
    set_check ("use_pbos",                  config->use_pbos);
    set_combo ("pixel_format",              config->pbo_format == 16 ? 0 : 1);
    set_check ("npot_textures",             config->npot_textures);
    set_check ("use_shaders",               config->use_shaders);
    set_entry_text ("fragment_shader",      config->shader_filename.c_str ());
#endif
    set_spin ("joystick_threshold",         config->joystick_threshold);

    /* Control bindings */
    pad = config->pad;
    shortcut = config->shortcut;
    bindings_to_dialog(0);

    set_combo("joypad_to_swap_with", 0);

#ifdef ALLOW_CPU_OVERCLOCK
    show_widget("cpu_overclock", true);
    show_widget("remove_sprite_limit", true);
    show_widget("allow_invalid_vram_access", true);
    show_widget("echo_buffer_hack", true);
    show_widget("soundfilterhbox", true);

    set_check("cpu_overclock", Settings.OneClockCycle != 6);
    set_check("remove_sprite_limit", Settings.MaxSpriteTilesPerLine != 34);
    set_check("echo_buffer_hack", Settings.SeparateEchoBuffer);
    set_combo("sound_filter", Settings.InterpolationMethod);
#endif
}

void Snes9xPreferences::get_settings_from_dialog()
{
    bool sound_needs_restart = false;
    bool gfx_needs_restart = false;
    bool sound_sync = false;

    Settings.SkipFrames               = get_combo("frameskip_combo");
    if (Settings.SkipFrames == THROTTLE_SOUND_SYNC)
        sound_sync = true;

    if ((config->sound_driver        != get_combo("sound_driver"))            ||
        (config->mute_sound          != get_check("mute_sound_check"))        ||
        (config->sound_buffer_size   != (int) get_spin("sound_buffer_size"))  ||
        (config->sound_playback_rate != (7 - (get_combo("playback_combo"))))  ||
        (config->sound_input_rate    != get_slider("sound_input_rate"))       ||
        (config->auto_input_rate     != get_check("auto_input_rate"))         ||
        (Settings.SoundSync          != sound_sync)                            ||
        (Settings.DynamicRateControl != get_check("dynamic_rate_control")))
    {
        sound_needs_restart = true;
    }

    if ((config->change_display_resolution != get_check("change_display_resolution") ||
            (config->change_display_resolution &&
                    (config->xrr_index != get_combo("resolution_combo")))) &&
        config->fullscreen)
    {
        top_level->leave_fullscreen_mode ();
        config->xrr_index = get_combo("resolution_combo");
        config->change_display_resolution = get_check("change_display_resolution");
        top_level->enter_fullscreen_mode ();
    }
    else
    {
        config->xrr_index = get_combo("resolution_combo");
    }

    config->change_display_resolution = get_check("change_display_resolution");

    if (config->splash_image != get_combo("splash_background"))
    {
        config->splash_image = get_combo("splash_background");
        if (!config->rom_loaded)
        {
            top_level->last_width = top_level->last_height = -1;
            top_level->refresh();
        }
    }

    config->splash_image = get_combo("splash_background");

    if (config->multithreading != get_check("multithreading"))
        gfx_needs_restart = true;

    if (config->hw_accel != hw_accel_value (get_combo("hw_accel")))
        gfx_needs_restart = true;

    if (config->force_inverted_byte_order != get_check("force_inverted_byte_order"))
        gfx_needs_restart = true;

    config->enable_icons = get_check("force_enable_icons");
    auto settings = Gtk::Settings::get_default();
    settings->set_property("gtk-menu-images", gui_config->enable_icons);
    settings->set_property("gtk-button-images", gui_config->enable_icons);

    config->full_screen_on_open       = get_check("full_screen_on_open");
    Settings.DisplayTime              = get_check("show_time");
    Settings.DisplayFrameRate         = get_check("show_frame_rate");
    Settings.DisplayPressedKeys       = get_check("show_pressed_keys");
    config->scale_to_fit              = get_check("scale_to_fit");
    config->overscan                  = get_check("overscan");
    config->maintain_aspect_ratio     = get_check("maintain_aspect_ratio");
    config->aspect_ratio              = get_combo("aspect_ratio");
    config->scale_method              = get_combo("scale_method_combo");
    config->hires_effect              = get_combo("hires_effect");
    config->force_inverted_byte_order = get_check("force_inverted_byte_order");
    Settings.AutoSaveDelay            = get_entry_value("save_sram_after_sec");
    config->multithreading            = get_check("multithreading");
    config->pause_emulation_on_switch = get_check("pause_emulation_on_switch");
    Settings.BlockInvalidVRAMAccessMaster   = !get_check("allow_invalid_vram_access");
    Settings.UpAndDown                = get_check("upanddown");
    Settings.SuperFXClockMultiplier   = get_spin("superfx_multiplier");
    config->sound_driver              = get_combo("sound_driver");
    config->sound_playback_rate       = 7 - (get_combo("playback_combo"));
    config->sound_buffer_size         = get_spin("sound_buffer_size");
    config->sound_input_rate          = get_slider("sound_input_rate");
    config->auto_input_rate           = get_check("auto_input_rate");
    Settings.SoundSync                = sound_sync;
    config->mute_sound                = get_check("mute_sound_check");
    config->mute_sound_turbo          = get_check("mute_sound_turbo_check");
    Settings.DynamicRateControl       = get_check("dynamic_rate_control");
    Settings.DynamicRateLimit         = (uint32) (get_spin("dynamic_rate_limit") * 1000);
    store_ntsc_settings();
    config->ntsc_scanline_intensity   = get_combo("ntsc_scanline_intensity");
    config->scanline_filter_intensity = get_combo("scanline_filter_intensity");
    config->hw_accel                  = hw_accel_value(get_combo("hw_accel"));
    Settings.BilinearFilter           = get_check("bilinear_filter");
    config->num_threads               = get_spin("num_threads");
    config->default_esc_behavior      = get_combo("default_esc_behavior");
    config->prevent_screensaver       = get_check("prevent_screensaver");
    config->rewind_buffer_size        = get_spin("rewind_buffer_size");
    config->rewind_granularity        = get_spin("rewind_granularity");
    config->joystick_threshold        = get_spin("joystick_threshold");

#ifdef ALLOW_CPU_OVERCLOCK
    if (get_check("cpu_overclock"))
    {
        Settings.OneClockCycle = 4;
        Settings.OneSlowClockCycle = 5;
        Settings.TwoClockCycles = 6;
    }
    else
    {
        Settings.OneClockCycle = 6;
        Settings.OneSlowClockCycle = 8;
        Settings.TwoClockCycles = 12;
    }

    if (get_check("remove_sprite_limit"))
    {
        Settings.MaxSpriteTilesPerLine = 128;
    }
    else
    {
        Settings.MaxSpriteTilesPerLine = 34;
    }

    Settings.SeparateEchoBuffer = get_check("echo_buffer_hack");
    Settings.InterpolationMethod = get_combo("sound_filter");
#endif

#ifdef USE_OPENGL
    int pbo_format = get_combo("pixel_format") == 1 ? 32 : 16;

    if (config->sync_to_vblank   != get_check("sync_to_vblank") ||
        config->use_sync_control != get_check("use_sync_control") ||
        config->npot_textures    != get_check("npot_textures") ||
        config->use_pbos         != get_check("use_pbos") ||
        config->pbo_format       !=  pbo_format ||
        config->use_shaders      != get_check("use_shaders") ||
        (config->shader_filename.compare(get_entry_text("fragment_shader"))))
    {
        gfx_needs_restart = true;
    }

    config->sync_to_vblank   = get_check("sync_to_vblank");
    config->use_pbos         = get_check("use_pbos");
    config->npot_textures    = get_check("npot_textures");
    config->use_shaders      = get_check("use_shaders");
    config->use_glfinish     = get_check("use_glfinish");
    config->use_sync_control = get_check("use_sync_control");
    config->shader_filename  = get_entry_text ("fragment_shader");
    config->pbo_format       = pbo_format;
#endif

    std::string new_sram_directory = get_entry_text("sram_directory");
    config->savestate_directory = get_entry_text("savestate_directory");
    config->patch_directory = get_entry_text("patch_directory");
    config->cheat_directory = get_entry_text("cheat_directory");
    config->export_directory = get_entry_text("export_directory");

    for (auto &i : { &new_sram_directory,
                     &config->savestate_directory,
                     &config->patch_directory,
                     &config->cheat_directory,
                     &config->export_directory })
    {
        if (!i->compare(SAME_AS_GAME))
            i->clear();
    }

    if (new_sram_directory.compare(config->sram_directory) && config->rom_loaded)
    {
        auto msg = Gtk::MessageDialog(
            *window.get(),
            _("Changing the SRAM directory with a game loaded will replace the .srm file in the selected directory with the SRAM from the running game. If this is not what you want, click 'cancel'."),
            false,
            Gtk::MESSAGE_WARNING,
            Gtk::BUTTONS_OK_CANCEL,
            true);
        msg.set_title(_("Warning: Possible File Overwrite"));

        auto result = msg.run();
        msg.hide();

        if (result == Gtk::RESPONSE_OK)
        {
            config->sram_directory = new_sram_directory;
        }
        else
        {
            if (config->sram_directory.empty ())
                set_entry_text("sram_directory", SAME_AS_GAME);
            else
                set_entry_text("sram_directory", config->sram_directory.c_str ());
        }
    }
    else
    {
        config->sram_directory = new_sram_directory;
    }

    config->pad = pad;
    config->shortcut = shortcut;

    if (sound_needs_restart)
    {
        S9xPortSoundReinit();
    }

    if (gfx_needs_restart)
    {
        S9xReinitDisplay();
    }

    S9xDisplayReconfigure();
    S9xDisplayRefresh(top_level->last_width, top_level->last_height);

    S9xDeinitUpdate(top_level->last_width, top_level->last_height);

    top_level->configure_widgets();

    if (config->default_esc_behavior != ESC_TOGGLE_MENUBAR)
        top_level->leave_fullscreen_mode();
}

int Snes9xPreferences::hw_accel_value(int combo_value)
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

int Snes9xPreferences::combo_value(int hw_accel)
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

void Snes9xPreferences::show()
{
    bool close_dialog;
    guint source_id = -1;

    move_settings_to_dialog();

    S9xGrabJoysticks();
    source_id = g_timeout_add(100, poll_joystick, (gpointer)this);

    if (config->preferences_width > 0 && config->preferences_height > 0)
        resize (config->preferences_width, config->preferences_height);

    get_object<Gtk::Notebook>("display_notebook")->set_current_page(config->current_display_tab);

    auto dialog = Glib::RefPtr<Gtk::Dialog>::cast_static(window);

    for (close_dialog = false; !close_dialog; )
    {
        dialog->show();
        auto result = dialog->run();

        config->preferences_width = get_width();
        config->preferences_height = get_height();
        config->current_display_tab = get_object<Gtk::Notebook>("display_notebook")->get_current_page();

        switch (result)
        {
            case GTK_RESPONSE_OK:
                get_settings_from_dialog();
                config->save_config_file();
                close_dialog = true;
                dialog->hide();
                break;

            case GTK_RESPONSE_APPLY:
                get_settings_from_dialog();
                config->save_config_file();
                break;

            case GTK_RESPONSE_CANCEL:
            case GTK_RESPONSE_CLOSE:
            case GTK_RESPONSE_DELETE_EVENT:
                dialog->hide();
                close_dialog = true;
                break;

            default:
                break;
        }
    }

    g_source_remove(source_id);
    S9xReleaseJoysticks();
}

void Snes9xPreferences::focus_next()
{
    int next = get_focused_binding() + 1;

    for (int i = 0; b_breaks[i] != -1; i++)
    {
        if (b_breaks[i] == next)
            next = -1;
    }

    if (next > 0)
        get_object<Gtk::Entry>(b_links[next].button_name)->grab_focus();
    else
        get_object<Gtk::Button>("cancel_button")->grab_focus();
}

void Snes9xPreferences::swap_with()
{
    JoypadBinding tmp;
    int source_joypad = get_combo("control_combo");
    int dest_joypad = get_combo("joypad_to_swap_with");

    tmp = pad[source_joypad];
    pad[source_joypad] = pad[dest_joypad];
    pad[dest_joypad] = tmp;

    bindings_to_dialog(source_joypad);
}

void Snes9xPreferences::reset_current_joypad()
{
    int joypad = get_combo("control_combo");

    for (unsigned int i = 0; i < NUM_JOYPAD_LINKS; i++)
    {
        pad[joypad].data[i].clear();
    }

    bindings_to_dialog(joypad);
}

void Snes9xPreferences::store_binding(const char *string, Binding binding)
{
    int current_joypad = get_combo("control_combo");
    Binding *pad_bindings = (Binding *)(&pad[current_joypad]);

    for (int i = 0; i < NUM_JOYPAD_LINKS; i++)
    {
        if (!strcmp(b_links[i].button_name, string))
        {
            pad_bindings[i] = binding;
        }
        else
        {
        }
    }

    for (int i = NUM_JOYPAD_LINKS; b_links[i].button_name; i++)
    {
        if (!strcmp(b_links[i].button_name, string))
        {
            shortcut[i - NUM_JOYPAD_LINKS] = binding;
        }
        else
        {
            if (shortcut[i - NUM_JOYPAD_LINKS] == binding)
            {
                shortcut[i - NUM_JOYPAD_LINKS].clear();
            }
        }
    }

    focus_next();

    bindings_to_dialog(get_combo("control_combo"));
}

int Snes9xPreferences::get_focused_binding()
{
    for (int i = 0; b_links[i].button_name; i++)
    {
        if (has_focus(b_links[i].button_name))
            return i;
    }

    return -1;
}

void Snes9xPreferences::clear_binding(const char *name)
{
    Binding unset;
    int i;

    for (i = 0; i < NUM_JOYPAD_LINKS; i++)
    {
        if (!strcmp(name, b_links[i].button_name))
        {
            int current_joypad = get_combo("control_combo");
            pad[current_joypad].data[i] = unset;
            break;
        }
    }
    if (i == NUM_JOYPAD_LINKS)
    {
        for (i = NUM_JOYPAD_LINKS; b_links[i].button_name; i++)
        {
            if (!strcmp(name, b_links[i].button_name))
            {
                shortcut[i - NUM_JOYPAD_LINKS] = unset;
                break;
            }
        }
    }

    if (b_links[i].button_name)
    {
        char buf[256];
        unset.to_string(buf);
        set_entry_text(b_links[i].button_name, buf);
    }
}

void Snes9xPreferences::bindings_to_dialog(int joypad)
{
    Binding *bindings = (Binding *)&pad[joypad].data;

    set_combo("control_combo", joypad);

    for (int i = 0; i < NUM_JOYPAD_LINKS; i++)
    {
        set_entry_text(b_links[i].button_name, bindings[i].as_string().c_str());
    }

    auto shortcut_names = &b_links[NUM_JOYPAD_LINKS];

    for (int i = 0; shortcut_names[i].button_name; i++)
    {
        set_entry_text(shortcut_names[i].button_name, shortcut[i].as_string().c_str());
    }
}

void Snes9xPreferences::calibration_dialog()
{
    config->joystick_register_centers();
    auto dialog = Gtk::MessageDialog(_("Current joystick centers have been saved."));
    dialog.set_title(_("Calibration Complete"));
    dialog.run();
    dialog.hide();
}
