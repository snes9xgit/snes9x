/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "s9x_sound_driver_oss.hpp"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstdio>

static inline int log2(int num)
{
    int power;

    if (num < 1)
        return 0;

    for (power = 0; num > 1; power++)
    {
        num >>= 1;
    }

    return power;
}

S9xOSSSoundDriver::S9xOSSSoundDriver()
{
    filedes = -1;
}

void S9xOSSSoundDriver::init()
{
}

void S9xOSSSoundDriver::deinit()
{
    stop();

    if (filedes >= 0)
    {
        close(filedes);
    }
}

void S9xOSSSoundDriver::start()
{
}

void S9xOSSSoundDriver::stop()
{
}

bool S9xOSSSoundDriver::open_device(int playback_rate, int buffer_size_ms)
{
    int temp;
    audio_buf_info info;

    output_buffer_size_bytes = (buffer_size_ms * playback_rate) / 1000;

    output_buffer_size_bytes *= 4;
    if (output_buffer_size_bytes < 256)
        output_buffer_size_bytes = 256;

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

    printf("    --> (Frequency: %d)...", playback_rate);
    if (ioctl(filedes, SNDCTL_DSP_SPEED, &playback_rate) < 0)
        goto close_fail;

    printf("OK\n");

    /* OSS requires a power-of-two buffer size, first 16 bits are the number
     * of fragments to generate, second 16 are the respective power-of-two. */
    temp = (4 << 16) | (log2(output_buffer_size_bytes / 4));

    if (ioctl(filedes, SNDCTL_DSP_SETFRAGMENT, &temp) < 0)
        goto close_fail;

    ioctl(filedes, SNDCTL_DSP_GETOSPACE, &info);

    output_buffer_size_bytes = info.fragsize * info.fragstotal;

    printf("    --> (Buffer size: %d bytes, %dms latency)...",
           output_buffer_size_bytes,
           (output_buffer_size_bytes * 250) / playback_rate);

    printf("OK\n");

    return true;

close_fail:

    close(filedes);

fail:
    printf("failed\n");

    return false;
}

int S9xOSSSoundDriver::space_free()
{
    audio_buf_info info;
    ioctl(filedes, SNDCTL_DSP_GETOSPACE, &info);
    return info.bytes / 2;
}

std::pair<int, int> S9xOSSSoundDriver::buffer_level()
{
    return { space_free(), output_buffer_size_bytes / 2};
}

void S9xOSSSoundDriver::write_samples(int16_t *data, int samples)
{
    audio_buf_info info;
    int bytes_to_write;
    int bytes_written;

    ioctl(filedes, SNDCTL_DSP_GETOSPACE, &info);

    if (samples > info.bytes / 2)
        samples = info.bytes / 2;

    if (samples == 0)
        return;

    bytes_written = 0;
    bytes_to_write = samples * 2;

    while (bytes_to_write > bytes_written)
    {
        int result;

        result = write(filedes,
                       ((char *)data) + bytes_written,
                       bytes_to_write - bytes_written);

        if (result < 0)
            break;

        bytes_written += result;
    }
}
