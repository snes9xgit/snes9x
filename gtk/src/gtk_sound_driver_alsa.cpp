/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_sound_driver_alsa.h"
#include "gtk_s9x.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>

static void alsa_samples_available(void *data)
{
    ((S9xAlsaSoundDriver *)data)->samples_available();
}

S9xAlsaSoundDriver::S9xAlsaSoundDriver()
{
    pcm = NULL;
    sound_buffer = NULL;
    sound_buffer_size = 0;
}

void S9xAlsaSoundDriver::init()
{
}

void S9xAlsaSoundDriver::terminate()
{
    stop();

    S9xSetSamplesAvailableCallback(NULL, NULL);

    if (pcm)
    {
        snd_pcm_drain(pcm);
        snd_pcm_close(pcm);
        pcm = NULL;
    }

    if (sound_buffer)
    {
        free(sound_buffer);
        sound_buffer = NULL;
    }
}

void S9xAlsaSoundDriver::start()
{
}

void S9xAlsaSoundDriver::stop()
{
}

bool S9xAlsaSoundDriver::open_device()
{
    int err;
    unsigned int periods = 8;
    unsigned int buffer_size = gui_config->sound_buffer_size * 1000;
    snd_pcm_sw_params_t *sw_params;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_uframes_t alsa_buffer_size, alsa_period_size;
    unsigned int min = 0;
    unsigned int max = 0;

    printf("ALSA sound driver initializing...\n");
    printf("    --> (Device: default)...\n");

    err = snd_pcm_open(&pcm,
                       "default",
                       SND_PCM_STREAM_PLAYBACK,
                       SND_PCM_NONBLOCK);

    if (err < 0)
    {
        goto fail;
    }

    printf("    --> (16-bit Stereo, %dhz, %d ms)...\n",
           Settings.SoundPlaybackRate,
           gui_config->sound_buffer_size);

    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_any(pcm, hw_params);
    snd_pcm_hw_params_set_format(pcm, hw_params, SND_PCM_FORMAT_S16);
    snd_pcm_hw_params_set_access(pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_rate_resample(pcm, hw_params, 0);
    snd_pcm_hw_params_set_channels(pcm, hw_params, 2);

    snd_pcm_hw_params_get_rate_min(hw_params, &min, NULL);
    snd_pcm_hw_params_get_rate_max(hw_params, &max, NULL);
    printf("    --> Available rates: %d to %d\n", min, max);
    if (Settings.SoundPlaybackRate > max && Settings.SoundPlaybackRate < min)
    {
        printf("        Rate %d not available. Using %d instead.\n", Settings.SoundPlaybackRate, max);
        Settings.SoundPlaybackRate = max;
    }
    snd_pcm_hw_params_set_rate_near(pcm, hw_params, &Settings.SoundPlaybackRate, NULL);

    snd_pcm_hw_params_get_buffer_time_min(hw_params, &min, NULL);
    snd_pcm_hw_params_get_buffer_time_max(hw_params, &max, NULL);
    printf("    --> Available buffer sizes: %dms to %dms\n", min / 1000, max / 1000);
    if (buffer_size < min && buffer_size > max)
    {
        printf("        Buffer size %dms not available. Using %d instead.\n", buffer_size / 1000, (min + max) / 2000);
        buffer_size = (min + max) / 2;
    }
    snd_pcm_hw_params_set_buffer_time_near(pcm, hw_params, &buffer_size, NULL);

    snd_pcm_hw_params_get_periods_min(hw_params, &min, NULL);
    snd_pcm_hw_params_get_periods_max(hw_params, &max, NULL);
    printf("    --> Period ranges: %d to %d blocks\n", min, max);
    if (periods > max)
    {
        periods = max;
    }
    snd_pcm_hw_params_set_periods_near(pcm, hw_params, &periods, NULL);

    if ((err = snd_pcm_hw_params(pcm, hw_params)) < 0)
    {
        printf("        Hardware parameter set failed.\n");
        goto close_fail;
    }

    snd_pcm_sw_params_alloca(&sw_params);
    snd_pcm_sw_params_current(pcm, sw_params);
    snd_pcm_get_params(pcm, &alsa_buffer_size, &alsa_period_size);
    /* Don't start until we're [nearly] full */
    snd_pcm_sw_params_set_start_threshold(pcm, sw_params, (alsa_buffer_size / 2));
    err = snd_pcm_sw_params(pcm, sw_params);

    output_buffer_size = snd_pcm_frames_to_bytes(pcm, alsa_buffer_size);

    if (err < 0)
    {
        printf("        Software parameter set failed.\n");
        goto close_fail;
    }

    printf("OK\n");

    S9xSetSamplesAvailableCallback(alsa_samples_available, this);

    return true;

close_fail:
    snd_pcm_drain(pcm);
    snd_pcm_close(pcm);
    pcm = NULL;

fail:
    printf("Failed: %s\n", snd_strerror(err));

    return false;
}

void S9xAlsaSoundDriver::samples_available()
{
    snd_pcm_sframes_t frames_written, frames;
    int bytes;

    frames = snd_pcm_avail(pcm);

    if (frames < 0)
    {
        frames = snd_pcm_recover(pcm, frames, 1);
        return;
    }

    if (Settings.DynamicRateControl)
    {
        S9xUpdateDynamicRate(snd_pcm_frames_to_bytes(pcm, frames),
                             output_buffer_size);
    }

    int snes_frames_available = S9xGetSampleCount() >> 1;

    if (Settings.DynamicRateControl && !Settings.SoundSync)
    {
        // Using rate control, we should always keep the emulator's sound buffers empty to
        // maintain an accurate measurement.
        if (frames < snes_frames_available)
        {
            S9xClearSamples();
            return;
        }
    }

    if (Settings.SoundSync && !Settings.TurboMode && !Settings.Mute)
    {
        snd_pcm_nonblock(pcm, 0);
        frames = snes_frames_available;
    }
    else
    {
        snd_pcm_nonblock(pcm, 1);
        frames = MIN(frames, snes_frames_available);
    }

    bytes = snd_pcm_frames_to_bytes(pcm, frames);
    if (bytes <= 0)
        return;

    if (sound_buffer_size < bytes || sound_buffer == NULL)
    {
        sound_buffer = (uint8 *)realloc(sound_buffer, bytes);
        sound_buffer_size = bytes;
    }

    S9xMixSamples(sound_buffer, frames * 2);

    frames_written = 0;

    while (frames_written < frames)
    {
        int result;

        result = snd_pcm_writei(pcm,
                                sound_buffer +
                                    snd_pcm_frames_to_bytes(pcm, frames_written),
                                frames - frames_written);

        if (result < 0)
        {
            result = snd_pcm_recover(pcm, result, 1);

            if (result < 0)
            {
                break;
            }
        }
        else
        {
            frames_written += result;
        }
    }
}
