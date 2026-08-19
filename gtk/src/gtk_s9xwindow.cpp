/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_compat.h"
#include "gtk_config.h"

#ifdef GDK_WINDOWING_X11
#include <X11/Xatom.h>
#endif

#ifdef USE_XV
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>
#endif

#include "gtk_shader_parameters.h"
#include "gtk_state_preview_dialog.h"

#include "gtk_s9x.h"
#include "gtk_preferences.h"
#include "gtk_display.h"
#include "gtk_file.h"
#include "gtk_sound.h"
#include "gtk_control.h"
#include "gtk_cheat.h"
#include "gtk_netplay.h"
#include "gtk_retroachievements.h"
#include "retroachievements.h"
#include "gtk_s9xwindow.h"

#include "fmt/format.h"
#include <array>
#include <utility>

#include "snes9x.h"
#include "ppu.h"
#include "controls.h"
#include "movie.h"
#include "apu/apu.h"
#include "memmap.h"
#include "cpuexec.h"
#include "snapshot.h"
#include "netplay.h"
#include "display.h"
#include "voicekun.h"

static Glib::RefPtr<Gtk::FileFilter> get_save_states_file_filter()
{
    const auto extensions = { "*.sst", "*.000", "*.001", "*.002", "*.003", "*.004",
                              "*.005", "*.006", "*.007", "*.008", "*.009" };
    auto filter = Gtk::FileFilter::create();
    filter->set_name(_("Save States"));
    for (auto &ext : extensions)
        filter->add_pattern(ext);

    return filter;
}

static Glib::RefPtr<Gtk::FileFilter> get_all_files_filter()
{
    auto filter = Gtk::FileFilter::create();
    filter->set_name(_("All Files"));
    filter->add_pattern("*.*");
    filter->add_pattern("*");
    return filter;
}

Snes9xWindow::Snes9xWindow(Snes9xConfig *config)
    : GtkBuilderWindow("main_window")
{
    user_pause = 0;
    sys_pause = 0;
    last_width = -1;
    last_height = -1;
    this->config = config;
    recent_menu = nullptr;
    fullscreen_state = 0;
    maximized_state = 0;
    focused = true;
    paused_from_focus_loss = false;
    cr = nullptr;
    cairo_owned = false;
    mouse_grabbed = false;

    if (Gtk::IconTheme::get_default()->has_icon("snes9x"))
    {
        Gtk::Window::set_default_icon_name("snes9x");
    }
    else
    {
        extern int mini_icon_size;
        extern unsigned char mini_icon[];
        auto loader = Gdk::PixbufLoader::create();
        loader->write(mini_icon, mini_icon_size);
        loader->close();
        if (auto pixbuf = loader->get_pixbuf())
            Gtk::Window::set_default_icon(pixbuf);
    }

    drawing_area = get_object<Gtk::DrawingArea>("drawingarea").get();
    window->show();
    drawing_area->show();

    enable_widget("shader_parameters_item", false);

    connect_signals();

    if (config->window_width < 100 || config->window_height < 100)
    {
        config->window_width = 256;
        config->window_height = 224;
    }

    window->get_window()->set_cursor();

    resize(config->window_width, config->window_height);
}

