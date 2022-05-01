/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <stdio.h>
#include <signal.h>
#include "giomm/application.h"
#include "glibmm/main.h"
#include "gtk_compat.h"
#include "gtk_config.h"
#include "gtk_s9x.h"
#include "gtk_control.h"
#include "gtk_sound.h"
#include "gtk_display.h"
#include "gtk_netplay.h"
#include "statemanager.h"
#include "background_particles.h"
#include "snes9x.h"
#include "display.h"
#include "apu/apu.h"
#include "netplay.h"
#include "movie.h"
#include "controls.h"
#include "snapshot.h"
#include "gfx.h"
#include "memmap.h"
#include "ppu.h"

static void S9xThrottle(int);
static void S9xCheckPointerTimer();
static bool S9xIdleFunc();
static bool S9xPauseFunc();
static bool S9xScreenSaverCheckFunc();

Snes9xWindow *top_level = nullptr;
Snes9xConfig *gui_config = nullptr;
StateManager state_manager;
gint64 frame_clock = -1;
gint64 pointer_timestamp = -1;

Background::Particles particles(Background::Particles::Snow);

static void S9xTerm(int signal)
{
    S9xExit();
}

int main(int argc, char *argv[])
{
    auto app = Gtk::Application::create("com.snes9x.gtk", Gio::APPLICATION_NON_UNIQUE);

    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, SNES9XLOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    struct sigaction sig_callback{};
    sig_callback.sa_handler = S9xTerm;
    sigaction(15, &sig_callback, NULL); // SIGTERM
    sigaction(3, &sig_callback, NULL);  // SIGQUIT
    sigaction(2, &sig_callback, NULL);  // SIGINT

    Settings = {};

    // Original config fills out values this port doesn't.
    S9xLoadConfigFiles(argv, argc);

    gui_config = new Snes9xConfig();
    gui_config->sound_drivers = S9xGetSoundDriverNames();

    S9xInitInputDevices();

    gui_config->load_config_file();

    char *rom_filename = S9xParseArgs(argv, argc);

    auto settings = Gtk::Settings::get_default();
    settings->set_property("gtk-menu-images", gui_config->enable_icons);
    settings->set_property("gtk-button-images", gui_config->enable_icons);

    if (!Memory.Init() || !S9xInitAPU())
        exit(3);

    top_level = new Snes9xWindow(gui_config);

    // Setting fullscreen before showing the window avoids some flicker.
    if ((gui_config->full_screen_on_open && rom_filename) || (gui_config->fullscreen))
        top_level->window->fullscreen();
    top_level->show();

    S9xInitDisplay(argc, argv);

    S9xPortSoundInit();

    S9xReportControllers();
    for (int port = 0; port < 2; port++)
    {
        enum controllers type;
        int8 id;
        S9xGetController(port, &type, &id, &id, &id, &id);
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

        device_type += std::to_string(port + 1);
        top_level->set_menu_item_selected(device_type.c_str());
    }

    gui_config->rebind_keys();
    top_level->update_accelerators();

    Settings.Paused = true;
    S9xNoROMLoaded();

    if (rom_filename)
    {
        if (S9xOpenROM(rom_filename) && gui_config->full_screen_on_open)
            top_level->window->unfullscreen();
    }

    // Perform the complete fullscreen process, including mode sets, which
    // didn't happen in the earlier Gtk call.
    if (gui_config->fullscreen)
        top_level->enter_fullscreen_mode();

    if (rom_filename && *Settings.InitialSnapshotFilename)
        S9xUnfreezeGame(Settings.InitialSnapshotFilename);

    gui_config->joysticks.flush_events();

    Glib::signal_timeout().connect(sigc::ptr_fun(S9xPauseFunc), 100);
    Glib::signal_timeout().connect(sigc::ptr_fun(S9xScreenSaverCheckFunc), 10000);
    app->run(*top_level->window.get());
    return 0;
}

