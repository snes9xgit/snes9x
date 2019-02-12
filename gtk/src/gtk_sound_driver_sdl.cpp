/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_sound_driver_sdl.h"
#include "gtk_s9x.h"

static void sdl_audio_callback(void *userdata, Uint8 *stream, int len)
{
    ((S9xSDLSoundDriver *)userdata)->mix((unsigned char *)stream, len);
}

static void c_samples_available(void *data)
{
    ((S9xSDLSoundDriver *)data)->samples_available();
}

void S9xSDLSoundDriver::samples_available()
{
    int snes_samples_available = S9xGetSampleCount();
    S9xMixSamples((uint8 *)temp, snes_samples_available);

    if (Settings.SoundSync && !Settings.TurboMode && !Settings.Mute)
    {
        mutex.lock();
        int samples = buffer->space_empty();
        mutex.unlock();

        while (samples < snes_samples_available)
        {
            usleep(100);
            mutex.lock();
            samples = buffer->space_empty();
            mutex.unlock();
        }
    }

    mutex.lock();
    buffer->push(temp, snes_samples_available);
    mutex.unlock();
}

void S9xSDLSoundDriver::mix(unsigned char *output, int bytes)
{
    mutex.lock();
    if (buffer->avail() >= bytes >> 1)
        buffer->read((int16_t *)output, bytes >> 1);
    mutex.unlock();
}

S9xSDLSoundDriver::S9xSDLSoundDriver()
{
    buffer = NULL;
    audiospec = NULL;
}

void S9xSDLSoundDriver::init()
{
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    stop();
}

void S9xSDLSoundDriver::terminate()
{
    stop();

    if (audiospec)
    {
        SDL_CloseAudio();
        if (buffer)
            delete buffer;
        free(audiospec);
        audiospec = NULL;
    }

    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void S9xSDLSoundDriver::start()
{
    if (!gui_config->mute_sound)
    {
        if (audiospec)
        {
            SDL_PauseAudio(0);
        }
    }
}

void S9xSDLSoundDriver::stop()
{
    if (audiospec)
    {
        SDL_PauseAudio(1);
    }
}

bool S9xSDLSoundDriver::open_device()
{
    audiospec = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));

    audiospec->freq = Settings.SoundPlaybackRate;
    audiospec->channels = 2;
    audiospec->format = AUDIO_S16SYS;
    audiospec->samples = (gui_config->sound_buffer_size * audiospec->freq / 1000) >> 2;
    audiospec->callback = sdl_audio_callback;
    audiospec->userdata = this;

    printf("SDL sound driver initializing...\n");
    printf("    --> (Frequency: %dhz, Latency: %dms)...",
           audiospec->freq,
           (audiospec->samples * 1000 / audiospec->freq));

    if (SDL_OpenAudio(audiospec, NULL) < 0)
    {
        printf("Failed\n");

        free(audiospec);
        audiospec = NULL;

        return false;
    }

    printf("OK\n");

    buffer = new Resampler(gui_config->sound_buffer_size * audiospec->freq / 500);
    buffer->time_ratio(1.0);

    S9xSetSamplesAvailableCallback(c_samples_available, this);

    return true;
}
