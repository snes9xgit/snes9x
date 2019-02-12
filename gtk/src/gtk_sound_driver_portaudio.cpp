/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_sound_driver_portaudio.h"
#include "gtk_s9x.h"

static inline int frames_to_bytes(int frames)
{
    return frames * 4;
}

static void port_audio_samples_available_callback(void *data)
{
    ((S9xPortAudioSoundDriver *)data)->samples_available();
}

S9xPortAudioSoundDriver::S9xPortAudioSoundDriver()
{
    audio_stream = NULL;
    sound_buffer = NULL;
    sound_buffer_size = 0;
}

void S9xPortAudioSoundDriver::init()
{
    PaError err;

    err = Pa_Initialize();

    if (err != paNoError)
        fprintf(stderr,
                "Couldn't initialize PortAudio: %s\n",
                Pa_GetErrorText(err));
}

void S9xPortAudioSoundDriver::terminate()
{
    stop();

    S9xSetSamplesAvailableCallback(NULL, NULL);

    Pa_Terminate();

    if (sound_buffer)
    {
        free(sound_buffer);
        sound_buffer = NULL;
    }
}

void S9xPortAudioSoundDriver::start()
{
    PaError err;

    if (audio_stream != NULL && !(gui_config->mute_sound))
    {
        if ((Pa_IsStreamActive(audio_stream)))
            return;

        err = Pa_StartStream(audio_stream);

        if (err != paNoError)
        {
            fprintf(stderr, "Error: %s\n", Pa_GetErrorText(err));
        }
    }
}

void S9xPortAudioSoundDriver::stop()
{
    if (audio_stream != NULL)
    {
        Pa_StopStream(audio_stream);
    }
}

bool S9xPortAudioSoundDriver::open_device()
{
    PaStreamParameters param;
    const PaDeviceInfo *device_info;
    const PaHostApiInfo *hostapi_info;
    PaError err = paNoError;

    if (audio_stream != NULL)
    {
        printf("Shutting down sound for reset\n");
        err = Pa_CloseStream(audio_stream);

        if (err != paNoError)
        {
            fprintf(stderr,
                    "Couldn't reset audio stream.\nError: %s\n",
                    Pa_GetErrorText(err));
            return true;
        }

        audio_stream = NULL;
    }

    param.channelCount = 2;
    param.sampleFormat = paInt16;
    param.hostApiSpecificStreamInfo = NULL;

    printf("PortAudio sound driver initializing...\n");

    for (int i = 0; i < Pa_GetHostApiCount(); i++)
    {
        printf("    --> ");

        hostapi_info = Pa_GetHostApiInfo(i);
        if (!hostapi_info)
        {
            printf("Host API #%d has no info\n", i);
            err = paNotInitialized;
            continue;
        }

        device_info = Pa_GetDeviceInfo(hostapi_info->defaultOutputDevice);
        if (!device_info)
        {
            printf("(%s)...No device info available.\n", hostapi_info->name);
            err = paNotInitialized;
            continue;
        }

        param.device = hostapi_info->defaultOutputDevice;
        param.suggestedLatency = gui_config->sound_buffer_size * 0.001;

        printf("(%s : %s, latency %dms)...",
               hostapi_info->name,
               device_info->name,
               (int)(param.suggestedLatency * 1000.0));

        fflush(stdout);

        err = Pa_OpenStream(&audio_stream,
                            NULL,
                            &param,
                            Settings.SoundPlaybackRate,
                            0,
                            paNoFlag,
                            NULL,
                            NULL);

        int frames = Pa_GetStreamWriteAvailable(audio_stream);
        output_buffer_size = frames_to_bytes(frames);

        if (err == paNoError)
        {
            printf("OK\n");
            break;
        }
        else
        {
            printf("Failed (%s)\n",
                   Pa_GetErrorText(err));
        }
    }

    if (err != paNoError || Pa_GetHostApiCount() < 1)
    {
        fprintf(stderr,
                "Couldn't initialize sound\n");
        return false;
    }

    S9xSetSamplesAvailableCallback(port_audio_samples_available_callback, this);

    fflush(stdout);
    fflush(stderr);

    return true;
}

void S9xPortAudioSoundDriver::samples_available()
{
    int frames;
    int bytes;

    frames = Pa_GetStreamWriteAvailable(audio_stream);

    if (Settings.DynamicRateControl)
    {
        S9xUpdateDynamicRate(frames_to_bytes(frames), output_buffer_size);
    }

    int snes_frames_available = S9xGetSampleCount() >> 1;

    if (Settings.DynamicRateControl && !Settings.SoundSync)
    {
        // Using rate control, we should always keep the emulator's sound buffers empty to
        // maintain an accurate measurement.
        if (frames < (S9xGetSampleCount() >> 1))
        {
            S9xClearSamples();
            return;
        }
    }

    if (Settings.SoundSync && !Settings.TurboMode && !Settings.Mute)
    {
        while (frames < snes_frames_available)
        {
            int usec_to_sleep = (snes_frames_available - frames) * 10000 /
                                (Settings.SoundPlaybackRate / 100);
            usleep(usec_to_sleep > 0 ? usec_to_sleep : 0);
            frames = Pa_GetStreamWriteAvailable(audio_stream);
        }
    }

    frames = MIN(frames, snes_frames_available);
    bytes = frames_to_bytes(frames);

    if (sound_buffer_size < bytes || sound_buffer == NULL)
    {
        sound_buffer = (uint8 *)realloc(sound_buffer, bytes);
        sound_buffer_size = bytes;
    }

    S9xMixSamples(sound_buffer, frames << 1);

    Pa_WriteStream(audio_stream, sound_buffer, frames);
}
