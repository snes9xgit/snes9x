/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <glib.h>

#include "gtk_s9x.h"
#include "gtk_netplay_dialog.h"
#include "gtk_netplay.h"
#include "gtk_sound.h"

uint16 MovieGetJoypad (int i);
void MovieSetJoypad (int i, uint16 buttons);

static uint32 local_joypads[8], joypads[8];
static GThread *npthread;

extern SNPServer NPServer;

static void
S9xNetplayPreconnect ()
{
    S9xNetplayDisconnect ();

    if (gui_config->rom_loaded)
    {
        S9xAutoSaveSRAM ();
    }

    NetPlay.MaxBehindFrameCount = gui_config->netplay_max_frame_loss;
    NetPlay.Waiting4EmulationThread = false;
}

static void
S9xNetplayConnect ()
{
    GtkWidget *msg;

    S9xNetplayPreconnect ();

    uint32 flags = CPU.Flags;

    if (!gui_config->netplay_last_rom.empty () &&
        !top_level->try_open_rom (gui_config->netplay_last_rom.c_str ()))
    {
        return;
    }

    if (!S9xNPConnectToServer (gui_config->netplay_last_host.c_str (),
                               gui_config->netplay_last_port,
                               Memory.ROMName))
    {
        msg = gtk_message_dialog_new (NULL,
                                      GTK_DIALOG_DESTROY_WITH_PARENT,
                                      GTK_MESSAGE_ERROR,
                                      GTK_BUTTONS_CLOSE,
                                      "Couldn't connect to server: %s:%d",
                                      gui_config->netplay_last_host.c_str (),
                                      gui_config->netplay_last_port);
        gtk_window_set_title (GTK_WINDOW (msg), _("Connection Error"));

        gtk_dialog_run (GTK_DIALOG (msg));
        gtk_widget_destroy (msg);
    }

    gui_config->netplay_activated = true;

    /* If no rom is specified, assume we'll get it from the server */
    if (gui_config->netplay_last_rom.empty ())
    {
        Settings.StopEmulation = false;
        S9xROMLoaded ();
    }

    S9xReset ();

    CPU.Flags = flags;

    top_level->configure_widgets ();
}

void
S9xNetplaySyncClients ()
{
    if (Settings.NetPlay && Settings.NetPlayServer)
        S9xNPServerQueueSyncAll ();
}

void
S9xNetplayStopServer ()
{
    S9xNPStopServer ();

    g_thread_join (npthread);
    Settings.NetPlayServer = false;
    gui_config->netplay_server_up = false;
}

void
S9xNetplayDisconnect ()
{
    if (Settings.NetPlay)
    {
        if (NetPlay.Connected)
            S9xNPDisconnect ();
    }

    if (gui_config->netplay_server_up)
    {
        S9xNetplayStopServer ();
    }

    gui_config->netplay_activated = false;
    NetPlay.Paused = false;

    top_level->configure_widgets ();
}

static gpointer
S9xNetplayServerThread (gpointer)
{
    S9xNPStartServer (gui_config->netplay_default_port);

    return NULL;
}

void
S9xNetplayStartServer ()
{
    uint32 flags;

    S9xNetplayPreconnect ();

    flags = CPU.Flags;

    if (gui_config->netplay_last_rom.empty () ||
        !top_level->try_open_rom (gui_config->netplay_last_rom.c_str ()))
    {
        return;
    }

    Settings.NetPlayServer = true;
    NPServer.SyncByReset = gui_config->netplay_sync_reset;
    NPServer.SendROMImageOnConnect = gui_config->netplay_send_rom;

    npthread = g_thread_new (NULL, S9xNetplayServerThread, NULL);

    /* Sleep to let the server create itself */
    usleep (10000);

    S9xNPConnectToServer ("127.0.0.1",
                          gui_config->netplay_default_port,
                          Memory.ROMName);

    S9xReset ();

    S9xROMLoaded ();

    gui_config->netplay_activated = true;
    gui_config->netplay_server_up = true;

    CPU.Flags = flags;

    top_level->configure_widgets ();
}

