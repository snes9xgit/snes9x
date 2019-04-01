/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <stdio.h>
#include <signal.h>
#include "gtk_2_3_compat.h"
#include "gtk_s9x.h"
#include "gtk_control.h"
#include "gtk_sound.h"
#include "gtk_display.h"
#include "gtk_netplay.h"
#include "statemanager.h"


void S9xPostRomInit ();
static void S9xThrottle ();
static void S9xCheckPointerTimer ();
static gboolean S9xIdleFunc (gpointer data);
static gboolean S9xPauseFunc (gpointer data);
static gboolean S9xScreenSaverCheckFunc (gpointer data);

Snes9xWindow *top_level;
Snes9xConfig *gui_config;
StateManager state_manager;
gint64       frame_clock = -1;
gint64       pointer_timestamp = -1;

void S9xTerm (int signal)
{
    S9xExit ();
}

int main (int argc, char *argv[])
{
    struct sigaction sig_callback;

    gtk_init (&argc, &argv);

    g_set_prgname ("snes9x-gtk");
    g_set_application_name ("Snes9x");

    setlocale (LC_ALL, "");
    bindtextdomain (GETTEXT_PACKAGE, SNES9XLOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    memset (&Settings, 0, sizeof (Settings));

    /* Allow original config file for backend settings */
    S9xLoadConfigFiles (argv, argc);

    /* Perform our config here */
    gui_config = new Snes9xConfig ();

    S9xInitInputDevices ();

    gui_config->load_config_file ();

    char *rom_filename = S9xParseArgs (argv, argc);

#if GTK_MAJOR_VERSION >= 3
    auto settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-menu-images", true, "gtk_button_images", true, NULL);
#endif

    S9xReportControllers ();

    if (!Memory.Init () || !S9xInitAPU ())
        exit (3);

    top_level = new Snes9xWindow (gui_config);

    /* If we're going to fullscreen, do it before showing window to avoid flicker. */
    if ((gui_config->full_screen_on_open && rom_filename) || (gui_config->fullscreen))
        gtk_window_fullscreen (top_level->get_window ());

    top_level->show ();

    S9xInitDisplay (argc, argv);

    Memory.PostRomInitFunc = S9xPostRomInit;

    S9xPortSoundInit ();

    for (int port = 0; port < 2; port++)
    {
        enum controllers type;
        int8 id;
        S9xGetController (port, &type, &id, &id, &id, &id);
        std::string device_type;

        switch (type)
        {
        case CTL_MP5:
            device_type = "multitap";
            break;
        case CTL_MOUSE:
            device_type = "mouse";
            break;
        case CTL_SUPERSCOPE:
            device_type = "superscope";
            break;
        case CTL_JOYPAD:
            device_type = "joypad";
            break;
        default:
            device_type = "nothingpluggedin";
        }

        device_type += std::to_string (port + 1);
        top_level->set_menu_item_selected (device_type.c_str ());
    }

    gui_config->rebind_keys ();
    top_level->update_accels ();

    Settings.Paused = true;
    g_timeout_add (100, S9xPauseFunc, NULL);
    g_timeout_add (10000, S9xScreenSaverCheckFunc, NULL);

    S9xNoROMLoaded ();

    if (rom_filename)
    {
        if (S9xOpenROM (rom_filename) && gui_config->full_screen_on_open)
            gtk_window_unfullscreen (top_level->get_window());
    }

    memset (&sig_callback, 0, sizeof (struct sigaction));
    sig_callback.sa_handler = S9xTerm;

    sigaction (15 /* SIGTERM */, &sig_callback, NULL);
    sigaction (3  /* SIGQUIT */, &sig_callback, NULL);
    sigaction (2  /* SIGINT  */, &sig_callback, NULL);

    if (gui_config->fullscreen)
    {
        top_level->enter_fullscreen_mode ();
    }

    gui_config->flush_joysticks ();

    if (rom_filename && *Settings.InitialSnapshotFilename)
        S9xUnfreezeGame(Settings.InitialSnapshotFilename);

    gtk_main ();
    return 0;
}

int S9xOpenROM (const char *rom_filename)
{
    uint32 flags;
    bool   loaded;

    if (gui_config->rom_loaded)
    {
        S9xAutoSaveSRAM ();
    }

    S9xNetplayDisconnect ();

    flags = CPU.Flags;

    loaded = false;

    if (Settings.Multi)
        loaded = Memory.LoadMultiCart (Settings.CartAName, Settings.CartBName);
    else if (rom_filename)
        loaded = Memory.LoadROM (rom_filename);

    Settings.StopEmulation = !loaded;

    if (!loaded && rom_filename)
    {
        char dir [_MAX_DIR + 1];
        char drive [_MAX_DRIVE + 1];
        char name [_MAX_FNAME + 1];
        char ext [_MAX_EXT + 1];
        char fname [_MAX_PATH + 1];

        _splitpath (rom_filename, drive, dir, name, ext);
        _makepath (fname, drive, dir, name, ext);

        strcpy (fname, S9xGetDirectory (ROM_DIR));
        strcat (fname, SLASH_STR);
        strcat (fname, name);

        if (ext [0])
        {
            strcat (fname, ".");
            strcat (fname, ext);
        }

        _splitpath (fname, drive, dir, name, ext);
        _makepath (fname, drive, dir, name, ext);

        if ((Settings.StopEmulation = !Memory.LoadROM (fname)))
        {
            fprintf (stderr, _("Error opening: %s\n"), rom_filename);

            loaded = false;
        }
        else
            loaded = true;
    }

    if (loaded)
    {
        Memory.LoadSRAM (S9xGetFilename (".srm", SRAM_DIR));
    }
    else
    {
        CPU.Flags = flags;
        Settings.Paused = true;

        S9xNoROMLoaded ();
        top_level->refresh ();

        return 1;
    }

    CPU.Flags = flags;

    if (state_manager.init (gui_config->rewind_buffer_size * 1024 * 1024))
    {
        printf ("Using rewind buffer of %uMB\n", gui_config->rewind_buffer_size);
    }

    S9xROMLoaded ();

    return 0;
}

void S9xROMLoaded ()
{
    gui_config->rom_loaded = true;
    top_level->configure_widgets ();

    if (gui_config->full_screen_on_open)
    {
        Settings.Paused = false;
        top_level->enter_fullscreen_mode ();
    }

    S9xSoundStart ();
}

void S9xNoROMLoaded ()
{
    S9xSoundStop ();
    gui_config->rom_loaded = false;
    S9xDisplayRefresh (-1, -1);
    top_level->configure_widgets ();
}

static gboolean S9xPauseFunc (gpointer data)
{
    S9xProcessEvents (true);

    if (!gui_config->rom_loaded)
        return true;

    if (!S9xNetplayPush ())
    {
        S9xNetplayPop ();
    }

    if (!Settings.Paused) /* Coming out of pause */
    {
        /* Clear joystick queues */
        gui_config->flush_joysticks ();

        S9xSetSoundMute (false);
        S9xSoundStart ();

        if (Settings.NetPlay && NetPlay.Connected)
        {
            S9xNPSendPause (false);
        }

        /* Resume high-performance callback */
        g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
                         S9xIdleFunc,
                         NULL,
                         NULL);
        return false;
    }

    return true;
}