void Snes9xWindow::connect_signals()
{
    drawing_area->signal_draw().connect(sigc::mem_fun(*this, &Snes9xWindow::draw));
    window->signal_delete_event().connect([](GdkEventAny *event) -> bool {
        S9xExit();
        return false;
    });
    get_object<Gtk::MenuItem>("exit_item")->signal_activate().connect([] {
         S9xExit();
    });

    window->signal_window_state_event().connect([&](GdkEventWindowState *state) -> bool {
        fullscreen_state = state->new_window_state & GDK_WINDOW_STATE_FULLSCREEN;
        maximized_state = state->new_window_state & GDK_WINDOW_STATE_MAXIMIZED;
        return false;
    });

    window->signal_focus_in_event().connect([&](GdkEventFocus *event) -> bool {
        focus_notify(true);
        return false;
    });

    window->signal_focus_out_event().connect([&](GdkEventFocus *event) -> bool {
        focus_notify(false);
        return false;
    });

    last_key_pressed_keyval = 0;
    last_key_pressed_type = GDK_NOTHING;
    window->signal_key_press_event().connect(sigc::mem_fun(*this, &Snes9xWindow::event_key), false);
    window->signal_key_release_event().connect(sigc::mem_fun(*this, &Snes9xWindow::event_key), false);

    drawing_area->signal_button_press_event().connect(sigc::mem_fun(*this, &Snes9xWindow::button_press));
    drawing_area->signal_button_release_event().connect(sigc::mem_fun(*this, &Snes9xWindow::button_release));
    drawing_area->signal_motion_notify_event().connect(sigc::mem_fun(*this, &Snes9xWindow::motion_notify));

    get_object<Gtk::MenuItem>("continue_item")->signal_activate().connect([&] {
        unpause_from_user();
    });

    get_object<Gtk::MenuItem>("pause_item")->signal_activate().connect([&] {
        pause_from_user();
    });

    get_object<Gtk::MenuItem>("fullscreen_item")->signal_activate().connect([&] {
        toggle_fullscreen_mode();
    });

    get_object<Gtk::MenuItem>("open_rom_item")->signal_activate().connect([&] {
        open_rom_dialog();
    });

    get_object<Gtk::MenuItem>("reset_item")->signal_activate().connect([&] {
        S9xSoftReset();
#ifdef RETROACHIEVEMENTS_SUPPORT
        RA_OnReset();
#endif
    });

    get_object<Gtk::MenuItem>("hard_reset_item")->signal_activate().connect([&] {
        S9xReset();
#ifdef RETROACHIEVEMENTS_SUPPORT
        RA_OnReset();
#endif
    });

    auto reload_with_bios_pref = [&](uint8_t pref) {
        if (refreshing_bios_menu)
            return;
        if (Settings.SGB_BIOSPreference == pref)
            return;
        Settings.SGB_BIOSPreference = pref;
        if (Settings.GBRomPath[0])
            try_open_rom(std::string(Settings.GBRomPath));
    };
    get_object<Gtk::RadioMenuItem>("bios_none_item")->signal_toggled().connect([&, reload_with_bios_pref] {
        if (get_object<Gtk::RadioMenuItem>("bios_none_item")->get_active())
            reload_with_bios_pref(0);
    });
    get_object<Gtk::RadioMenuItem>("bios_sgb1_item")->signal_toggled().connect([&, reload_with_bios_pref] {
        if (get_object<Gtk::RadioMenuItem>("bios_sgb1_item")->get_active())
            reload_with_bios_pref(1);
    });
    get_object<Gtk::RadioMenuItem>("bios_sgb2_item")->signal_toggled().connect([&, reload_with_bios_pref] {
        if (get_object<Gtk::RadioMenuItem>("bios_sgb2_item")->get_active())
            reload_with_bios_pref(2);
    });

    for (int i = 0; i <= 4; i++)
    {
        std::string name = "run_ahead_" + std::to_string(i) + "_item";
        get_object<Gtk::RadioMenuItem>(name.c_str())->signal_toggled().connect([&, name, i] {
            if (refreshing_runahead_menu)
                return;
            if (get_object<Gtk::RadioMenuItem>(name.c_str())->get_active())
                Settings.RunAhead = i;
        });
    }
    // The preferences dialog can also change Settings.RunAhead, so sync the
    // radio state whenever the menu opens.
    get_object<Gtk::Menu>("emulation_menu_item_menu")->signal_show().connect([&] {
        int clamped = Settings.RunAhead < 0 ? 0 : (Settings.RunAhead > 4 ? 4 : Settings.RunAhead);
        std::string name = "run_ahead_" + std::to_string(clamped) + "_item";
        refreshing_runahead_menu = true;
        get_object<Gtk::RadioMenuItem>(name.c_str())->set_active(true);
        refreshing_runahead_menu = false;
    });

    get_object<Gtk::MenuItem>("shader_parameters_item")->signal_activate().connect([&] {
        gtk_shader_parameters_dialog(get_window());
    });

    const std::vector<const char *> port_items = { "joypad1", "mouse1", "superscope1", "joypad2", "mouse2", "multitap2", "superscope2", "nothingpluggedin2" };
    for (auto &name : port_items)
    {
        get_object<Gtk::MenuItem>(name)->signal_activate().connect(sigc::bind<const char *>(sigc::mem_fun(*this, &Snes9xWindow::port_activate), name));
    }

    build_state_menus();

    get_object<Gtk::MenuItem>("from_file1")->signal_activate().connect([&] {
        load_state_dialog();
    });

    get_object<Gtk::MenuItem>("to_file1")->signal_activate().connect([&] {
        save_state_dialog();
    });

    get_object<Gtk::MenuItem>("save_state_preview_item")->signal_activate().connect([&] {
        state_preview_dialog(true);
    });

    get_object<Gtk::MenuItem>("load_state_preview_item")->signal_activate().connect([&] {
        state_preview_dialog(false);
    });

    get_object<Gtk::MenuItem>("load_state_undo")->signal_activate().connect([&] {
        S9xUnfreezeGame(S9xGetFilename(".undo", SNAPSHOT_DIR).c_str());
    });

    get_object<Gtk::MenuItem>("save_spc_item")->signal_activate().connect([&] {
        save_spc_dialog();
    });

    get_object<Gtk::MenuItem>("open_movie_item")->signal_activate().connect([&] {
        if (S9xMovieActive())
            S9xMovieStop(false);

        S9xMovieOpen(open_movie_dialog(true).c_str(), false);
    });

    get_object<Gtk::MenuItem>("stop_recording_item")->signal_activate().connect([&] {
        if (S9xMovieActive())
            S9xMovieStop(false);
    });

    get_object<Gtk::MenuItem>("jump_to_frame_item")->signal_activate().connect([&] {
        movie_seek_dialog();
    });

    get_object<Gtk::MenuItem>("record_movie_item")->signal_activate().connect([&] {
        if (S9xMovieActive())
            S9xMovieStop(false);

        S9xMovieCreate(open_movie_dialog(false).c_str(), 0xFF, MOVIE_OPT_FROM_RESET, nullptr, 0);
    });

    get_object<Gtk::MenuItem>("cheats_item")->signal_activate().connect([&] {
        open_snes9x_cheats_dialog();
    });

    get_object<Gtk::MenuItem>("preferences_item")->signal_activate().connect([&] {
        snes9x_preferences_open(this);
    });

    get_object<Gtk::MenuItem>("open_netplay_item")->signal_activate().connect([&] {
        S9xNetplayDialogOpen();
    });

    get_object<Gtk::MenuItem>("rom_info_item")->signal_activate().connect([&] {
        show_rom_info();
    });

    get_object<Gtk::MenuItem>("sync_clients_item")->signal_activate().connect([&] {
        S9xNetplaySyncClients();
    });

    get_object<Gtk::MenuItem>("hide_ui")->signal_activate().connect([&] {
        toggle_ui();
    });

    for (int i = 1; i <= 10; i++)
    {
        std::string name = "exact_pixels_" + std::to_string(i) + "x_item";
        get_object<Gtk::MenuItem>(name.c_str())->signal_activate().connect([i, this] {
            resize_to_multiple(i);
        });
    }

    get_object<Gtk::MenuItem>("open_multicart_item")->signal_activate().connect([&] {
        open_multicart_dialog();
    });

    get_object<Gtk::MenuItem>("voicekun_attach_item")->signal_activate().connect([&] {
        open_voicekun_dialog();
    });

    get_object<Gtk::MenuItem>("voicekun_detach_item")->signal_activate().connect([&] {
        if (S9xVoiceKunAttached())
        {
            S9xVoiceKunDetach();
            S9xSetInfoString(_("Voicer-kun: audio CD ejected"));
        }
        configure_widgets();
    });

    // Sound Channels submenu, as on win32's Sound->Channels popup.
    for (int i = 0; i < 8; i++)
    {
        std::string name = "sound_channel_" + std::to_string(i + 1) + "_item";
        auto item = get_object<Gtk::CheckMenuItem>(name.c_str());
        item->signal_toggled().connect([i, item] {
            uint8_t mask = S9xGetSoundChannelMask();
            uint8_t new_mask = item->get_active() ? mask | (1 << i)
                                                  : mask & ~(1 << i);
            // The menu-open sync also flips items; only apply real changes.
            if (new_mask != mask)
                S9xSetSoundChannelMask(new_mask);
        });
    }

    get_object<Gtk::MenuItem>("enable_all_channels_item")->signal_activate().connect([] {
        S9xSetSoundChannelMask(255);
    });

    get_object<Gtk::MenuItem>("color_correction_item")->signal_activate().connect([this] {
        show_color_correction_dialog();
    });

    // Same setting as the preferences dialog's "Mute sound output" checkbox.
    auto mute_item = get_object<Gtk::CheckMenuItem>("mute_item");
    mute_item->signal_toggled().connect([mute_item] {
        bool active = mute_item->get_active();
        // The menu-open sync also flips the item; only apply real changes.
        if (gui_config->mute_sound != active)
        {
            gui_config->mute_sound = active;
            S9xPortSoundReinit();
        }
    });

    get_object<Gtk::MenuItem>("sound_settings_item")->signal_activate().connect([&] {
        snes9x_preferences_open(this, 1); // the Sound tab
    });

    // win32's Input->Enable Rumble (Shake): pass LRG rumble-cart motor
    // effects to the port-1 gamepad.
    auto rumble_item = get_object<Gtk::CheckMenuItem>("enable_rumble_item");
    rumble_item->set_active(gui_config->enable_rumble);
    rumble_item->signal_toggled().connect([rumble_item] {
        gui_config->enable_rumble = rumble_item->get_active();
    });

    get_object<Gtk::MenuItem>("sound_menu_item")->signal_activate().connect([this] {
        uint8_t mask = S9xGetSoundChannelMask();
        // Channels 1-4 drive both SPC voices 1-4 and the GB APU's CH1-CH4.
        // In BIOS-less GB mode the SPC isn't running, so 5-8 control
        // nothing — grey them there, as on win32.
        bool gb_only = Settings.SuperGameBoy && !Settings.SGB_BIOSModeActive;
        for (int i = 0; i < 8; i++)
        {
            std::string name = "sound_channel_" + std::to_string(i + 1) + "_item";
            auto item = get_object<Gtk::CheckMenuItem>(name.c_str());
            item->set_active((mask & (1 << i)) != 0);
            if (i >= 4)
                item->set_sensitive(!gb_only);
        }
        get_object<Gtk::CheckMenuItem>("mute_item")->set_active(gui_config->mute_sound);
    });

#ifdef RETROACHIEVEMENTS_SUPPORT
    get_object<Gtk::CheckMenuItem>("ra_enabled_item")->set_active(gui_config->ra_enabled);
    get_object<Gtk::CheckMenuItem>("ra_enabled_item")->signal_toggled().connect([this] {
        bool checked = get_object<Gtk::CheckMenuItem>("ra_enabled_item")->get_active();
        gui_config->ra_enabled = checked;
        gui_config->save_config_file();
        RA_SetEnabled(checked);
        if (checked)
        {
            RA_Gtk_RegisterCallbacks();
            RA_Init();
            RA_SetHardcoreEnabled(gui_config->ra_hardcore_mode);
            RA_AttemptLogin(gui_config->ra_username.c_str(), gui_config->ra_api_token.c_str());
            if (gui_config->rom_loaded)
                RA_OnLoadROM();
        }
        else
        {
            RA_Shutdown();
        }
    });

    get_object<Gtk::MenuItem>("ra_login_item")->signal_activate().connect([this] {
        RA_Gtk_RegisterCallbacks();
        RA_Init();
        if (RA_IsLoggedIn())
        {
            RA_Logout();
            get_object<Gtk::MenuItem>("ra_login_item")->set_label(_("_Login..."));
        }
        else
        {
            RA_ShowLoginDialog();
        }
    });

    get_object<Gtk::CheckMenuItem>("ra_hardcore_item")->set_active(gui_config->ra_hardcore_mode);
    get_object<Gtk::CheckMenuItem>("ra_hardcore_item")->signal_toggled().connect([this] {
        bool checked = get_object<Gtk::CheckMenuItem>("ra_hardcore_item")->get_active();
        gui_config->ra_hardcore_mode = checked;
        RA_SetHardcoreEnabled(checked);
    });

    get_object<Gtk::MenuItem>("ra_achievement_list_item")->signal_activate().connect([] {
        RA_ShowAchievementList();
    });

    get_object<Gtk::MenuItem>("ra_view_profile_item")->signal_activate().connect([] {
        rc_client_t *client = RA_GetClient();
        const rc_client_user_t *user = client ? rc_client_get_user_info(client) : nullptr;
        if (user && user->username && user->username[0])
        {
            std::string url = std::string("https://retroachievements.org/user/") + user->username;
            g_app_info_launch_default_for_uri(url.c_str(), nullptr, nullptr);
        }
    });

    // Stored credentials imply we auto-login at startup; reflect that in the label.
    if (!gui_config->ra_username.empty() && !gui_config->ra_api_token.empty())
        get_object<Gtk::MenuItem>("ra_login_item")->set_label(_("_Logout"));
#else
    get_object<Gtk::Widget>("retroachievements_menu_item")->hide();
#endif
}

