#include "gtk_netplay_dialog.h"
#include "gtk_s9x.h"
#include "gtk_file.h"

static void
event_browse_clicked (GtkButton *button, gpointer data)
{
    char                *filename;
    Snes9xNetplayDialog *np_dialog = (Snes9xNetplayDialog *) data;

    filename = S9xOpenROMDialog ();

    if (filename)
    {
        gtk_entry_set_text (GTK_ENTRY (np_dialog->get_widget ("rom_image")),
                            filename);

        g_free (filename);
    }

    return;
}

static void
event_clear_clicked (GtkButton *button, gpointer data)
{
    Snes9xNetplayDialog *np_dialog = (Snes9xNetplayDialog *) data;

    gtk_entry_set_text (GTK_ENTRY (np_dialog->get_widget ("rom_image")), "");

    return;
}

static void
event_server_toggled (GtkToggleButton *toggle, gpointer data)
{
    Snes9xNetplayDialog *np_dialog = (Snes9xNetplayDialog *) data;

    np_dialog->update_state ();
}

Snes9xNetplayDialog::Snes9xNetplayDialog (Snes9xConfig *config) :
    GtkBuilderWindow ("netplay_dialog")
{
    GtkBuilderWindowCallbacks callbacks[] =
    {
        { "server_toggled", G_CALLBACK (event_server_toggled) },
        { "browse_clicked", G_CALLBACK (event_browse_clicked) },
        { "clear_clicked", G_CALLBACK (event_clear_clicked) },
        { NULL, NULL }
    };

    signal_connect (callbacks);

    this->config = config;

    return;
}

void
Snes9xNetplayDialog::update_state (void)
{
    if (get_check ("host_radio"))
    {
        enable_widget ("connect_box", FALSE);
        enable_widget ("default_port_box", TRUE);
        enable_widget ("sync_reset", TRUE);
        enable_widget ("send_image", TRUE);
    }

    else
    {
        enable_widget ("connect_box", TRUE);
        enable_widget ("default_port_box", FALSE);
        enable_widget ("sync_reset", FALSE);
        enable_widget ("send_image", FALSE);
    }

    return;
}

void
Snes9xNetplayDialog::settings_to_dialog (void)
{
    set_entry_text ("rom_image", config->netplay_last_rom);
    set_entry_text ("ip_entry", config->netplay_last_host);
    set_check ("sync_reset", config->netplay_sync_reset);
    set_check ("send_image", config->netplay_send_rom);
    set_spin ("port", config->netplay_last_port);
    set_spin ("default_port", config->netplay_default_port);
    set_spin ("frames_behind", config->netplay_max_frame_loss);
    set_check ("connect_radio", !config->netplay_is_server);
    set_check ("host_radio", config->netplay_is_server);

    update_state ();

    return;
}

void
Snes9xNetplayDialog::settings_from_dialog (void)
{
    strncpy (config->netplay_last_rom, get_entry_text ("rom_image"), PATH_MAX);
    strncpy (config->netplay_last_host, get_entry_text ("ip_entry"), PATH_MAX);
    config->netplay_sync_reset = get_check ("sync_reset");
    config->netplay_send_rom = get_check ("send_image");
    config->netplay_last_port = get_spin ("port");
    config->netplay_default_port = get_spin ("default_port");
    config->netplay_max_frame_loss = get_spin ("frames_behind");
    config->netplay_is_server = get_check ("host_radio");

    config->save_config_file ();

    return;
}

int
Snes9xNetplayDialog::show (void)
{
    int result;

    settings_to_dialog ();

    result = gtk_dialog_run (GTK_DIALOG (window));

    gtk_widget_hide (window);

    if (result == GTK_RESPONSE_OK)
    {
        settings_from_dialog ();

        return TRUE;
    }

    else
    {
        return FALSE;
    }

}

Snes9xNetplayDialog::~Snes9xNetplayDialog (void)
{
    gtk_widget_destroy (window);
}
