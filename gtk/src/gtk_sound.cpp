/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <unistd.h>

#include "gtk_s9x.h"
#include "gtk_sound.h"
#include "common/audio/s9x_sound_driver.hpp"
#include "snes9x.h"
#include "apu/apu.h"

#ifdef USE_PORTAUDIO
#include "common/audio/s9x_sound_driver_portaudio.hpp"
#endif
#ifdef USE_OSS
#include "common/audio/s9x_sound_driver_oss.hpp"
#endif
#include "common/audio/s9x_sound_driver_sdl.hpp"
#ifdef USE_ALSA
#include "common/audio/s9x_sound_driver_alsa.hpp"
#endif
#ifdef USE_PULSEAUDIO
#include "common/audio/s9x_sound_driver_pulse.hpp"
#endif

static int playback_rates[8] =
{
    0, 8000, 11025, 16000, 22050, 32000, 44100, 48000
};

static S9xSoundDriver *driver;

int S9xSoundBase2log(int num)
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

int S9xSoundPowerof2(int num)
{
    return (1 << num);
}

std::vector<std::string> S9xGetSoundDriverNames()
{
    std::vector<std::string> names;

#ifdef USE_PORTAUDIO
    names.push_back("PortAudio");
#endif
#ifdef USE_OSS
    names.push_back("OSS");
#endif
#ifdef USE_ALSA
    names.push_back("ALSA");
#endif
#ifdef USE_PULSEAUDIO
    names.push_back("PulseAudio");
#endif
    names.push_back("SDL");

    return names;
}

void S9xPortSoundInit()
{
    if (gui_config->sound_driver >= (int)gui_config->sound_drivers.size())
        gui_config->sound_driver = 0;

    auto &name = gui_config->sound_drivers[gui_config->sound_driver];

#ifdef USE_PORTAUDIO
    if (name == "PortAudio")
        driver = new S9xPortAudioSoundDriver();
#endif

#ifdef USE_OSS
    if (name == "OSS")
        driver = new S9xOSSSoundDriver();
#endif

#ifdef USE_ALSA
    if (name == "ALSA")
        driver = new S9xAlsaSoundDriver();
#endif

#ifdef USE_PULSEAUDIO
    if (name == "PulseAudio")
        driver = new S9xPulseSoundDriver();
#endif

    if (name == "SDL")
        driver = new S9xSDLSoundDriver();

    if (driver != NULL)
    {
        driver->init();

        if (gui_config->auto_input_rate)
        {
            Settings.SoundInputRate = top_level->get_auto_input_rate();
            if (Settings.SoundInputRate == 0.0)
            {
                Settings.SoundInputRate = 31950;
                gui_config->auto_input_rate = 0;
            }
        }
        else
        {
            Settings.SoundInputRate = CLAMP(gui_config->sound_input_rate, 31700, 32300);
        }

        Settings.SoundPlaybackRate = playback_rates[gui_config->sound_playback_rate];

        S9xInitSound(0);

        S9xSetSoundMute(false);
    }
    else
    {
        S9xSetSoundMute(gui_config->mute_sound);
    }
}

void S9xPortSoundReinit()
{
    S9xPortSoundDeinit();

    /* Ensure the sound device is released before trying to reopen it. */
    sync();

    S9xPortSoundInit();
}

void S9xPortSoundDeinit()
{
    S9xSoundStop();

    if (driver)
        driver->deinit();

    delete driver;
}

void S9xSoundStart()
{
    if (driver)
        driver->start();
}

void S9xSoundStop()
{
    if (driver)
        driver->stop();
}

static std::vector<int16_t> temp_buffer;
void S9xSamplesAvailable(void *userdata)
{
    bool clear_leftover_samples = false;
    int samples = S9xGetSampleCount();
    int space_free = driver->space_free();

    if (space_free < samples)
    {
        if (!Settings.SoundSync)
            clear_leftover_samples = true;

        if (Settings.SoundSync && !Settings.TurboMode && !Settings.Mute)
        {
            for (int i = 0; i < 200; i++) // Wait for a max of 5ms
            {
                space_free = driver->space_free();
                if (space_free < samples)
                    usleep(50);
                else
                    break;
            }
        }
    }

    if (space_free < samples)
        samples = space_free & ~1;

    if (samples == 0)
    {
        S9xClearSamples();
        return;
    }

    if ((int)temp_buffer.size() < samples)
        temp_buffer.resize(samples);
    S9xMixSamples((uint8_t *)temp_buffer.data(), samples);
    driver->write_samples(temp_buffer.data(), samples);

    if (clear_leftover_samples)
        S9xClearSamples();

    if (Settings.DynamicRateControl)
    {
        auto level = driver->buffer_level();
        S9xUpdateDynamicRate(level.first, level.second);
    }
}

bool8 S9xOpenSoundDevice()
{
    if (gui_config->mute_sound)
        return false;

    gui_config->sound_buffer_size = CLAMP(gui_config->sound_buffer_size, 2, 256);

    S9xSetSamplesAvailableCallback(S9xSamplesAvailable, nullptr);
    return driver->open_device(Settings.SoundPlaybackRate, gui_config->sound_buffer_size);
}

/* This really shouldn't be in the port layer */
void S9xToggleSoundChannel(int c)
{
    static int sound_switch = 255;

    if (c == 8)
        sound_switch = 255;
    else
        sound_switch ^= 1 << c;

    S9xSetSoundControl(sound_switch);
}