bool Snes9xWindow::button_press(GdkEventButton *event)
{
    if (!S9xIsMousePluggedIn() && event->button == 3)
    {
        get_object<Gtk::Menu>("view_menu_menu")->popup_at_pointer(nullptr);
        return false;
    }

    S9xReportButton(BINDING_MOUSE_BUTTON0 + event->button - 1, true);

    return false;
}

bool Snes9xWindow::button_release(GdkEventButton *event)
{
    S9xReportButton(BINDING_MOUSE_BUTTON0 + event->button - 1, false);

    return false;
}

bool Snes9xWindow::motion_notify(GdkEventMotion *event)
{
    if (!config->rom_loaded || last_width <= 0 || last_height <= 0)
        return false;

    if (mouse_grabbed)
    {
        if (event->x_root == gdk_mouse_x && event->y_root == gdk_mouse_y)
            return false;

        snes_mouse_x += (event->x_root - gdk_mouse_x);
        snes_mouse_y += (event->y_root - gdk_mouse_y);
        center_mouse();

        return false;
    }

    int scale_factor = window->get_scale_factor();

    snes_mouse_x = ((int)(event->x * scale_factor) - mouse_region_x) * 256 /
                   (mouse_region_width <= 0 ? 1 : mouse_region_width);

    snes_mouse_y = ((int)(event->y * scale_factor) - mouse_region_y) * (gui_config->overscan ? SNES_HEIGHT_EXTENDED : SNES_HEIGHT) /
                   (mouse_region_height <= 0 ? 1 : mouse_region_height);

    if (!config->pointer_is_visible)
    {
        if (!S9xIsMousePluggedIn())
            show_mouse_cursor();
    }

    config->pointer_timestamp = g_get_monotonic_time();

    return false;
}

void Snes9xWindow::port_activate(const char *name)
{
    auto item = get_object<Gtk::CheckMenuItem>(name);
    if (!item->get_active())
        return;

    struct {
        const char *name;
        int port;
        enum controllers controller;
        int8_t id1, id2, id3, id4;
    } map[] = {
        { "joypad1", 0, CTL_JOYPAD, 0, 0, 0, 0 },
        { "joypad2", 1, CTL_JOYPAD, 1, 0, 0, 0 },
        { "mouse1", 0, CTL_MOUSE, 0, 0, 0, 0 },
        { "mouse2", 1, CTL_MOUSE, 0, 0, 0, 0 },
        { "superscope1", 0, CTL_SUPERSCOPE, 0, 0, 0, 0 },
        { "superscope2", 1, CTL_SUPERSCOPE, 0, 0, 0, 0 },
        { "multitap1", 0, CTL_MP5, 0, 1, 2, 3 },
        { "multitap2", 1, CTL_MP5, 1, 2, 3, 4 },
        { "nothingpluggedin2", 1, CTL_NONE, 0, 0, 0, 0}
    };

    for (auto &m : map)
        if (!strcasecmp(m.name, name))
        {
            S9xSetController(m.port, m.controller, m.id1, m.id2, m.id3, m.id4);
            break;
        }
}

bool Snes9xWindow::event_key(GdkEventKey *event)
{
    Binding b;
    s9xcommand_t cmd;

    // Ignore multiple identical keypresses to discard repeating keys
    if (event->keyval == last_key_pressed_keyval && event->type == last_key_pressed_type)
    {
        return true;
    }

    // Provide escape key to get out of fullscreen
    if (event->keyval == GDK_Escape)
    {
        if (event->type == GDK_KEY_RELEASE)
        {
            if (config->default_esc_behavior == ESC_EXIT_FULLSCREEN)
                leave_fullscreen_mode();
            else if (config->default_esc_behavior == ESC_EXIT_SNES9X)
                S9xExit();
            else
                toggle_ui();
        }

        return true;
    }

    last_key_pressed_keyval = event->keyval;
    last_key_pressed_type = event->type;

    b = Binding(event);

    // If no mapping for modifier version exists, try non-modifier
    cmd = S9xGetMapping(b.hex());
    if (cmd.type == S9xNoMapping)
    {
        b = Binding(event->keyval, false, false, false);
        cmd = S9xGetMapping(b.hex());
    }

    if (cmd.type != S9xNoMapping)
    {
        S9xReportButton(b.hex(), (event->type == GDK_KEY_PRESS));
        return true;
    }

    return false; // Pass the key to GTK
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
    if (!screen_ptr)
        return;

    // Load splash image (RGB24) into Snes9x buffer (RGB15)
    last_width = 256;
    last_height = 224;

    if (config->splash_image == SPLASH_IMAGE_PATTERN ||
        config->splash_image == SPLASH_IMAGE_SMTPE   ||
        config->splash_image == SPLASH_IMAGE_COMBO) {
        unsigned char *pattern = nullptr;
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

        auto pixbuf_loader = gdk_pixbuf_loader_new_with_type("png", nullptr);
        gdk_pixbuf_loader_write(pixbuf_loader, pattern, pattern_size, nullptr);
        gdk_pixbuf_loader_close(pixbuf_loader, nullptr);
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
}

bool Snes9xWindow::draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    this->cr = cr->cobj();
    cairo_owned = false;

    if (!(config->fullscreen) && !(maximized_state))
    {
        auto vbox = get_object<Gtk::VBox>("main_window_vbox");
        config->window_width = vbox->get_width();
        config->window_height = vbox->get_height();
    }

    if (last_width < 0)
    {
        setup_splash();
    }

    S9xDisplayRefresh();

    if ((is_paused() || NetPlay.Paused) && (gui_config->splash_image < SPLASH_IMAGE_STARFIELD || gui_config->rom_loaded))
    {
        S9xDeinitUpdate(last_width, last_height);
    }

    this->cr = nullptr;

    return true;
}

void Snes9xWindow::focus_notify(bool state)
{
    focused = state;

    if (!state && config->pause_emulation_on_switch && !paused_from_focus_loss)
    {
        sys_pause++;
        propagate_pause_state();
        paused_from_focus_loss = true;
    }

    if (state && paused_from_focus_loss)
    {
        unpause_from_focus_change();
        paused_from_focus_loss = false;
    }
}