gboolean S9xIdleFunc (gpointer data)
{
    if (Settings.Paused)
    {
        S9xSetSoundMute (gui_config->mute_sound);
        S9xSoundStop ();

        gui_config->flush_joysticks ();

        if (Settings.NetPlay && NetPlay.Connected)
        {
            S9xNPSendPause (true);
        }

        /* Move to a timer-based function to use less CPU */
        g_timeout_add (100, S9xPauseFunc, NULL);
        return false;
    }

    S9xCheckPointerTimer ();

    S9xProcessEvents (true);

    if (!S9xDisplayDriverIsReady ())
    {
        usleep(100);
        return true;
    }

    S9xThrottle ();

    if (!S9xNetplayPush ())
    {
        if(Settings.Rewinding)
        {
            uint16 joypads[8];
            for (int i = 0; i < 8; i++)
                joypads[i] = MovieGetJoypad(i);

            Settings.Rewinding = state_manager.pop();

            for (int i = 0; i < 8; i++)
                MovieSetJoypad (i, joypads[i]);
        }
        else if(IPPU.TotalEmulatedFrames % gui_config->rewind_granularity == 0)
            state_manager.push();

        static int muted_from_turbo = false;
        static int mute_saved_state = false;

        if (Settings.TurboMode && !muted_from_turbo && gui_config->mute_sound_turbo)
        {
            muted_from_turbo = true;
            mute_saved_state = Settings.Mute;
            S9xSetSoundMute (true);
        }

        if (!Settings.TurboMode && muted_from_turbo)
        {
            muted_from_turbo = false;
            Settings.Mute = mute_saved_state;
        }

        S9xMainLoop ();

        S9xNetplayPop ();
    }

    return true;
}