void
S9xNetplayDialogOpen ()
{
    Snes9xNetplayDialog *np_dialog;

    top_level->pause_from_focus_change ();

    np_dialog = new Snes9xNetplayDialog (gui_config);

    gtk_window_set_transient_for (np_dialog->get_window (),
                                  top_level->get_window ());

    if (np_dialog->show ())
    {
        if (!gui_config->netplay_is_server)
        {
            S9xNetplayConnect ();
        }
        else
        {
            S9xNetplayStartServer ();
        }

        S9xSoundStart ();
    }

    delete np_dialog;

    top_level->unpause_from_focus_change ();
}

int
S9xNetplaySyncSpeed ()
{
    if (!Settings.NetPlay || !NetPlay.Connected)
        return 0;

    // Send 1st joypad's position update to server
    S9xNPSendJoypadUpdate (local_joypads[0]);

    // set input from network
    for (int i = 0; i < NP_MAX_CLIENTS; i++)
        joypads[i] = S9xNPGetJoypad (i);

    if (!S9xNPCheckForHeartBeat ())
    {
        // No heartbeats already arrived, have to wait for one.
        NetPlay.PendingWait4Sync = !S9xNPWaitForHeartBeatDelay (100);

        IPPU.RenderThisFrame = true;
        IPPU.SkippedFrames = 0;
    }
    else
    {
        int difference = (int) (NetPlay.MySequenceNum) -
                         (int) (NetPlay.ServerSequenceNum);

        if (difference < 0)
            difference += 256;

        if (NetPlay.Waiting4EmulationThread)
        {
            if ((unsigned int) difference <= (NetPlay.MaxBehindFrameCount / 2))
            {
                NetPlay.Waiting4EmulationThread = false;
                S9xNPSendPause (false);
            }
        }
        else
        {
            if ((unsigned int) difference >= (NetPlay.MaxBehindFrameCount))
            {
                NetPlay.Waiting4EmulationThread = true;
                S9xNPSendPause (true);
            }
        }

        NetPlay.PendingWait4Sync = !S9xNPWaitForHeartBeatDelay (200);

        if (IPPU.SkippedFrames < NetPlay.MaxFrameSkip)
        {
            IPPU.SkippedFrames++;
            IPPU.RenderThisFrame = false;
        }
        else
        {
            IPPU.RenderThisFrame = true;
            IPPU.SkippedFrames = 0;
        }
    }

    if (!NetPlay.PendingWait4Sync)
    {
        NetPlay.FrameCount++;
        S9xNPStepJoypadHistory ();
    }

    return 1;
}

int
S9xNetplayPush ()
{
    if (gui_config->netplay_activated &&
        (!Settings.NetPlay || !NetPlay.Connected))
        S9xNetplayDisconnect ();

    if (!Settings.NetPlay)
        return 0;

    if (NetPlay.PendingWait4Sync && !S9xNPWaitForHeartBeatDelay (100))
    {
        S9xProcessEvents (false);

        S9xSoundStop ();
        NetPlay.Paused = true;

        return 1;
    }

    NetPlay.Paused = false;

    S9xSoundStart ();

    /* Save the joypad input */
    for (int i = 0; i < 8; i++)
    {
        local_joypads[i] = MovieGetJoypad (i);

        MovieSetJoypad (i, joypads[i]);
    }

    if (NetPlay.PendingWait4Sync)
    {
        NetPlay.PendingWait4Sync = false;
        NetPlay.FrameCount++;
        S9xNPStepJoypadHistory ();
    }

    return 0;
}

void
S9xNetplayPop ()
{
    if (!Settings.NetPlay)
        return;

    for (int i = 0; i < 8; i++)
        MovieSetJoypad (i, local_joypads[i]);
}