void Snes9xWindow::open_voicekun_dialog()
{
    if (S9xVoiceKunAttached())
        return;

    pause_from_focus_change();

    Gtk::FileChooserDialog dialog(*window.get(),
                                  _("Attach Audio CD (.cue or .zip)"),
                                  Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.add_button(Gtk::StockID("gtk-cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::StockID("gtk-open"), Gtk::RESPONSE_ACCEPT);

    auto filter = Gtk::FileFilter::create();
    filter->set_name(_("Audio CD Images"));
    for (const char *ext : { "*.cue", "*.CUE", "*.zip", "*.ZIP" })
        filter->add_pattern(ext);
    dialog.add_filter(filter);
    dialog.add_filter(get_all_files_filter());

    if (!gui_config->last_directory.empty())
        dialog.set_current_folder(config->last_directory);

    auto result = dialog.run();
    dialog.hide();

    if (result == Gtk::RESPONSE_ACCEPT)
    {
        auto filename = dialog.get_filename();
        if (S9xVoiceKunAttach(filename.c_str()))
        {
            auto message = fmt::format("Voicer-kun: {} verified, {} tracks ({})",
                                       S9xVoiceKunDiscLabel(),
                                       S9xVoiceKunTrackCount(),
                                       S9xVoiceKunGameTitle());
            S9xSetInfoString(message.c_str());
        }
        else
        {
            auto message = fmt::format(fmt::runtime(_("This audio CD was not accepted:\n{}")),
                                       S9xVoiceKunLastError());
            Gtk::MessageDialog msg(*window.get(), message, false,
                                   Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
            msg.set_title(_("Voicer-kun"));
            msg.run();
        }
    }

    unpause_from_focus_change();
    configure_widgets();
}

void Snes9xWindow::open_multicart_dialog()
{
    GtkBuilderWindow dialog_builder("multicart_dialog");
    auto dialog = Glib::RefPtr<Gtk::Dialog>::cast_static(dialog_builder.window);

    dialog->set_transient_for(*window.get());

    pause_from_focus_change();

    auto slota = get_object<Gtk::FileChooserDialog>("multicart_slota");
    auto slotb = get_object<Gtk::FileChooserDialog>("multicart_slotb");

    slota->set_current_folder(config->last_directory);
    slotb->set_current_folder(config->last_directory);

    int result = dialog->run();
    dialog->hide();

    if (result == GTK_RESPONSE_OK)
    {
        auto filename = slota->get_filename();
        if (!filename.empty())
            strncpy(Settings.CartAName, filename.c_str(), PATH_MAX);
        else
            Settings.CartAName[0] = '\0';

        filename = slotb->get_filename();
        if (!filename.empty())
            strncpy(Settings.CartBName, filename.c_str(), PATH_MAX);
        else
            Settings.CartBName[0] = '\0';

        Settings.Multi = true;

        if (S9xOpenROM(nullptr))
        {
            auto msg = Gtk::MessageDialog(*window.get(),
                                          _("Couldn't load files."),
                                          false,
                                          Gtk::MESSAGE_ERROR,
                                          Gtk::BUTTONS_CLOSE,
                                          true);
            msg.run();
        }
    }

    unpause_from_focus_change();
}

std::string Snes9xWindow::open_movie_dialog(bool readonly)
{
    this->pause_from_focus_change();

    std::string title;
    Gtk::FileChooserAction action;

    if (readonly)
    {
        title = _("Open SNES Movie");
        action = Gtk::FILE_CHOOSER_ACTION_OPEN;
    }
    else
    {
        title = _("New SNES Movie");
        action = Gtk::FILE_CHOOSER_ACTION_SAVE;
    }

    Gtk::FileChooserDialog dialog(*window.get(), title, action);
    dialog.add_button(Gtk::StockID("gtk-cancel"), Gtk::RESPONSE_CANCEL);
    if (readonly)
        dialog.add_button(Gtk::StockID("gtk-open"), Gtk::RESPONSE_ACCEPT);
    else
        dialog.add_button(Gtk::StockID("gtk-save"), Gtk::RESPONSE_ACCEPT);

    if (!readonly)
    {
        auto default_name = S9xGetFilename(".smv", s9x_getdirtype::ROM_DIR);
        dialog.set_current_name(default_name);

    }

    auto filter = Gtk::FileFilter::create();
    filter->set_name(_("SNES Movies"));
    filter->add_pattern("*.smv");
    filter->add_pattern("*.SMV");
    dialog.add_filter(filter);
    dialog.add_filter(get_all_files_filter());

    dialog.set_current_folder(S9xGetDirectory(SRAM_DIR));
    auto result = dialog.run();
    dialog.hide();
    this->unpause_from_focus_change();

    if (result == Gtk::RESPONSE_ACCEPT)
        return dialog.get_filename();

    return std::string{};
}

std::string Snes9xWindow::open_rom_dialog(bool run)
{
    // Gtk::FileFilter patterns are case-sensitive, hence the doubled entries.
    const auto snes_extensions = {
        "*.smc", "*.SMC", "*.fig", "*.FIG", "*.sfc", "*.SFC",
        "*.swc", "*.SWC", "*.gd3", "*.GD3", "*.bs", "*.BS",
        "*.st", "*.ST", "*.bin", "*.BIN"
    };
    // .gb/.gbc route into the SGB subsystem in CMemory::LoadROM, and .sgb (plus
    // any GB dump under a foreign extension) is caught by the Nintendo-logo
    // content sniff, so Game Boy carts load straight from here.
    const auto gb_extensions = {
        "*.gb", "*.GB", "*.gbc", "*.GBC", "*.sgb", "*.SGB"
    };
    const auto archive_extensions = {
        "*.jma", "*.JMA", "*.zip", "*.ZIP", "*.gz", "*.GZ"
    };

    pause_from_focus_change();

    auto dialog = Gtk::FileChooserDialog(*top_level->window.get(),
                                         _("Open ROM Image"));
    dialog.add_button(Gtk::StockID("gtk-cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::StockID("gtk-open"), Gtk::RESPONSE_ACCEPT);

    auto add_patterns = [](const Glib::RefPtr<Gtk::FileFilter> &filter,
                           const std::initializer_list<const char *> &extensions) {
        for (const auto &ext : extensions)
            filter->add_pattern(ext);
    };

    auto filter = Gtk::FileFilter::create();
    filter->set_name(_("ROM Images"));
    add_patterns(filter, snes_extensions);
    add_patterns(filter, gb_extensions);
    add_patterns(filter, archive_extensions);
    dialog.add_filter(filter);

    auto snes_filter = Gtk::FileFilter::create();
    snes_filter->set_name(_("Super Nintendo ROM Images"));
    add_patterns(snes_filter, snes_extensions);
    dialog.add_filter(snes_filter);

    auto gb_filter = Gtk::FileFilter::create();
    gb_filter->set_name(_("Game Boy ROM Images"));
    add_patterns(gb_filter, gb_extensions);
    dialog.add_filter(gb_filter);

    dialog.add_filter(get_all_files_filter());

    if (!gui_config->last_directory.empty())
        dialog.set_current_folder(config->last_directory);

    auto result = dialog.run();
    dialog.hide();

    std::string filename;

    if (result == Gtk::RESPONSE_ACCEPT)
    {
        std::string directory = dialog.get_current_folder();

        if (!directory.empty())
            gui_config->last_directory = directory;

        filename = dialog.get_filename();

        if (!filename.empty())
        {
            if (run)
            {
                Settings.Multi = false;
                try_open_rom(filename);
            }
        }
    }

    unpause_from_focus_change();
    return filename;
}

bool Snes9xWindow::try_open_rom(const std::string &filename)
{
    pause_from_focus_change();

    Settings.Multi = false;

    if (S9xOpenROM(filename.c_str()))
    {
        std::string message = _("Couldn't load file: ") + filename;
        Gtk::MessageDialog msg(*window.get(), message, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
        msg.run();
        unpause_from_focus_change();
        return false;
    }

    // The load has already happened, so the SGB flags say what kind of cart it
    // is — more reliable than the extension (a GB dump can arrive as .sgb, or
    // zipped, and still be routed to the GB core by the content sniff).
    const bool gb_cart = Settings.SuperGameBoy || Settings.SGB_BIOSModeActive;

    Gtk::RecentManager::Data data;
    data.description = gb_cart ? "Game Boy ROM" : "SNES ROM";
    data.mime_type = gb_cart ? "application/x-gameboy-rom" : "application/x-snes-rom";
    data.app_name = "SuperSnes9x";
    data.groups = { "cartridge" };
    data.is_private = false;
    data.app_exec = Glib::get_prgname() + " %f";
    auto uri = Glib::filename_to_uri(filename);
    Gtk::RecentManager::get_default()->add_item(uri, data);

    unpause_from_user();
    unpause_from_focus_change();

    return true;
}

void Snes9xWindow::load_state_dialog()
{
    this->pause_from_focus_change();

    Gtk::FileChooserDialog dialog(*window.get(), _("Load Saved State"));
    dialog.add_button(Gtk::StockID("gtk-cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::StockID("gtk-open"), Gtk::RESPONSE_ACCEPT);

    dialog.add_filter(get_save_states_file_filter());
    dialog.add_filter(get_all_files_filter());
    dialog.set_current_folder(S9xGetDirectory(SNAPSHOT_DIR));

    auto result = dialog.run();
    dialog.hide();
    if (result == Gtk::RESPONSE_ACCEPT)
    {
#ifdef RETROACHIEVEMENTS_SUPPORT
        if (!RA_WarnDisableHardcore(_("Loading save states")))
        {
            unpause_from_focus_change();
            return;
        }
#endif
        std::string filename = dialog.get_filename();
        S9xLoadState(filename.c_str());
#ifdef RETROACHIEVEMENTS_SUPPORT
        RA_OnLoadState(filename.c_str());
#endif
    }

    unpause_from_focus_change();
}

/* win32's "Save/Load with Preview": pick a slot from a thumbnail grid. */
void Snes9xWindow::state_preview_dialog(bool is_save)
{
    if (!config->rom_loaded)
        return;

#ifdef RETROACHIEVEMENTS_SUPPORT
    if (!is_save && !RA_WarnDisableHardcore(_("Loading save states")))
        return;
#endif

    pause_from_focus_change();

    int slot = S9xStatePreviewDialog(*window.get(), is_save);

    if (slot >= 0)
    {
        if (is_save)
            S9xQuickSaveSlot(slot);
        else
            S9xQuickLoadSlot(slot);
    }

    unpause_from_focus_change();
}

void Snes9xWindow::movie_seek_dialog()
{
    if (!S9xMovieActive())
        return;

    pause_from_focus_change();

    GtkBuilderWindow seek_dialog("frame_advance_dialog");

    {
        std::string str;
        str = fmt::format(fmt::runtime(_("The current frame in the movie is <b>{0:Ld}</b>.")), S9xMovieGetFrameCounter());
        seek_dialog.get_object<Gtk::Label>("current_frame_label")->set_label(str);

        str = fmt::format("{0:d}", S9xMovieGetFrameCounter());
        seek_dialog.set_entry_text("frame_entry", str);
    }

    auto dialog = Glib::RefPtr<Gtk::Dialog>::cast_static(seek_dialog.window);

    dialog->set_transient_for(*window.get());
    auto result = dialog->run();

    int entry_value = seek_dialog.get_entry_value("frame_entry");

    if (result == Gtk::RESPONSE_OK)
    {
        if (entry_value > 0 &&
            entry_value > (int)S9xMovieGetFrameCounter())
        {
            Settings.HighSpeedSeek =
                entry_value - S9xMovieGetFrameCounter();
        }
    }

    unpause_from_focus_change();
}

void Snes9xWindow::save_state_dialog()
{
    pause_from_focus_change();

    auto dialog = Gtk::FileChooserDialog(*window.get(), _("Save State"), Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.add_button(Gtk::StockID("gtk-cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::StockID("gtk-save"), Gtk::RESPONSE_ACCEPT);
    dialog.set_current_folder(S9xGetDirectory(SNAPSHOT_DIR));
    dialog.set_current_name(S9xBasename(S9xGetFilename(".sst", SNAPSHOT_DIR)));
    dialog.add_filter(get_save_states_file_filter());
    dialog.add_filter(get_all_files_filter());

    auto result = dialog.run();
    dialog.hide();

    if (result == GTK_RESPONSE_ACCEPT)
    {
        std::string filename = dialog.get_filename();
        S9xSaveState(filename.c_str());
#ifdef RETROACHIEVEMENTS_SUPPORT
        RA_OnSaveState(filename.c_str());
#endif
    }

    unpause_from_focus_change();
}

void Snes9xWindow::save_spc_dialog()
{
    pause_from_focus_change();

    auto dialog = Gtk::FileChooserDialog(*window.get(), _("Save SPC file..."), Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.add_button(Gtk::StockID("gtk-cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::StockID("gtk-save"), Gtk::RESPONSE_ACCEPT);
    dialog.set_current_folder(S9xGetDirectory(SNAPSHOT_DIR));
    dialog.set_current_name(S9xGetFilename(".spc", SNAPSHOT_DIR));
    auto filter = Gtk::FileFilter::create();
    filter->add_pattern("*.spc");
    filter->add_pattern("*.SPC");
    dialog.add_filter(filter);
    dialog.add_filter(get_all_files_filter());

    auto result = dialog.run();
    dialog.hide();

    if (result == Gtk::RESPONSE_ACCEPT)
    {
        if (S9xSPCDump(dialog.get_filename().c_str()))
        {
            /* Success ? */
        }
        else
        {
            std::string message = _("Couldn't save SPC file:");
            message += " " + dialog.get_filename();
            Gtk::MessageDialog(*window.get(), message, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true).run();
        }
    }

    unpause_from_focus_change();
}

void Snes9xWindow::set_menu_item_selected(const char *name)
{
    get_object<Gtk::CheckMenuItem>(name)->set_active(true);
}

void Snes9xWindow::show_rom_info()
{
const char *markup = _(R"(<b>Information for %s</b>

<i>Name:</i> %s
<i>Speed:</i> %02X/%s
<i>Map:</i> %s
<i>Type:</i> %02x
<i>Contents:</i> %s
<i>ROM Size:</i> %s
<i>Calculated Size:</i> %d
<i>SRAM Size:</i> %s
<i>Header Checksum:</i> %04X
<i>Checksum Complement:</i> %04X
<i>Actual Checksum:</i> %04X
<i>Video:</i> %s
<i>CRC32:</i> %08X
<i>Revision:</i> %s
<b><i>%s%s</i></b>)");
    char output[2048];

    snprintf(output, 2048, markup,
             Memory.ROMFilename.c_str(),
             Glib::convert(Memory.ROMName, "utf8", "shift-jis").c_str(),
             Memory.ROMSpeed,
             ((Memory.ROMSpeed & 0x10) != 0) ? "FastROM" : "SlowROM",
             (Memory.HiROM) ? "HiROM" : "LoROM",
             Memory.ROMType,
             Memory.KartContents(),
             Memory.Size(),
             Memory.CalculatedSize / 0x20000,
             Memory.StaticRAMSize(),
             Memory.ROMChecksum,
             Memory.ROMComplementChecksum,
             Memory.CalculatedChecksum,
             (Memory.ROMRegion > 12 || Memory.ROMRegion < 2) ? "NTSC 60Hz" : "PAL 50Hz",
             Memory.ROMCRC32,
             Memory.Revision(),
             (Settings.IsPatched) ? _("\n\nThis ROM has been auto-patched with ") : (Memory.ROMChecksum != Memory.CalculatedChecksum) ? _("\n\nThis ROM has been modified or damaged") : "",
             Settings.IsPatched == 1 ? "IPS" : Settings.IsPatched == 2 ? "BPS" : Settings.IsPatched == 3 ? "UPS" : "");

    std::string str_output(output);

    for (size_t i = 0; i < str_output.length(); i++)
    {
        if (str_output[i] == '&')
            str_output.insert(i + 1, "amp;");
    }

    pause_from_focus_change();

    auto dialog = Gtk::MessageDialog(*window.get(), str_output, true, Gtk::MESSAGE_OTHER, Gtk::BUTTONS_CLOSE, true);
    dialog.set_title(_("File Information"));
    dialog.run();

    unpause_from_focus_change();
}

/* win32's Video->Color Correction dialog: an accurate-SNES-colors toggle plus
 * optional gamma/contrast/saturation adjustments. Values apply on OK. */
void Snes9xWindow::show_color_correction_dialog()
{
    const int RESPONSE_DEFAULTS = 1;

    pause_from_focus_change();

    Gtk::Dialog dialog(_("Color Correction"), *window.get(), true);
    dialog.add_button(_("Defaults"), RESPONSE_DEFAULTS);
    dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(_("_OK"), Gtk::RESPONSE_OK);
    dialog.set_default_response(Gtk::RESPONSE_OK);

    auto vbox = Gtk::manage(new Gtk::VBox);
    vbox->set_spacing(6);
    vbox->set_border_width(12);

    auto correction_check = Gtk::manage(new Gtk::CheckButton(_("Enable color correction (accurate SNES colors)")));
    correction_check->set_active(Settings.ColorCorrection);
    vbox->pack_start(*correction_check, Gtk::PACK_SHRINK);

    auto frame = Gtk::manage(new Gtk::Frame(_("Adjustments")));
    auto grid = Gtk::manage(new Gtk::Grid);
    grid->set_row_spacing(6);
    grid->set_column_spacing(12);
    grid->set_border_width(12);

    auto adjustments_check = Gtk::manage(new Gtk::CheckButton(_("Apply adjustments")));
    adjustments_check->set_active(Settings.AdjustmentsEnabled);
    grid->attach(*adjustments_check, 0, 0, 2, 1);

    const std::pair<const char *, int> rows[] = {
        { _("Gamma:"), Settings.Gamma },
        { _("Contrast:"), Settings.Contrast },
        { _("Saturation:"), Settings.Saturation },
    };

    std::array<Gtk::Scale *, 3> scales{};
    for (int i = 0; i < 3; i++)
    {
        auto label = Gtk::manage(new Gtk::Label(rows[i].first));
        label->set_halign(Gtk::ALIGN_END);

        auto scale = Gtk::manage(new Gtk::Scale(
            Gtk::Adjustment::create(rows[i].second, -100.0, 100.0, 1.0, 10.0, 0.0)));
        scale->set_digits(0);
        scale->set_value_pos(Gtk::POS_RIGHT);
        scale->set_size_request(300, -1);
        scale->set_hexpand(true);
        scale->add_mark(0.0, Gtk::POS_BOTTOM, "");
        scale->signal_format_value().connect([](double value) -> Glib::ustring {
            int v = (int)value;
            return (v > 0 ? "+" : "") + std::to_string(v);
        });
        scales[i] = scale;

        grid->attach(*label, 0, i + 1, 1, 1);
        grid->attach(*scale, 1, i + 1, 1, 1);
    }

    // Sliders mean nothing until "Apply adjustments" is on, as on win32.
    auto sync_sensitive = [adjustments_check, scales] {
        for (auto *scale : scales)
            scale->set_sensitive(adjustments_check->get_active());
    };
    adjustments_check->signal_toggled().connect(sync_sensitive);
    sync_sensitive();

    frame->add(*grid);
    vbox->pack_start(*frame, Gtk::PACK_SHRINK);

    dialog.get_content_area()->pack_start(*vbox, Gtk::PACK_EXPAND_WIDGET);
    dialog.show_all();

    int result;
    while ((result = dialog.run()) == RESPONSE_DEFAULTS)
    {
        correction_check->set_active(false);
        adjustments_check->set_active(false);
        for (auto *scale : scales)
            scale->set_value(0.0);
    }

    if (result == Gtk::RESPONSE_OK)
    {
        Settings.ColorCorrection = correction_check->get_active();
        Settings.AdjustmentsEnabled = adjustments_check->get_active();
        Settings.Gamma = (int)scales[0]->get_value();
        Settings.Contrast = (int)scales[1]->get_value();
        Settings.Saturation = (int)scales[2]->get_value();
        // The palette tables bake these in; rebuild them so the change shows.
        if (config->rom_loaded)
            S9xFixColourBrightness();
    }

    unpause_from_focus_change();
}

void Snes9xWindow::configure_widgets()
{
    auto enable_when_rom_loaded = {
        "continue_item",
        "pause_item",
        "reset_item",
        "load_state_item",
        "save_state_item",
        "save_state_preview_item",
        "load_state_preview_item",
        "save_spc_item",
        "hard_reset_item",
        "record_movie_item",
        "stop_recording_item",
        "open_movie_item",
        "jump_to_frame_item",
        "cheats_item",
        "rom_info_item",
        "run_ahead_item"
    };
    for (auto &widget : enable_when_rom_loaded)
        enable_widget(widget, config->rom_loaded);

#ifdef RETROACHIEVEMENTS_SUPPORT
    // The achievement list only makes sense with a game loaded and a user
    // logged in (win32 gates it on a running game); View Profile needs a login.
    enable_widget("ra_achievement_list_item", config->rom_loaded && RA_IsLoggedIn());
    enable_widget("ra_view_profile_item", RA_IsLoggedIn());
#endif

    enable_widget("sync_clients_item",
                  config->rom_loaded &&
                      Settings.NetPlay &&
                      Settings.NetPlayServer);

    const bool gb_loaded = (Settings.GBRomPath[0] != '\0');
    show_widget("bios_item", gb_loaded);
    show_widget("bios_separator", gb_loaded);
    if (gb_loaded)
    {
        const bool sgb1_avail = S9xSGBBIOSAvailable(1, Settings.GBRomPath);
        const bool sgb2_avail = S9xSGBBIOSAvailable(2, Settings.GBRomPath);
        enable_widget("bios_sgb1_item", sgb1_avail);
        enable_widget("bios_sgb2_item", sgb2_avail);
        uint8_t active = 0;
        if (Settings.SGB_BIOSModeActive)
            active = (Settings.GameBoyRunMode == 2) ? 2 : 1;
        const char *names[3] = { "bios_none_item", "bios_sgb1_item", "bios_sgb2_item" };
        refreshing_bios_menu = true;
        get_object<Gtk::RadioMenuItem>(names[active])->set_active(true);
        refreshing_bios_menu = false;
    }

    // Koei Voicer-kun audio CD: only offered while a supported game is loaded,
    // mirroring win32. Attach/eject toggle on whether a disc is mounted.
    const bool voicekun_supported = config->rom_loaded && S9xVoiceKunGameSupported();
    show_widget("voicekun_item", voicekun_supported);
    if (voicekun_supported)
    {
        const bool attached = S9xVoiceKunAttached();
        enable_widget("voicekun_attach_item", !attached);
        enable_widget("voicekun_detach_item", attached);
    }

    if (config->default_esc_behavior != ESC_TOGGLE_MENUBAR)
    {
        enable_widget("fullscreen_item", config->rom_loaded);

        config->ui_visible = true;

        show_widget("menubar", !config->fullscreen);
        show_widget("hide_ui", false);
        show_widget("hide_ui_separator", false);
    }
    else
    {
        enable_widget("fullscreen_item", true);

        show_widget("hide_ui", true);
        show_widget("hide_ui_separator", true);
        show_widget("menubar", config->ui_visible);
    }

    propagate_pause_state();

    if (config->rom_loaded && !Settings.Paused)
        hide_mouse_cursor();
    else
        show_mouse_cursor();

    if (config->rom_loaded)
    {
        std::string title = S9xBasenameNoExt(Memory.ROMFilename);
        title += " - SuperSnes9x " VERSION_DISPLAY;
        window->set_title(title);
    }
    else
    {
        window->set_title("SuperSnes9x " VERSION_DISPLAY);
    }
}

void Snes9xWindow::set_mouseable_area(int x, int y, int width, int height)
{
    mouse_region_x = x;
    mouse_region_y = y;
    mouse_region_width = width;
    mouse_region_height = height;
}

void Snes9xWindow::reset_screensaver()
{
    if (!focused)
        return;

    GdkDisplay *gdk_display = window->get_display()->gobj();

#ifdef GDK_WINDOWING_X11
    if (is_x11())
    {
        XResetScreenSaver(GDK_DISPLAY_XDISPLAY(gdk_display));
    }
#endif

    config->screensaver_needs_reset = false;
}

void Snes9xWindow::toggle_fullscreen_mode()
{
    if (config->fullscreen)
        leave_fullscreen_mode();
    else
        enter_fullscreen_mode();
}

static double XRRGetExactRefreshRate(Display *dpy, Window window)
{
    XRRScreenResources *resources = nullptr;
    XRRCrtcInfo *crtc_info = nullptr;
    int event_base;
    int error_base;
    int version_major;
    int version_minor;
    double refresh_rate = 0.0;

    if (!XRRQueryExtension(dpy, &event_base, &error_base) ||
        !XRRQueryVersion(dpy, &version_major, &version_minor))
    {
        return refresh_rate;
    }

    if (version_minor < 3)
        return refresh_rate;

    resources = XRRGetScreenResourcesCurrent(dpy, window);
    crtc_info = XRRGetCrtcInfo(dpy, resources, resources->crtcs[0]);

    for (int i = 0; i < resources->nmode; i++)
    {
        if (resources->modes[i].id == crtc_info->mode)
        {
            XRRModeInfo *m = &resources->modes[i];

            refresh_rate = (double)m->dotClock / m->hTotal / m->vTotal;
            refresh_rate /= m->modeFlags & RR_DoubleScan ? 2 : 1;
            refresh_rate /= m->modeFlags & RR_ClockDivideBy2 ? 2 : 1;
            refresh_rate *= m->modeFlags & RR_DoubleClock ? 2 : 1;

            break;
        }
    }

    XRRFreeCrtcInfo(crtc_info);
    XRRFreeScreenResources(resources);

    return refresh_rate;
}

double
Snes9xWindow::get_refresh_rate()
{
    double refresh_rate = 0.0;
#if defined GDK_WINDOWING_X11 || defined GDK_WINDOWING_WAYLAND
    GdkDisplay *gdk_display = window->get_display()->gobj();
    GdkWindow *gdk_window = window->get_window()->gobj();
#endif

#ifdef GDK_WINDOWING_X11
    if (is_x11())
    {
        Window xid = gdk_x11_window_get_xid(gdk_window);
        Display *dpy = gdk_x11_display_get_xdisplay(gdk_display);
        refresh_rate = XRRGetExactRefreshRate(dpy, xid);
    }
#endif

#ifdef GDK_WINDOWING_WAYLAND
    if (is_wayland())
    {
        GdkMonitor *monitor = gdk_display_get_monitor_at_window(gdk_display, gdk_window);
        refresh_rate = (double)gdk_monitor_get_refresh_rate(monitor) / 1000.0;
    }
#endif

    if (refresh_rate < 10.0)
    {
        printf("Warning: Couldn't read refresh rate.\n");
        refresh_rate = 60.0;
    }

    return refresh_rate;
}

int Snes9xWindow::get_auto_input_rate()
{
    double refresh_rate = get_refresh_rate();

    if (refresh_rate == 0.0)
        return 0;

    // Try for a close multiple of 60hz
    if (refresh_rate > 119.0 && refresh_rate < 121.0)
        refresh_rate /= 2.0;
    if (refresh_rate > 179.0 && refresh_rate < 181.0)
        refresh_rate /= 3.0;
    if (refresh_rate > 239.0 && refresh_rate < 241.0)
        refresh_rate /= 4.0;

    double new_input_rate = refresh_rate * 32040.0 / 60.09881389744051 + 0.5;

    if (new_input_rate > 32040.0 * 1.05 || new_input_rate < 32040.0 * 0.95)
        new_input_rate = 32040.0;

    return static_cast<int>(new_input_rate);
}


void Snes9xWindow::set_bypass_compositor(bool bypass)
{
#ifdef GDK_WINDOWING_X11
    auto gdk_window = window->get_window()->gobj();
    if (is_x11() && config->default_esc_behavior != ESC_TOGGLE_MENUBAR)
    {
        auto gdk_display = window->get_display()->gobj();
        Display *dpy = gdk_x11_display_get_xdisplay(gdk_display);
        Window window = gdk_x11_window_get_xid(gdk_window);
        uint32 value = bypass;
        Atom net_wm_bypass_compositor = XInternAtom(dpy, "_NET_WM_BYPASS_COMPOSITOR", False);
        XChangeProperty(dpy, window, net_wm_bypass_compositor, XA_CARDINAL, 32, PropModeReplace, (const unsigned char *)&value, 1);
    }
#endif
}

void Snes9xWindow::set_custom_video_mode(bool enable)
{
#ifdef GDK_WINDOWING_X11
    GdkDisplay *gdk_display = window->get_display()->gobj();

    if (!is_x11())
        return;

    Display *dpy = gdk_x11_display_get_xdisplay(gdk_display);

    RRMode id = 0;
    if (enable)
        id = config->xrr_screen_resources->modes[config->xrr_index].id;
    else
        id = config->xrr_crtc_info->mode;

    if (XRRSetCrtcConfig(dpy,
                         config->xrr_screen_resources,
                         config->xrr_screen_resources->crtcs[0],
                         CurrentTime,
                         config->xrr_crtc_info->x,
                         config->xrr_crtc_info->y,
                         id,
                         config->xrr_crtc_info->rotation,
                         &config->xrr_crtc_info->outputs[0],
                         1) != 0)
    {
        config->change_display_resolution = false;
    }

    if (gui_config->auto_input_rate)
    {
        Settings.SoundInputRate = top_level->get_auto_input_rate();
        S9xUpdateDynamicRate(1, 2);
    }
#endif
}

void Snes9xWindow::enter_fullscreen_mode()
{
    bool rom_loaded = config->rom_loaded;

    if (config->fullscreen)
        return;


    config->rom_loaded = false;
    config->fullscreen = true;

    if (config->change_display_resolution)
        set_custom_video_mode(true);

    /* Make sure everything is done synchronously */
    GdkDisplay *gdk_display = window->get_display()->gobj();
    gdk_display_sync(gdk_display);
    window->fullscreen();
    gdk_display_sync(gdk_display);
    window->present();

    if (config->auto_vrr)
    {
        autovrr_saved_frameskip = Settings.SkipFrames;
        autovrr_saved_sound_input_rate = Settings.SoundInputRate;
        autovrr_saved_sync_to_vblank = gui_config->sync_to_vblank;
        autovrr_saved_sound_sync = Settings.SoundSync;

        Settings.SoundSync = false;
        Settings.SkipFrames = THROTTLE_TIMER;
        Settings.SoundInputRate = 32040;
        S9xUpdateDynamicRate(1, 2);
        gui_config->sync_to_vblank = true;
        S9xDisplayRefresh();
    }

    set_bypass_compositor(true);
    config->rom_loaded = rom_loaded;

    /* If we're running a game, disable ui when entering fullscreen */
    if (!Settings.Paused && config->rom_loaded)
        config->ui_visible = false;

    configure_widgets();
}

void Snes9xWindow::leave_fullscreen_mode()
{
    bool rom_loaded = config->rom_loaded;

    if (!config->fullscreen)
        return;

    if (config->auto_vrr)
    {
        Settings.SkipFrames = autovrr_saved_frameskip;
        Settings.SoundInputRate = autovrr_saved_sound_input_rate;
        gui_config->sync_to_vblank = autovrr_saved_sync_to_vblank;
        Settings.SoundSync = autovrr_saved_sound_sync;
        S9xUpdateDynamicRate(1, 2);
        S9xDisplayRefresh();
    }

    config->rom_loaded = false;

    if (config->change_display_resolution)
        set_custom_video_mode(false);

    // If the window is covered by a subsurface, for some reason Gtk doesn't
    // send any resize events or do anything to resize the window. So shrink
    // the subsurface's viewport to 2x2 temporarily.
    auto driver = S9xDisplayGetDriver();
    if (driver)
        driver->shrink();

    window->unfullscreen();
    window->show();

    set_bypass_compositor(false);

    config->rom_loaded = rom_loaded;
    config->fullscreen = false;
    configure_widgets();
    window->show();

    if (driver)
        driver->regrow();
}

void Snes9xWindow::resize_viewport(int width, int height)
{
    auto menubar = get_object<Gtk::MenuBar>("menubar");
    if (menubar->get_visible())
        height += menubar->get_height();

    resize(width, height);
}

void Snes9xWindow::hide_mouse_cursor()
{
    auto blank = Gdk::Cursor::create(Gdk::BLANK_CURSOR);
    drawing_area->get_window()->set_cursor(blank);
    config->pointer_is_visible = false;
}

void Snes9xWindow::show_mouse_cursor()
{
    auto left_ptr = Gdk::Cursor::create(Gdk::LEFT_PTR);
    drawing_area->get_window()->set_cursor(left_ptr);
    config->pointer_is_visible = true;
}

void Snes9xWindow::center_mouse()
{
    int x;
    int y;

    window->get_window()->get_origin(x, y);
    int half_w = window->get_width() / 2;
    int half_h = window->get_height() / 2;

    gdk_mouse_x = x + half_w;
    gdk_mouse_y = y + half_h;

    window->get_display()->get_default_seat()->get_pointer()->warp(window->get_screen(), gdk_mouse_x, gdk_mouse_y);
}

void Snes9xWindow::toggle_grab_mouse()
{
    if ((!mouse_grabbed && !S9xIsMousePluggedIn()) || !config->rom_loaded)
        return;

    auto seat = window->get_display()->get_default_seat();
    if (!mouse_grabbed)
        seat->grab(window->get_window(), Gdk::SEAT_CAPABILITY_ALL_POINTING, true, Gdk::Cursor::create(Gdk::BLANK_CURSOR));
    else
        seat->ungrab();

    S9xReportPointer(BINDING_MOUSE_POINTER, 0, 0);
    snes_mouse_x = 0.0;
    snes_mouse_y = 0.0;
    mouse_grabbed = !mouse_grabbed;
    if (mouse_grabbed)
        center_mouse();
}

void Snes9xWindow::show()
{
    window->show();
    configure_widgets();

    if (!recent_menu)
    {
        auto manager = Gtk::RecentManager::get_default();
        recent_menu = new Gtk::RecentChooserMenu(manager);

        auto filter = Gtk::RecentFilter::create();
        filter->add_group("cartridge");
        recent_menu->add_filter(filter);
        recent_menu->set_local_only();
        recent_menu->set_show_icons(false);
        recent_menu->set_sort_type(Gtk::RECENT_SORT_MRU);
        get_object<Gtk::MenuItem>("open_recent_item")->set_submenu(*recent_menu);
        recent_menu->signal_item_activated().connect([&] {
            try_open_rom(Glib::filename_from_uri(recent_menu->get_current_uri()));
        });

        recent_menu->show();

        auto clear_recent = get_object<Gtk::MenuItem>("clear_recent_items");
        clear_recent->signal_activate().connect([] {
            auto manager = Gtk::RecentManager::get_default();
            auto items = manager->get_items();
            for (auto &i : items)
            {
                auto groups = i->get_groups();
                if (groups.end() != std::find_if(groups.begin(), groups.end(), [](Glib::ustring &name) -> bool { return name == "cartridge"; }))
                    manager->remove_item(i->get_uri());
            }
        });
    }
}

void Snes9xWindow::propagate_pause_state()
{
    int oldpause = Settings.Paused;

    Settings.Paused = (sys_pause || user_pause || !(config->rom_loaded));

    if (Settings.Paused != oldpause)
    {
        if (!is_paused())
        {
            S9xSoundStart();
            if (config->rom_loaded)
                enable_widget("pause_item", true);
        }
        else
        {
            S9xSoundStop();
            enable_widget("pause_item", false);
        }

        configure_widgets();
    }
}

void Snes9xWindow::toggle_ui()
{
    config->ui_visible = !config->ui_visible;
    configure_widgets();
}

/* gui_[un]pause Handles when system needs to pause the emulator */
void Snes9xWindow::pause_from_focus_change()
{
    sys_pause += config->modal_dialogs;
    propagate_pause_state();
}

void Snes9xWindow::unpause_from_focus_change()
{
    if (--sys_pause < 0)
        sys_pause = 0;
    propagate_pause_state();
}

/* client_[un]pause Handles when user manually chooses to pause */
void Snes9xWindow::pause_from_user()
{
    user_pause = true;
    propagate_pause_state();
}

void Snes9xWindow::unpause_from_user()
{
    user_pause = false;
    propagate_pause_state();
}

bool Snes9xWindow::is_paused()
{
    if (user_pause || sys_pause || Settings.Paused || !(config->rom_loaded))
        return true;

    return false;
}

void Snes9xWindow::set_accelerator_to_binding(Gtk::MenuItem *item, const char *binding)
{
    Binding bin;

    if (!strcmp(binding, "Escape Key"))
    {
        bin = Binding(GDK_Escape, false, false, false);
    }
    else
    {
        bin = S9xGetBindingByName(binding);
    }

    if (!(bin.is_key()))
        return;

    AcceleratorEntry entry{};
    entry.item = item;
    entry.key = bin.get_key();
    entry.modifiers = bin.get_gdk_modifiers();

    item->add_accelerator("activate", accel_group, entry.key, entry.modifiers, Gtk::ACCEL_VISIBLE);
    accelerators.push_back(entry);
}

void Snes9xWindow::set_accelerator_to_binding(const char *name, const char *binding)
{
    set_accelerator_to_binding(get_object<Gtk::MenuItem>(name).get(), binding);
}

/* Populates File->Save State and File->Load State with a submenu per bank,
 * each holding one item per slot, mirroring the win32 layout. */
void Snes9xWindow::build_state_menus()
{
    auto save_menu = get_object<Gtk::Menu>("save_state_item_menu");
    auto load_menu = get_object<Gtk::Menu>("load_state_item_menu");

    for (int bank = 0; bank < NUM_SAVE_BANKS; bank++)
    {
        auto bank_label = fmt::format(fmt::runtime(_("Bank #_{}")), bank);

        auto save_bank_item = Gtk::manage(new Gtk::MenuItem(bank_label, true));
        auto load_bank_item = Gtk::manage(new Gtk::MenuItem(bank_label, true));
        auto save_bank_menu = Gtk::manage(new Gtk::Menu());
        auto load_bank_menu = Gtk::manage(new Gtk::Menu());

        for (int slot = 0; slot < SAVE_SLOTS_PER_BANK; slot++)
        {
            int index = bank * SAVE_SLOTS_PER_BANK + slot;
            auto slot_label = fmt::format(fmt::runtime(_("Slot #_{}")), slot);

            auto save_item = Gtk::manage(new Gtk::MenuItem(slot_label, true));
            save_item->signal_activate().connect([index] {
                S9xQuickSaveSlot(index);
            });
            save_bank_menu->append(*save_item);
            save_state_items[index] = save_item;

            auto load_item = Gtk::manage(new Gtk::MenuItem(slot_label, true));
            load_item->signal_activate().connect([index] {
#ifdef RETROACHIEVEMENTS_SUPPORT
                if (!RA_WarnDisableHardcore(_("Loading save states")))
                    return;
#endif
                S9xQuickLoadSlot(index);
            });
            load_bank_menu->append(*load_item);
            load_state_items[index] = load_item;
        }

        save_bank_item->set_submenu(*save_bank_menu);
        load_bank_item->set_submenu(*load_bank_menu);

        /* Insert ahead of the separator and the "…File…" items from the .ui. */
        save_menu->insert(*save_bank_item, bank);
        load_menu->insert(*load_bank_item, bank);
    }

    save_menu->show_all();
    load_menu->show_all();
}

void Snes9xWindow::update_accelerators()
{
    if (!accel_group)
    {
        accel_group = Gtk::AccelGroup::create();
        window->add_accel_group(accel_group);
    }

    for (auto &entry : accelerators)
    {
        entry.item->remove_accelerator(accel_group, entry.key, entry.modifiers);
    }
    accelerators.clear();

    std::initializer_list<const char *[2]> pairs  =
    {
        { "fullscreen_item", "GTK_fullscreen" },
        { "reset_item", "SoftReset" },
        { "pause_item", "GTK_pause" },
        { "save_state_preview_item", "GTK_state_dialog_save" },
        { "load_state_preview_item", "GTK_state_dialog_load" },
        { "from_file1", "GTK_state_file_load" },
        { "to_file1", "GTK_state_file_save" },
        { "save_spc_item", "GTK_save_spc" },
        { "open_rom_item", "GTK_open_rom" },
        { "record_movie_item", "BeginRecordingMovie" },
        { "open_movie_item", "LoadMovie" },
        { "stop_recording_item", "EndRecordingMovie" },
        { "jump_to_frame_item", "GTK_seek_to_frame" },
        { "reset_item", "SoftReset" },
        { "hard_reset_item", "Reset" },
        { "exit_item", "GTK_quit" },
        // Special UI assignment
        { "hide_ui", "Escape Key" }
    };

    for (auto &[accelerator, binding_name] : pairs) {
        set_accelerator_to_binding(accelerator, binding_name);
    }

    /* The numbered quick save/load hotkeys act on the current bank, so show
     * them on that bank's slot items only. */
    for (int slot = 0; slot < SAVE_SLOTS_PER_BANK; slot++)
    {
        int index = config->current_save_bank * SAVE_SLOTS_PER_BANK + slot;
        auto save_binding = fmt::format("QuickSave{:03d}", slot);
        auto load_binding = fmt::format("QuickLoad{:03d}", slot);

        set_accelerator_to_binding(save_state_items[index], save_binding.c_str());
        set_accelerator_to_binding(load_state_items[index], load_binding.c_str());
    }
}

void Snes9xWindow::resize_to_multiple(int factor)
{
    int h = (config->overscan ? 239 : 224) * factor;
    int w = h * S9xGetAspect() + 0.5;

    resize_viewport(w, h);
}

cairo_t *Snes9xWindow::get_cairo()
{
    if (cr)
        return cr;

    auto allocation = drawing_area->get_allocation();

    Cairo::RectangleInt rect = { 0, 0, allocation.get_width(), allocation.get_height() };
    gdk_drawing_context = drawing_area->get_window()->begin_draw_frame(Cairo::Region::create(rect));
    cr = gdk_drawing_context->get_cairo_context()->cobj();

    cairo_owned = true;
    return cr;
}

void Snes9xWindow::release_cairo()
{
    if (cairo_owned)
    {
        drawing_area->get_window()->end_draw_frame(gdk_drawing_context);
        gdk_drawing_context.clear();
        cairo_owned = false;
        cr = nullptr;
    }
}
