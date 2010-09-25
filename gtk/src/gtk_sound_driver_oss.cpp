#include "gtk_s9x.h"
#include "gtk_sound_driver_oss.h"

#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>

static void
oss_samples_available (void *data)
{
    ((S9xOSSSoundDriver *) data)->samples_available ();
}

S9xOSSSoundDriver::S9xOSSSoundDriver (void)
{
    filedes = -1;
    sound_buffer = NULL;
    sound_buffer_size = 0;

    return;
}

void
S9xOSSSoundDriver::init (void)
{
    return;
}

void
S9xOSSSoundDriver::terminate (void)
{
    stop ();

    S9xSetSamplesAvailableCallback (NULL, NULL);

    if (filedes >= 0)
    {
        close (filedes);
    }

    if (sound_buffer)
    {
        free (sound_buffer);
        sound_buffer = NULL;
    }

    return;
}

void
S9xOSSSoundDriver::start (void)
{
    return;
}

void
S9xOSSSoundDriver::stop (void)
{
    return;
}

bool8
S9xOSSSoundDriver::open_device (void)
{
    int temp;
    int output_buffer_size;

    output_buffer_size = (gui_config->sound_buffer_size * Settings.SoundPlaybackRate) / 1000;

    if (Settings.Stereo)
        output_buffer_size *= 2;
    if (Settings.SixteenBitSound)
        output_buffer_size *= 2;
    if (output_buffer_size > 65536)
        output_buffer_size = 65536;
    if (output_buffer_size < 256)
        output_buffer_size = 256;

    printf ("OSS sound driver initializing...\n");

    printf ("    --> (Device: /dev/dsp)...");

    filedes = open ("/dev/dsp", O_WRONLY | O_NONBLOCK);

    if (filedes < 0)
        goto fail;

    printf ("OK\n");


    if (Settings.SixteenBitSound)
    {
        printf ("    --> (Format: 16-bit)...");

        temp = AFMT_S16_LE;
        if (ioctl (filedes, SNDCTL_DSP_SETFMT, &temp) < 0)
            goto close_fail;
    }
    else
    {
        printf ("    --> (Format: 8-bit)...");

        temp = AFMT_U8;
        if (ioctl (filedes, SNDCTL_DSP_SETFMT, &temp) < 0)
            goto close_fail;
    }

    printf ("OK\n");

    if (Settings.Stereo)
    {
        temp = 2;
        printf ("    --> (Stereo)...");
    }
    else
    {
        temp = 1;
        printf ("    --> (Mono)...");
    }

    if (ioctl (filedes, SNDCTL_DSP_CHANNELS, &temp) < 0)
        goto close_fail;

    printf ("OK\n");

    printf ("    --> (Frequency: %d)...", Settings.SoundPlaybackRate);
    if (ioctl (filedes, SNDCTL_DSP_SPEED, &Settings.SoundPlaybackRate) < 0)
        goto close_fail;

    printf ("OK\n");

    /* OSS requires a power-of-two buffer size, first 16 bits are the number
     * of fragments to generate, second 16 are the respective power-of-two. */
    temp = (2 << 16) | (S9xSoundBase2log (output_buffer_size));

    output_buffer_size = S9xSoundPowerof2 (temp & 0xffff);

    printf ("    --> (Buffer size: %d bytes, %dms latency)...",
            output_buffer_size,
            (((output_buffer_size * 1000) >> (Settings.Stereo ? 1 : 0))
                                          >> (Settings.SixteenBitSound ? 1 : 0))
                                           / (Settings.SoundPlaybackRate));

    if (ioctl (filedes, SNDCTL_DSP_SETFRAGMENT, &temp) < 0)
        goto close_fail;

    printf ("OK\n");

    S9xSetSamplesAvailableCallback (oss_samples_available, this);

    return TRUE;

close_fail:

    close (filedes);

fail:
    printf ("failed\n");

    return FALSE;
}

void
S9xOSSSoundDriver::mix (void)
{
    return;
}

void
S9xOSSSoundDriver::samples_available (void)
{
    audio_buf_info info;
    int samples_to_write;
    int bytes_to_write;
    int bytes_written;

    S9xFinalizeSamples ();

    samples_to_write = S9xGetSampleCount ();

    ioctl (filedes, SNDCTL_DSP_GETOSPACE, &info);

    samples_to_write = MIN (info.bytes >> (Settings.SixteenBitSound ? 1 : 0),
                            samples_to_write);

    if (samples_to_write < 0)
        return;

    if (sound_buffer_size < samples_to_write << (Settings.SixteenBitSound ? 1 : 0))
    {
        sound_buffer = (uint8 *) realloc (sound_buffer, samples_to_write << (Settings.SixteenBitSound ? 1 : 0));
        sound_buffer_size = samples_to_write << (Settings.SixteenBitSound ? 1 : 0);
    }

    S9xMixSamples (sound_buffer, samples_to_write);

    bytes_written = 0;
    bytes_to_write = samples_to_write << (Settings.SixteenBitSound ? 1 : 0);

    while (bytes_to_write > bytes_written)
    {
        int result;

        result = write (filedes,
                        ((char *) sound_buffer) + bytes_written,
                        bytes_to_write - bytes_written);

        if (result < 0)
            break;

        bytes_written += result;
    }

    return;
}
