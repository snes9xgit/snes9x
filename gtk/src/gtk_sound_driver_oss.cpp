/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

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

S9xOSSSoundDriver::S9xOSSSoundDriver ()
{
    filedes = -1;
    sound_buffer = NULL;
    sound_buffer_size = 0;
}

void
S9xOSSSoundDriver::init ()
{
}

void
S9xOSSSoundDriver::terminate ()
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
}

void
S9xOSSSoundDriver::start ()
{
}

void
S9xOSSSoundDriver::stop ()
{
}

bool8
S9xOSSSoundDriver::open_device ()
{
    int temp;
    audio_buf_info info;

    output_buffer_size = (gui_config->sound_buffer_size * Settings.SoundPlaybackRate) / 1000;

    if (Settings.Stereo)
        output_buffer_size *= 2;
    if (Settings.SixteenBitSound)
        output_buffer_size *= 2;
    if (output_buffer_size < 256)
        output_buffer_size = 256;

    printf ("OSS sound driver initializing...\n");

    printf ("Device: /dev/dsp: ");

    filedes = open ("/dev/dsp", O_WRONLY | O_NONBLOCK);

    if (filedes < 0)
    {
        printf ("Failed.\n");
        char dspstring[16] = "/dev/dspX\0";

        for (int i = 1; i <= 9; i++)
        {
            dspstring[8] = '0' + i;

            printf ("Trying %s: ", dspstring);

            filedes = open (dspstring, O_WRONLY | O_NONBLOCK);

            if (filedes < 0)
            {
                if (i == 9)
                    goto fail;
                printf ("Failed.\n");
            }
            else
                break;
        }
    }

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
    temp = (4 << 16) | (S9xSoundBase2log (output_buffer_size / 4));

    if (ioctl (filedes, SNDCTL_DSP_SETFRAGMENT, &temp) < 0)
        goto close_fail;

    ioctl (filedes, SNDCTL_DSP_GETOSPACE, &info);

    output_buffer_size = info.fragsize * info.fragstotal;

    printf ("    --> (Buffer size: %d bytes, %dms latency)...",
            output_buffer_size,
            (((output_buffer_size * 1000) >> (Settings.Stereo ? 1 : 0))
                                          >> (Settings.SixteenBitSound ? 1 : 0))
                                           / (Settings.SoundPlaybackRate));

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
S9xOSSSoundDriver::samples_available ()
{
    audio_buf_info info;
    int samples_to_write;
    int bytes_to_write;
    int bytes_written;

    ioctl (filedes, SNDCTL_DSP_GETOSPACE, &info);

    if (Settings.DynamicRateControl)
    {
        S9xUpdateDynamicRate (info.bytes, output_buffer_size);
    }

    S9xFinalizeSamples ();

    samples_to_write = S9xGetSampleCount ();

    if (Settings.DynamicRateControl)
    {
        // Using rate control, we should always keep the emulator's sound buffers empty to
        // maintain an accurate measurement.
        if (samples_to_write > (info.bytes >> (Settings.SixteenBitSound ? 1 : 0)))
        {
            S9xClearSamples ();
            return;
        }
    }

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
}
