#include "gtk_s9x.h"
#include "gtk_sound_driver_alsa.h"

#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>

static void
alsa_samples_available (void *data)
{
    ((S9xAlsaSoundDriver *) data)->samples_available ();
}

S9xAlsaSoundDriver::S9xAlsaSoundDriver (void)
{
    pcm = NULL;
    sound_buffer = NULL;
    sound_buffer_size = 0;

    return;
}

void
S9xAlsaSoundDriver::init (void)
{
    return;
}

void
S9xAlsaSoundDriver::terminate (void)
{
    stop ();

    S9xSetSamplesAvailableCallback (NULL, NULL);

    if (pcm)
    {
        snd_pcm_drain (pcm);
        snd_pcm_close (pcm);
        pcm = NULL;
    }

    if (sound_buffer)
    {
        free (sound_buffer);
        sound_buffer = NULL;
    }

    return;
}

void
S9xAlsaSoundDriver::start (void)
{
    return;
}

void
S9xAlsaSoundDriver::stop (void)
{
    return;
}

bool8
S9xAlsaSoundDriver::open_device (void)
{
    int err;
    snd_pcm_sw_params_t *sw_params;
    snd_pcm_uframes_t alsa_buffer_size, alsa_period_size;

    printf ("ALSA sound driver initializing...\n");
    printf ("    --> (Device: default)...");

    err = snd_pcm_open (&pcm,
                        "default",
                        SND_PCM_STREAM_PLAYBACK,
                        SND_PCM_NONBLOCK);

    if (err < 0)
    {
        goto fail;
    }

    printf ("OK\n");

    printf ("    --> (%s, %s, %dhz, %d ms)...",
            Settings.SixteenBitSound ? "16-bit" : "8-bit",
            Settings.Stereo ? "Stereo" : "Mono",
            Settings.SoundPlaybackRate,
            gui_config->sound_buffer_size);

    if ((err = snd_pcm_set_params (pcm,
                                   Settings.SixteenBitSound ? SND_PCM_FORMAT_S16 : SND_PCM_FORMAT_U8,
                                   SND_PCM_ACCESS_RW_INTERLEAVED,
                                   Settings.Stereo ? 2 : 1,
                                   Settings.SoundPlaybackRate,
                                   1 /* Allow software resampling */,
                                   gui_config->sound_buffer_size * 1000))
         < 0)
    {
        goto close_fail;
    }

    snd_pcm_sw_params_alloca (&sw_params);
    snd_pcm_sw_params_current (pcm, sw_params);
    snd_pcm_get_params (pcm, &alsa_buffer_size, &alsa_period_size);
    /* Don't start until we're [nearly] full */
    snd_pcm_sw_params_set_start_threshold (pcm,
                                           sw_params,
                                           (alsa_buffer_size / alsa_period_size) * alsa_period_size);
    /* Transfer in blocks of period-size */
    snd_pcm_sw_params_set_avail_min (pcm, sw_params, alsa_period_size);
    err = snd_pcm_sw_params (pcm, sw_params);
    if (err < 0)
        goto close_fail;

    printf ("OK\n");

    S9xSetSamplesAvailableCallback (alsa_samples_available, this);

    return TRUE;

close_fail:
    snd_pcm_drain (pcm);
    snd_pcm_close (pcm);
    pcm = NULL;

fail:
    printf ("Failed: %s\n", snd_strerror (err));

    return FALSE;
}

void
S9xAlsaSoundDriver::mix (void)
{
    return;
}

void
S9xAlsaSoundDriver::samples_available (void)
{
    snd_pcm_sframes_t frames_written, frames;
    int bytes;

    S9xFinalizeSamples ();

    frames = snd_pcm_avail_update (pcm);
    if (frames < 0)
    {
        frames = snd_pcm_recover (pcm, frames, 1);
    }

    frames = MIN (frames, S9xGetSampleCount () >> (Settings.Stereo ? 1 : 0));

    bytes = snd_pcm_frames_to_bytes (pcm, frames);
    if (bytes <= 0)
    {
        return;
    }

    if (sound_buffer_size < bytes || sound_buffer == NULL)
    {
        sound_buffer = (uint8 *) realloc (sound_buffer, bytes);
        sound_buffer_size = bytes;
    }

    S9xMixSamples (sound_buffer, frames << (Settings.Stereo ? 1 : 0));

    frames_written = 0;

    while (frames_written < frames)
    {
        int result;

        result = snd_pcm_writei (pcm,
                                 sound_buffer +
                                  snd_pcm_frames_to_bytes (pcm, frames_written),
                                 frames - frames_written);

        if (result < 0)
        {
            result = snd_pcm_recover (pcm, result, 1);

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

    return;
}
