/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "s9x_sound_driver_alsa.hpp"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>

S9xAlsaSoundDriver::S9xAlsaSoundDriver()
{
    pcm = {};
}

void S9xAlsaSoundDriver::init()
{
}

void S9xAlsaSoundDriver::deinit()
{
    stop();

    if (pcm)
    {
        snd_pcm_drain(pcm);
        snd_pcm_close(pcm);
        pcm = nullptr;
    }
}

void S9xAlsaSoundDriver::start()
{
}

void S9xAlsaSoundDriver::stop()
{
}

bool S9xAlsaSoundDriver::open_device(int playback_rate, int buffer_size_ms)
{
    unsigned int periods = 8;
    unsigned int buffer_size = buffer_size_ms * 1000;
    snd_pcm_sw_params_t *sw_params;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_uframes_t alsa_buffer_size, alsa_period_size;
    unsigned int min = 0;
    unsigned int max = 0;
    unsigned int playback_rate_param = 0;

    printf("ALSA sound driver initializing...\n");
    printf("    --> (Device: default)...\n");

    int err = snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);

    if (err < 0)
    {
        goto fail;
    }

    printf("    --> (16-bit Stereo, %dhz, %d ms)...\n",
           playback_rate,
           buffer_size_ms);

    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_any(pcm, hw_params);
    snd_pcm_hw_params_set_format(pcm, hw_params, SND_PCM_FORMAT_S16);
    snd_pcm_hw_params_set_access(pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_rate_resample(pcm, hw_params, 0);
    snd_pcm_hw_params_set_channels(pcm, hw_params, 2);

    snd_pcm_hw_params_get_rate_min(hw_params, &min, nullptr);
    snd_pcm_hw_params_get_rate_max(hw_params, &max, nullptr);
    printf("    --> Available rates: %d to %d\n", min, max);
    if (playback_rate > (int)max || playback_rate < (int)min)
    {
        printf("        Rate %d not available. Using %d instead.\n", playback_rate, max);
        playback_rate = max;
    }

    playback_rate_param = playback_rate;
    snd_pcm_hw_params_set_rate_near(pcm, hw_params, &playback_rate_param, nullptr);

    snd_pcm_hw_params_get_buffer_time_min(hw_params, &min, nullptr);
    snd_pcm_hw_params_get_buffer_time_max(hw_params, &max, nullptr);
    printf("    --> Available buffer sizes: %dms to %dms\n", min / 1000, max / 1000);
    if (buffer_size < min && buffer_size > max)
    {
        printf("        Buffer size %dms not available. Using %d instead.\n", buffer_size / 1000, (min + max) / 2000);
        buffer_size = (min + max) / 2;
    }
    snd_pcm_hw_params_set_buffer_time_near(pcm, hw_params, &buffer_size, nullptr);

    snd_pcm_hw_params_get_periods_min(hw_params, &min, nullptr);
    snd_pcm_hw_params_get_periods_max(hw_params, &max, nullptr);
    printf("    --> Period ranges: %d to %d blocks\n", min, max);
    if (periods > max)
    {
        periods = max;
    }
    snd_pcm_hw_params_set_periods_near(pcm, hw_params, &periods, nullptr);

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

    output_buffer_size_bytes = snd_pcm_frames_to_bytes(pcm, alsa_buffer_size);

    if (err < 0)
    {
        printf("        Software parameter set failed.\n");
        goto close_fail;
    }

    printf("OK\n");

    return true;

close_fail:
    snd_pcm_drain(pcm);
    snd_pcm_close(pcm);
    pcm = nullptr;

fail:
    printf("Failed: %s\n", snd_strerror(err));

    return false;
}

bool S9xAlsaSoundDriver::write_samples(int16_t *data, int samples)
{
    snd_pcm_sframes_t frames = snd_pcm_avail(pcm);

    if (frames < 0)
    {
        snd_pcm_recover(pcm, frames, 1);
        return false;
    }

    bool result = true;
    snd_pcm_nonblock(pcm, 0);
    if (frames > samples / 2)
    {
        frames = samples / 2;
        result = false;
    }

    size_t bytes = snd_pcm_frames_to_bytes(pcm, frames);
    if (bytes <= 0)
        return false;

    snd_pcm_sframes_t frames_written = 0;
    while (frames_written < frames)
    {
        int result = snd_pcm_writei(pcm,
                                    &data[snd_pcm_frames_to_bytes(pcm, frames_written) / 2],
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

    return result;
}

int S9xAlsaSoundDriver::space_free()
{
    return snd_pcm_avail(pcm) * 2;
}

std::pair<int, int> S9xAlsaSoundDriver::buffer_level()
{
    return { snd_pcm_avail(pcm), output_buffer_size_bytes / 4 };
}