int S9xOpenROM(const char *rom_filename)
{
    uint32 flags;
    bool loaded;

    if (gui_config->rom_loaded)
    {
        S9xAutoSaveSRAM();
    }

    S9xNetplayDisconnect();

    flags = CPU.Flags;

    loaded = false;

    if (Settings.Multi)
        loaded = Memory.LoadMultiCart(Settings.CartAName, Settings.CartBName);
    else if (rom_filename)
        loaded = Memory.LoadROM(rom_filename);

    Settings.StopEmulation = !loaded;

    if (loaded)
    {
        Memory.LoadSRAM(S9xGetFilename(".srm", SRAM_DIR));
    }
    else
    {
        CPU.Flags = flags;
        Settings.Paused = true;

        S9xNoROMLoaded();
        top_level->refresh();

        return 1;
    }

    CPU.Flags = flags;

    if (state_manager.init(gui_config->rewind_buffer_size * 1024 * 1024))
    {
        printf("Using rewind buffer of %uMB\n", gui_config->rewind_buffer_size);
    }

    S9xROMLoaded();

    return 0;
}

void S9xROMLoaded()
{
    gui_config->rom_loaded = true;
    top_level->configure_widgets();

    if (gui_config->full_screen_on_open)
    {
        Settings.Paused = false;
        top_level->enter_fullscreen_mode();
    }

    S9xSoundStart();
}

void S9xNoROMLoaded()
{
    S9xSoundStop();
    gui_config->rom_loaded = false;
    S9xDisplayRefresh(-1, -1);
    top_level->configure_widgets();
}

static bool S9xPauseFunc()
{
    S9xProcessEvents(true);

    if (!S9xNetplayPush())
    {
        S9xNetplayPop();
    }

    if (!Settings.Paused) /* Coming out of pause */
    {
        /* Clear joystick queues */
        gui_config->joysticks.flush_events();

        S9xSoundStart();

        if (Settings.NetPlay && NetPlay.Connected)
        {
            S9xNPSendPause(false);
        }

        /* Resume high-performance callback */
        Glib::signal_idle().connect(sigc::ptr_fun(S9xIdleFunc));

        return false;
    }

    if (!gui_config->rom_loaded)
    {
        if (gui_config->splash_image >= SPLASH_IMAGE_STARFIELD)
        {
            if (gui_config->splash_image == SPLASH_IMAGE_STARFIELD)
                particles.setmode(Background::Particles::Stars);
            else
                particles.setmode(Background::Particles::Snow);

            S9xThrottle(THROTTLE_TIMER);
            particles.advance();
            particles.copyto(GFX.Screen, GFX.Pitch);
            S9xDeinitUpdate(256, 224);
        }
    }

    Glib::signal_timeout().connect(sigc::ptr_fun(S9xPauseFunc), 8);

    return false;
}

static bool S9xIdleFunc()
{
    if (Settings.Paused && gui_config->rom_loaded)
    {
        S9xSoundStop();

        gui_config->joysticks.flush_events();

        if (Settings.NetPlay && NetPlay.Connected)
        {
            S9xNPSendPause(true);
        }

        /* Move to a timer-based function to use less CPU */
        Glib::signal_timeout().connect(sigc::ptr_fun(S9xPauseFunc), 8);
        return false;
    }

    S9xCheckPointerTimer();

    S9xProcessEvents(true);

    S9xThrottle(Settings.SkipFrames);

    if (!S9xDisplayDriverIsReady())
    {
        usleep(100);
        return true;
    }

    if (!S9xNetplayPush())
    {
        if (Settings.Rewinding)
        {
            uint16 joypads[8];
            for (int i = 0; i < 8; i++)
                joypads[i] = MovieGetJoypad(i);

            Settings.Rewinding = state_manager.pop();

            for (int i = 0; i < 8; i++)
                MovieSetJoypad(i, joypads[i]);
        }
        else if (IPPU.TotalEmulatedFrames % gui_config->rewind_granularity == 0)
            state_manager.push();

        if ((Settings.TurboMode || Settings.Rewinding) && gui_config->mute_sound_turbo)
            Settings.Mute |= 0x80;
        else
            Settings.Mute &= ~0x80;

        S9xMainLoop();

        S9xNetplayPop();
    }

    return true;
}