gboolean S9xScreenSaverCheckFunc (gpointer data)
{

    if (!Settings.Paused &&
        (gui_config->screensaver_needs_reset ||
         gui_config->prevent_screensaver))
        top_level->reset_screensaver ();

    return true;
}

/* Snes9x core hooks */
void S9xMessage (int type, int number, const char *message)
{
    switch (number)
    {
      case S9X_MOVIE_INFO:
        S9xSetInfoString (message);
        break;
      default:
        break;
    }
}

/* Varies from ParseArgs because this one is for the OS port to handle */
void S9xParseArg (char **argv, int &i, int argc)
{
    if (!strcasecmp (argv[i], "-filter"))
    {
        if ((++i) < argc)
        {
            if (!strcasecmp (argv[i], "none"))
            {
                gui_config->scale_method = FILTER_NONE;
            }
            else if (!strcasecmp (argv[i], "supereagle"))
            {
                gui_config->scale_method = FILTER_SUPEREAGLE;
            }
            else if (!strcasecmp (argv[i], "2xsai"))
            {
                gui_config->scale_method = FILTER_2XSAI;
            }
            else if (!strcasecmp (argv[i], "super2xsai"))
            {
                gui_config->scale_method = FILTER_SUPER2XSAI;
            }
#ifdef USE_HQ2X
            else if (!strcasecmp (argv[i], "hq2x"))
            {
                gui_config->scale_method = FILTER_HQ2X;
            }
            else if (!strcasecmp (argv[i], "hq3x"))
            {
                gui_config->scale_method = FILTER_HQ3X;
            }
            else if (!strcasecmp (argv[i], "hq4x"))
            {
                gui_config->scale_method = FILTER_HQ4X;
            }
#endif /* USE_HQ2X */
#ifdef USE_XBRZ
            else if (!strcasecmp (argv[i], "2xbrz"))
            {
                gui_config->scale_method = FILTER_2XBRZ;
            }
            else if (!strcasecmp (argv[i], "3xbrz"))
            {
                gui_config->scale_method = FILTER_3XBRZ;
            }
            else if (!strcasecmp (argv[i], "4xbrz"))
            {
                gui_config->scale_method = FILTER_4XBRZ;
            }
#endif /* USE_XBRZ */
            else if (!strcasecmp (argv[i], "epx"))
            {
                gui_config->scale_method = FILTER_EPX;
            }
            else if (!strcasecmp (argv[i], "ntsc"))
            {
                gui_config->scale_method = FILTER_NTSC;
            }
            else
            {
                gui_config->scale_method = FILTER_NONE;
            }
        }
    }
    else if (!strcasecmp (argv[i], "-mutesound"))
    {
        gui_config->mute_sound = true;
    }
}

static void S9xThrottle ()
{
    gint64 now;

    if (S9xNetplaySyncSpeed ())
        return;

    now = g_get_monotonic_time ();

    if (Settings.HighSpeedSeek > 0)
    {
        Settings.HighSpeedSeek--;
        IPPU.RenderThisFrame = false;
        IPPU.SkippedFrames = 0;
        frame_clock = now;

        return;
    }

    if (Settings.TurboMode)
    {
        IPPU.FrameSkip++;
        if ((IPPU.FrameSkip >= Settings.TurboSkipFrames)
            && !Settings.HighSpeedSeek)
        {
            IPPU.FrameSkip = 0;
            IPPU.SkippedFrames = 0;
            IPPU.RenderThisFrame = true;
        }
        else
        {
            IPPU.SkippedFrames++;
            IPPU.RenderThisFrame = false;
        }

        frame_clock = now;

        return;
    }

    IPPU.RenderThisFrame = true;

    if (now - frame_clock > 500000)
    {
        frame_clock = now;
    }

    if (Settings.SkipFrames == THROTTLE_SOUND_SYNC ||
        Settings.SkipFrames == THROTTLE_NONE)
    {
        frame_clock = now;
        IPPU.SkippedFrames = 0;
    }
    else // THROTTLE_TIMER or THROTTLE_TIMER_FRAMESKIP
    {
        if (Settings.SkipFrames == THROTTLE_TIMER_FRAMESKIP)
        {
            if (now - frame_clock > Settings.FrameTime)
            {
                IPPU.SkippedFrames++;

                if (IPPU.SkippedFrames < 8)
                {
                    IPPU.RenderThisFrame = false;
                    frame_clock += Settings.FrameTime;
                    return;
                }
                else
                {
                    frame_clock = now - Settings.FrameTime;
                }
            }
        }

        while (now - frame_clock < Settings.FrameTime)
        {
            usleep (100);
            now = g_get_monotonic_time ();
        }

        frame_clock += Settings.FrameTime;
        IPPU.FrameSkip = 0;
        IPPU.SkippedFrames = 0;
    }
}

