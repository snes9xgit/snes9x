/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <errno.h>
#include <unistd.h>

#include "gtk_s9x.h"
#include "gtk_sound.h"
#include "gtk_sound_driver.h"

#ifdef USE_PORTAUDIO
#include "gtk_sound_driver_portaudio.h"
#endif
#ifdef USE_OSS
#include "gtk_sound_driver_oss.h"
#endif
#include "gtk_sound_driver_sdl.h"
#ifdef USE_ALSA
#include "gtk_sound_driver_alsa.h"
#endif
#ifdef USE_PULSEAUDIO
#include "gtk_sound_driver_pulse.h"
#endif

static int playback_rates[8] =
{
    0, 8000, 11025, 16000, 22050, 32000, 44100, 48000
};

S9xSoundDriver *driver;

int
S9xSoundBase2log (int num)
{
    int power;

    if (num < 1)
        return 0;

    for (power = 0; num > 1; power++)
    {
        num >>= 1;
    }

    return power;
}

int
S9xSoundPowerof2 (int num)
{
    return (1 << num);
}

void
S9xPortSoundInit ()
{
    int pao_driver = 0;
    int sdl_driver = 0;
    int oss_driver = 0;
    int alsa_driver = 0;
    int pulse_driver = 0;
    int max_driver = 0;

    driver = NULL;

#ifdef USE_PORTAUDIO
    sdl_driver++;
    oss_driver++;
    alsa_driver++;
    pulse_driver++;

    max_driver++;
#endif

#ifdef USE_OSS
    sdl_driver++;
    alsa_driver++;
    pulse_driver++;

    max_driver++;
#endif

    /* SDL */
    alsa_driver++;
    pulse_driver++;

    max_driver++;

#ifdef USE_ALSA
    max_driver++;
    pulse_driver++;
#endif

#ifdef USE_PULSEAUDIO
    max_driver++;
#endif

    if (gui_config->sound_driver >= max_driver)
        gui_config->sound_driver = 0;

#ifdef USE_PORTAUDIO
    if (gui_config->sound_driver == pao_driver)
        driver = new S9xPortAudioSoundDriver ();
#endif

#ifdef USE_OSS
    if (gui_config->sound_driver == oss_driver)
        driver = new S9xOSSSoundDriver ();
#endif

    if (gui_config->sound_driver == sdl_driver)
        driver = new S9xSDLSoundDriver ();

#ifdef USE_ALSA
    if (gui_config->sound_driver == alsa_driver)
        driver = new S9xAlsaSoundDriver ();
#endif

#ifdef USE_PULSEAUDIO
    if (gui_config->sound_driver == pulse_driver)
        driver = new S9xPulseSoundDriver ();
#endif

    if (driver != NULL)
    {
        driver->init ();

        if (gui_config->auto_input_rate)
        {
            Settings.SoundInputRate = top_level->get_auto_input_rate ();
            if (Settings.SoundInputRate == 0.0)
            {
                Settings.SoundInputRate = 31920;
                gui_config->auto_input_rate = 0;
            }
        }
        else
        {
            Settings.SoundInputRate = CLAMP (gui_config->sound_input_rate, 31700, 32300);
        }

        Settings.SoundPlaybackRate = playback_rates[gui_config->sound_playback_rate];

        S9xInitSound(0);

        S9xSetSoundMute (false);
    }
    else
    {
        S9xSetSoundMute (gui_config->mute_sound);
    }
}

void
S9xPortSoundReinit ()
{
    S9xPortSoundDeinit ();

    /* Ensure the sound device is released before trying to reopen it. */
    sync ();

    S9xPortSoundInit ();
}

void
S9xPortSoundDeinit ()
{
    S9xSoundStop ();

    driver->terminate ();

    delete driver;
}

void
S9xSoundStart ()
{
    driver->start ();
}

void
S9xSoundStop ()
{
    driver->stop ();
}

bool8
S9xOpenSoundDevice ()
{
    if (gui_config->mute_sound)
        return false;

    gui_config->sound_buffer_size = CLAMP (gui_config->sound_buffer_size, 2, 256);

    return driver->open_device ();
}

/* This really shouldn't be in the port layer */
void
S9xToggleSoundChannel (int c)
{
    static int sound_switch = 255;

    if (c == 8)
        sound_switch = 255;
    else
        sound_switch ^= 1 << c;

    S9xSetSoundControl (sound_switch);
}