static bool S9xScreenSaverCheckFunc()
{

    if (!Settings.Paused &&
        (gui_config->screensaver_needs_reset ||
         gui_config->prevent_screensaver))
        top_level->reset_screensaver();

    return true;
}

/* Snes9x core hooks */
void S9xMessage(int type, int number, const char *message)
{
    switch (number)
    {
    case S9X_MOVIE_INFO:
        S9xSetInfoString(message);
        break;
    default:
        break;
    }
}

/* Varies from ParseArgs because this one is for the OS port to handle */
void S9xParseArg(char **argv, int &i, int argc)
{
    if (!strcasecmp(argv[i], "-filter"))
    {
        std::string filter_names[] =
        {
            "none",
            "supereagle",
            "2xsai",
            "super2xsai",
            "epx",
            "epx_smooth",
            "ntsc",
            "scanlines",
            "simple2x",
            "simple3x",
            "simple4x",
            "hq2x",
            "hq3x",
            "hq4x",
            "2xbrz",
            "3xbrz",
            "4xbrz"
        };

        if ((++i) < argc)
        {
            for (int f = 0; f < NUM_FILTERS; f++)
                if (filter_names[f] == argv[i])
                    gui_config->scale_method = f;
        }
    }
    else if (!strcasecmp(argv[i], "-mutesound"))
    {
        gui_config->mute_sound = true;
    }
}

static void S9xThrottle(int method)
{
    gint64 now;

    if (S9xNetplaySyncSpeed())
        return;

    now = g_get_monotonic_time();

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
        if ((IPPU.FrameSkip >= Settings.TurboSkipFrames) && !Settings.HighSpeedSeek)
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

    if (method == THROTTLE_SOUND_SYNC ||
        method == THROTTLE_NONE)
    {
        frame_clock = now;
        IPPU.SkippedFrames = 0;
    }
    else // THROTTLE_TIMER or THROTTLE_TIMER_FRAMESKIP
    {
        if (method == THROTTLE_TIMER_FRAMESKIP)
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
            usleep(100);
            now = g_get_monotonic_time();
        }

        frame_clock += Settings.FrameTime;
        IPPU.FrameSkip = 0;
        IPPU.SkippedFrames = 0;
    }
}

void S9xSyncSpeed()
{
}

static void S9xCheckPointerTimer()
{
    if (!gui_config->pointer_is_visible)
        return;

    if (g_get_monotonic_time() - gui_config->pointer_timestamp > 1000000)
    {
        top_level->hide_mouse_cursor();
        gui_config->pointer_is_visible = false;
    }
}

/* Final exit point, issues exit (0) */
void S9xExit()
{
    gui_config->save_config_file();

    top_level->leave_fullscreen_mode();

    S9xPortSoundDeinit();

    Settings.StopEmulation = true;

    if (gui_config->rom_loaded)
    {
        S9xAutoSaveSRAM();
    }

    S9xDeinitAPU();

    S9xDeinitInputDevices();
    S9xDeinitDisplay();

    delete top_level;
    delete gui_config;

    exit(0);
}

const char *S9xStringInput(const char *message)
{
    return NULL;
}

void S9xExtraUsage()
{
    printf("GTK port options:\n"
           "-filter [option]               Use a filter to scale the image.\n"
           "                               [option] is one of: none supereagle 2xsai\n"
           "                               super2xsai hq2x hq3x hq4x 2xbrz 3xbrz 4xbrz epx ntsc\n"
           "\n"
           "-mutesound                     Disables sound output.\n");
}
