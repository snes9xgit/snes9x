#include "gtk_s9x.h"
#include "gtk_sound_driver_portaudio.h"

static int
port_audio_callback (const void *input,
                     void *output,
                     unsigned long frameCount,
                     const PaStreamCallbackTimeInfo* timeInfo,
                     PaStreamCallbackFlags statusFlags,
                     void *userData)
{
    ((S9xPortAudioSoundDriver *) userData)->mix ((unsigned char *) output, frameCount * (Settings.Stereo ? 2 : 1) * (Settings.SixteenBitSound ? 2 : 1));

    return 0;
}

static void
port_audio_samples_available_callback (void *data)
{
    ((S9xPortAudioSoundDriver *) data)->samples_available ();

    return;
}

void
S9xPortAudioSoundDriver::mix (unsigned char *output, int bytes)
{
    g_mutex_lock (mutex);

    S9xMixSamples (output, bytes >> (Settings.SixteenBitSound ? 1 : 0));

    g_mutex_unlock (mutex);

    return;
}

S9xPortAudioSoundDriver::S9xPortAudioSoundDriver(void)
{
    audio_stream = NULL;
    mutex = NULL;

    return;
}

void
S9xPortAudioSoundDriver::init (void)
{
    PaError err;

    err = Pa_Initialize ();

    if (err != paNoError)
        fprintf (stderr,
                 "Couldn't initialize PortAudio: %s\n",
                 Pa_GetErrorText (err));

    return;
}

void
S9xPortAudioSoundDriver::terminate (void)
{
    stop ();

    if (mutex)
    {
        g_mutex_free (mutex);
        mutex = NULL;
    }

    S9xSetSamplesAvailableCallback (NULL, NULL);

    Pa_Terminate ();

    return;
}

void
S9xPortAudioSoundDriver::start (void)
{
    PaError err;

    if (audio_stream != NULL && !(gui_config->mute_sound))
    {
        if ((Pa_IsStreamActive (audio_stream)))
            return;

        err = Pa_StartStream (audio_stream);

        if (err != paNoError)
        {
            fprintf (stderr, "Error: %s\n", Pa_GetErrorText (err));
        }
    }

    return;
}

void
S9xPortAudioSoundDriver::stop (void)
{
    if (audio_stream != NULL)
    {
        Pa_StopStream (audio_stream);
    }

    return;
}

bool8
S9xPortAudioSoundDriver::open_device (void)
{
    PaStreamParameters  param;
    const PaDeviceInfo  *device_info;
    const PaHostApiInfo *hostapi_info;
    PaError             err = paNoError;

    if (audio_stream != NULL)
    {
        printf ("Shutting down sound for reset\n");
        err = Pa_CloseStream (audio_stream);

        if (err != paNoError)
        {
            fprintf (stderr,
                     "Couldn't reset audio stream.\nError: %s\n",
                     Pa_GetErrorText (err));
            return TRUE;
        }

        audio_stream = NULL;
    }

    param.channelCount = Settings.Stereo ? 2 : 1;
    param.sampleFormat = Settings.SixteenBitSound ? paInt16 : paUInt8;
    param.hostApiSpecificStreamInfo = NULL;

    printf ("PortAudio sound driver initializing...\n");

    for (int i = 0; i < Pa_GetHostApiCount (); i++)
    {
        printf ("    --> ");

        hostapi_info = Pa_GetHostApiInfo (i);
        if (!hostapi_info)
        {
            printf ("Host API #%d has no info\n", i);
            err = paNotInitialized;
            continue;
        }

        device_info = Pa_GetDeviceInfo (hostapi_info->defaultOutputDevice);
        if (!device_info)
        {
            printf ("(%s)...No device info available.\n", hostapi_info->name);
            err = paNotInitialized;
            continue;
        }

        param.device = hostapi_info->defaultOutputDevice;
        param.suggestedLatency = gui_config->sound_buffer_size * 0.001;

        printf ("(%s : %s, latency %dms)...",
                hostapi_info->name,
                device_info->name,
                (int) (param.suggestedLatency * 1000.0));

        fflush (stdout);

        err = Pa_OpenStream (&audio_stream,
                             NULL,
                             &param,
                             Settings.SoundPlaybackRate,
                             0,
                             paNoFlag,
                             port_audio_callback,
                             this);

        if (err == paNoError)
        {
            printf ("OK\n");
            break;
        }
        else
        {
            printf ("Failed (%s)\n",
                    Pa_GetErrorText (err));
        }
    }

    if (err != paNoError || Pa_GetHostApiCount () < 1)
    {
        fprintf (stderr,
                 "Couldn't initialize sound\n");
        return FALSE;
    }

    mutex = g_mutex_new ();
    S9xSetSamplesAvailableCallback (port_audio_samples_available_callback, this);

    fflush (stdout);
    fflush (stderr);

    return TRUE;
}

void
S9xPortAudioSoundDriver::samples_available (void)
{
    g_mutex_lock (mutex);

    S9xFinalizeSamples ();

    g_mutex_unlock (mutex);

    return;
}

void
S9xPortAudioSoundDriver::mix (void)
{
    return;
}
