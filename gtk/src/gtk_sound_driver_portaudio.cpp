/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_sound_driver_portaudio.h"
#include <cstring>

S9xPortAudioSoundDriver::S9xPortAudioSoundDriver()
{
    audio_stream = NULL;
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

void S9xPortAudioSoundDriver::deinit()
{
    stop();

    Pa_Terminate();
}

void S9xPortAudioSoundDriver::start()
{
    PaError err;

    if (audio_stream != NULL)
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

bool S9xPortAudioSoundDriver::open_device(int playback_rate, int buffer_size_ms)
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
        param.suggestedLatency = buffer_size_ms * 0.001;

        printf("(%s : %s, latency %dms)...\n",
               hostapi_info->name,
               device_info->name,
               (int)(param.suggestedLatency * 1000.0));

        fflush(stdout);

        err = Pa_OpenStream(&audio_stream,
                            NULL,
                            &param,
                            playback_rate,
                            0,
                            paNoFlag,
                            NULL,
                            NULL);

        int frames = Pa_GetStreamWriteAvailable(audio_stream);
        printf ("PortAudio set buffer size to %d frames.\n", frames);
        output_buffer_size = frames;

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

    return true;
}

int S9xPortAudioSoundDriver::space_free()
{
    return Pa_GetStreamWriteAvailable(audio_stream) * 2;
}

std::pair<int, int> S9xPortAudioSoundDriver::buffer_level()
{
    return { Pa_GetStreamWriteAvailable(audio_stream), output_buffer_size };
}

void S9xPortAudioSoundDriver::write_samples(int16_t *data, int samples)
{
    int frames;

    frames = Pa_GetStreamWriteAvailable(audio_stream);

    if (frames == output_buffer_size)
    {
        // Prime the stream
        std::vector<int16_t> tmp(output_buffer_size);
        memset(tmp.data(), 0, output_buffer_size << 1);
        Pa_WriteStream(audio_stream, tmp.data(), output_buffer_size >> 1);
        frames -= output_buffer_size >> 1;
    }

    if (frames > samples / 2)
        frames = samples / 2;

    Pa_WriteStream(audio_stream, data, frames);
}
