#include "gtk_s9x.h"
#include "gtk_sound_driver_sdl.h"

static void
sdl_audio_callback (void *userdata, Uint8 *stream, int len)
{
    ((S9xSDLSoundDriver *) userdata)->mix ((unsigned char *) stream, len);
}

static void
samples_available (void *data)
{
    SDL_LockAudio ();
    S9xFinalizeSamples ();
    SDL_UnlockAudio ();
}

void
S9xSDLSoundDriver::mix (unsigned char *output, int bytes)
{
    SDL_LockAudio ();
    S9xMixSamples (output, bytes >> (Settings.SixteenBitSound ? 1 : 0));
    SDL_UnlockAudio ();
}

S9xSDLSoundDriver::S9xSDLSoundDriver ()
{
    audiospec = NULL;
}

void
S9xSDLSoundDriver::init ()
{
    SDL_InitSubSystem (SDL_INIT_AUDIO);
    stop ();
}

void
S9xSDLSoundDriver::terminate ()
{
    stop ();

    if (audiospec)
    {
        SDL_CloseAudio ();
        free (audiospec);
        audiospec = NULL;
    }

    SDL_QuitSubSystem (SDL_INIT_AUDIO);
}

void
S9xSDLSoundDriver::start ()
{
    if (!gui_config->mute_sound)
    {
        if (audiospec)
        {
            SDL_PauseAudio (0);
        }
    }
}

void
S9xSDLSoundDriver::stop ()
{
    if (audiospec)
    {
        SDL_PauseAudio (1);
    }
}

bool8
S9xSDLSoundDriver::open_device ()
{
    audiospec = (SDL_AudioSpec *) malloc (sizeof (SDL_AudioSpec));

    audiospec->freq = Settings.SoundPlaybackRate;
    audiospec->channels = Settings.Stereo ? 2 : 1;
    audiospec->format = Settings.SixteenBitSound ? AUDIO_S16SYS : AUDIO_U8;
    audiospec->samples = (gui_config->sound_buffer_size * audiospec->freq / 1000) >> 1;
    audiospec->callback = sdl_audio_callback;
    audiospec->userdata = this;

    printf ("SDL sound driver initializing...\n");
    printf ("    --> (Frequency: %dhz, Latency: %dms)...",
            audiospec->freq,
            (audiospec->samples * 1000 / audiospec->freq) << 1);

    if (SDL_OpenAudio (audiospec, NULL) < 0)
    {
        printf ("Failed\n");

        free (audiospec);
        audiospec = NULL;

        return FALSE;
    }

    printf ("OK\n");

    S9xSetSamplesAvailableCallback (samples_available, NULL);

    return TRUE;
}
