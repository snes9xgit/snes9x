/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_netplay_dialog.h"
#include "gtk_s9x.h"

Snes9xNetplayDialog::Snes9xNetplayDialog(Snes9xConfig *config)
    : GtkBuilderWindow("netplay_dialog")
{
    get_object<Gtk::RadioButton>("host_radio")->signal_toggled().connect([&] {
        update_state();
    });

    get_object<Gtk::Button>("clear_netplay")->signal_clicked().connect([&] {
        get_object<Gtk::Entry>("rom_image")->set_text("");
    });

    get_object<Gtk::Button>("browse_button")->signal_clicked().connect([&] {
        auto filename = top_level->open_rom_dialog(false);
        if (!filename.empty())
            get_object<Gtk::Entry>("rom_image")->set_text(filename);
    });
    // Handle plurals on GtkLabel "frames_behind_label"
    get_object<Gtk::SpinButton>("frames_behind")->signal_value_changed().connect([&] {
        set_label("frames_behind_label",
            // GLib’s g_dngettext() would have been a better fit here but
            // xgettext does not extract g_dngettext() by default
            ngettext("frame behind", "frames behind", get_spin("frames_behind")));
    });

    this->config = config;
}

Snes9xNetplayDialog::~Snes9xNetplayDialog()
{
}

void Snes9xNetplayDialog::update_state()
{
    if (get_check("host_radio"))
    {
        enable_widget("connect_box", false);
        enable_widget("default_port_box", true);
        enable_widget("sync_reset", true);
        enable_widget("send_image", true);
    }

    else
    {
        enable_widget("connect_box", true);
        enable_widget("default_port_box", false);
        enable_widget("sync_reset", false);
        enable_widget("send_image", false);
    }
}

void Snes9xNetplayDialog::settings_to_dialog()
{
    set_entry_text("rom_image", config->netplay_last_rom.c_str());
    set_entry_text("ip_entry", config->netplay_last_host.c_str());
    set_check("sync_reset", config->netplay_sync_reset);
    set_check("send_image", config->netplay_send_rom);
    set_spin("port", config->netplay_last_port);
    set_spin("default_port", config->netplay_default_port);
    set_spin("frames_behind", config->netplay_max_frame_loss);
    set_check("connect_radio", !config->netplay_is_server);
    set_check("host_radio", config->netplay_is_server);

    update_state();
}

void Snes9xNetplayDialog::settings_from_dialog()
{
    config->netplay_last_rom = get_entry_text("rom_image");
    config->netplay_last_host = get_entry_text("ip_entry");
    config->netplay_sync_reset = get_check("sync_reset");
    config->netplay_send_rom = get_check("send_image");
    config->netplay_last_port = get_spin("port");
    config->netplay_default_port = get_spin("default_port");
    config->netplay_max_frame_loss = get_spin("frames_behind");
    config->netplay_is_server = get_check("host_radio");

    config->save_config_file();
}

bool Snes9xNetplayDialog::show()
{
    settings_to_dialog();

    auto result = Glib::RefPtr<Gtk::Dialog>::cast_static(window)->run();
    window->hide();

    if (result == Gtk::RESPONSE_OK)
    {
        settings_from_dialog();
        return true;
    }

    return false;
}
