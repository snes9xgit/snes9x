/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_sound_driver_oss.h"
#include "gtk_s9x.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <sys/time.h>

static void oss_samples_available(void *data)
{
    ((S9xOSSSoundDriver *)data)->samples_available();
}

S9xOSSSoundDriver::S9xOSSSoundDriver()
{
    filedes = -1;
    sound_buffer = NULL;
    sound_buffer_size = 0;
}

void S9xOSSSoundDriver::init()
{
}

void S9xOSSSoundDriver::terminate()
{
    stop();

    S9xSetSamplesAvailableCallback(NULL, NULL);

    if (filedes >= 0)
    {
        close(filedes);
    }

    if (sound_buffer)
    {
        free(sound_buffer);
        sound_buffer = NULL;
    }
}

void S9xOSSSoundDriver::start()
{
}

void S9xOSSSoundDriver::stop()
{
}

bool S9xOSSSoundDriver::open_device()
{
    int temp;
    audio_buf_info info;

    output_buffer_size = (gui_config->sound_buffer_size * Settings.SoundPlaybackRate) / 1000;

    output_buffer_size *= 4;
    if (output_buffer_size < 256)
        output_buffer_size = 256;

    printf("OSS sound driver initializing...\n");

    printf("Device: /dev/dsp: ");

    filedes = open("/dev/dsp", O_WRONLY | O_NONBLOCK);

    if (filedes < 0)
    {
        printf("Failed.\n");
        char dspstring[16] = "/dev/dspX\0";

        for (int i = 1; i <= 9; i++)
        {
            dspstring[8] = '0' + i;

            printf("Trying %s: ", dspstring);

            filedes = open(dspstring, O_WRONLY | O_NONBLOCK);

            if (filedes < 0)
            {
                if (i == 9)
                    goto fail;
                printf("Failed.\n");
            }
            else
                break;
        }
    }

    printf("OK\n");

    printf("    --> (Format: 16-bit)...");

    temp = AFMT_S16_LE;
    if (ioctl(filedes, SNDCTL_DSP_SETFMT, &temp) < 0)
        goto close_fail;

    printf("OK\n");

    temp = 2;
    printf("    --> (Stereo)...");

    if (ioctl(filedes, SNDCTL_DSP_CHANNELS, &temp) < 0)
        goto close_fail;

    printf("OK\n");

    printf("    --> (Frequency: %d)...", Settings.SoundPlaybackRate);
    if (ioctl(filedes, SNDCTL_DSP_SPEED, &Settings.SoundPlaybackRate) < 0)
        goto close_fail;

    printf("OK\n");

    /* OSS requires a power-of-two buffer size, first 16 bits are the number
     * of fragments to generate, second 16 are the respective power-of-two. */
    temp = (4 << 16) | (S9xSoundBase2log(output_buffer_size / 4));

    if (ioctl(filedes, SNDCTL_DSP_SETFRAGMENT, &temp) < 0)
        goto close_fail;

    ioctl(filedes, SNDCTL_DSP_GETOSPACE, &info);

    output_buffer_size = info.fragsize * info.fragstotal;

    printf("    --> (Buffer size: %d bytes, %dms latency)...",
           output_buffer_size,
           (output_buffer_size * 250) / Settings.SoundPlaybackRate);

    printf("OK\n");

    S9xSetSamplesAvailableCallback(oss_samples_available, this);

    return true;

close_fail:

    close(filedes);

fail:
    printf("failed\n");

    return false;
}

void S9xOSSSoundDriver::samples_available()
{
    audio_buf_info info;
    int samples_to_write;
    int bytes_to_write;
    int bytes_written;

    ioctl(filedes, SNDCTL_DSP_GETOSPACE, &info);

    if (Settings.DynamicRateControl)
    {
        S9xUpdateDynamicRate(info.bytes, output_buffer_size);
    }

    samples_to_write = S9xGetSampleCount();

    if (Settings.DynamicRateControl && !Settings.SoundSync)
    {
        // Using rate control, we should always keep the emulator's sound buffers empty to
        // maintain an accurate measurement.
        if (samples_to_write > (info.bytes >> 1))
        {
            S9xClearSamples();
            return;
        }
    }

    if (Settings.SoundSync && !Settings.TurboMode && !Settings.Mute)
    {
        while (info.bytes >> 1 < samples_to_write)
        {
            int usec_to_sleep = ((samples_to_write >> 1) - (info.bytes >> 2)) * 10000 /
                                (Settings.SoundPlaybackRate / 100);
            usleep(usec_to_sleep > 0 ? usec_to_sleep : 0);
            ioctl(filedes, SNDCTL_DSP_GETOSPACE, &info);
        }
    }
    else
    {
        samples_to_write = MIN(info.bytes >> 1, samples_to_write) & ~1;
    }

    if (samples_to_write < 0)
        return;

    if (sound_buffer_size < samples_to_write * 2)
    {
        sound_buffer = (uint8 *)realloc(sound_buffer, samples_to_write * 2);
        sound_buffer_size = samples_to_write * 2;
    }

    S9xMixSamples(sound_buffer, samples_to_write);

    bytes_written = 0;
    bytes_to_write = samples_to_write * 2;

    while (bytes_to_write > bytes_written)
    {
        int result;

        result = write(filedes,
                       ((char *)sound_buffer) + bytes_written,
                       bytes_to_write - bytes_written);

        if (result < 0)
            break;

        bytes_written += result;
    }
}