void S9xSyncSpeed ()
{
}

static void S9xCheckPointerTimer ()
{
    if (!gui_config->pointer_is_visible)
        return;

    if (g_get_monotonic_time () - gui_config->pointer_timestamp > 1000000)
    {
        top_level->hide_mouse_cursor ();
        gui_config->pointer_is_visible = false;
    }
}

/* Final exit point, issues exit (0) */
void S9xExit ()
{
    gui_config->save_config_file ();

    top_level->leave_fullscreen_mode ();

    S9xPortSoundDeinit ();

    Settings.StopEmulation = true;

    if (gui_config->rom_loaded)
    {
        S9xAutoSaveSRAM ();
    }

    S9xDeinitAPU ();

    S9xDeinitInputDevices ();
    S9xDeinitDisplay ();

    gtk_main_quit ();

    delete top_level;
    delete gui_config;

    exit (0);
}

void
S9xPostRomInit ()
{
    if (!strncmp ((const char *) Memory.NSRTHeader + 24, "NSRT", 4))
    {
        switch (Memory.NSRTHeader[29])
        {
            case 0: //Everything goes
                break;

            case 0x10: //Mouse in Port 0
                S9xSetController (0, CTL_MOUSE,      0, 0, 0, 0);
                top_level->set_menu_item_selected ("mouse1");
                break;

            case 0x01: //Mouse in Port 1
                S9xSetController (1, CTL_MOUSE,      1, 0, 0, 0);
                top_level->set_menu_item_selected ("mouse2");
                break;

            case 0x03: //Super Scope in Port 1
                S9xSetController (1, CTL_SUPERSCOPE, 0, 0, 0, 0);
                top_level->set_menu_item_selected ("superscope1");
                break;

            case 0x06: //Multitap in Port 1
                S9xSetController (1, CTL_MP5,        1, 2, 3, 4);
                top_level->set_menu_item_selected ("multitap1");
                break;

            case 0x66: //Multitap in Ports 0 and 1
                S9xSetController (0, CTL_MP5,        0, 1, 2, 3);
                S9xSetController (1, CTL_MP5,        4, 5, 6, 7);
                top_level->set_menu_item_selected ("multitap1");
                top_level->set_menu_item_selected ("multitap2");
                break;

            case 0x08: //Multitap in Port 1, Mouse in new Port 1
                S9xSetController (1, CTL_MOUSE,      1, 0, 0, 0);
                //There should be a toggle here for putting in Multitap instead
                top_level->set_menu_item_selected ("mouse2");
                break;

            case 0x04: //Pad or Super Scope in Port 1
                S9xSetController (1, CTL_SUPERSCOPE, 0, 0, 0, 0);
                top_level->set_menu_item_selected ("superscope2");
                //There should be a toggle here for putting in a pad instead
                break;

            case 0x05: //Justifier - Must ask user...
                S9xSetController (1, CTL_JUSTIFIER,  1, 0, 0, 0);
                //There should be a toggle here for how many justifiers
                break;

            case 0x20: //Pad or Mouse in Port 0
                S9xSetController (0, CTL_MOUSE,      0, 0, 0, 0);
                //There should be a toggle here for putting in a pad instead
                break;

            case 0x22: //Pad or Mouse in Port 0 & 1
                S9xSetController (0, CTL_MOUSE,      0, 0, 0, 0);
                S9xSetController (1, CTL_MOUSE,      1, 0, 0, 0);
                //There should be a toggles here for putting in pads instead
                break;

            case 0x24: //Pad or Mouse in Port 0, Pad or Super Scope in Port 1
                //There should be a toggles here for what to put in, I'm leaving it at gamepad for now
                break;

            case 0x27: //Pad or Mouse in Port 0, Pad or Mouse or Super Scope in Port 1
                //There should be a toggles here for what to put in, I'm leaving it at gamepad for now
                break;

                //Not Supported yet
            case 0x99: break; //Lasabirdie
            case 0x0A: break; //Barcode Battler
        }
    }
}

const char *S9xStringInput(const char *message)
{
    return NULL;
}

void S9xExtraUsage ()
{
    printf ("GTK port options:\n"
            "-filter [option]               Use a filter to scale the image.\n"
            "                               [option] is one of: none supereagle 2xsai\n"
            "                               super2xsai hq2x hq3x hq4x 2xbrz 3xbrz 4xbrz epx ntsc\n"
            "\n"
            "-mutesound                     Disables sound output.\n");
}
