#include "gtk_s9x.h"
#include "gtk_sound_driver_pulse.h"

#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>

static void
pulse_samples_available (void *data)
{
    ((S9xPulseSoundDriver *) data)->samples_available ();
}

S9xPulseSoundDriver::S9xPulseSoundDriver (void)
{
    pulse = NULL;
    buffer = NULL;
    buffer_size = 0;

    return;
}

void
S9xPulseSoundDriver::init (void)
{
    return;
}

void
S9xPulseSoundDriver::terminate (void)
{
    stop ();

    S9xSetSamplesAvailableCallback (NULL, NULL);

    if (pulse)
    {
        pa_simple_free (pulse);
    }

    if (buffer)
    {
        delete[] buffer;
        buffer = NULL;
    }

    return;
}

void
S9xPulseSoundDriver::start (void)
{
    return;
}

void
S9xPulseSoundDriver::stop (void)
{
    return;
}

bool8
S9xPulseSoundDriver::open_device (void)
{
    int err;
    pa_sample_spec ss;
    pa_buffer_attr buffer_attr;

    ss.channels = Settings.Stereo ? 2 : 1;
    ss.format   = Settings.SixteenBitSound ? PA_SAMPLE_S16NE : PA_SAMPLE_U8;
    ss.rate     = Settings.SoundPlaybackRate;

    buffer_attr.fragsize  = -1;
    buffer_attr.tlength   = pa_usec_to_bytes (gui_config->sound_buffer_size * 1000, &ss);
    buffer_attr.maxlength = -1;
    buffer_attr.minreq    = -1;
    buffer_attr.prebuf    = -1;

    printf ("PulseAudio sound driver initializing...\n");

    printf ("    --> (%dhz, %s %s, %dms)...",
            Settings.SoundPlaybackRate,
            Settings.SixteenBitSound ? "16-bit" : "8-bit",
            Settings.Stereo ? "Stereo" : "Mono",
            gui_config->sound_buffer_size);

    pulse = pa_simple_new (NULL,
                           "Snes9x",
                           PA_STREAM_PLAYBACK,
                           NULL,
                           "Game",
                           &ss,
                           NULL,
                           &buffer_attr,
                           &err);

    if (!pulse)
    {
        printf ("Failed: %s\n", pa_strerror (err));
        return FALSE;
    }

    printf ("OK\n");

    S9xSetSamplesAvailableCallback (pulse_samples_available, this);

    return TRUE;
}

void
S9xPulseSoundDriver::mix (void)
{
    return;
}

void
S9xPulseSoundDriver::samples_available (void)
{
    int bytes, err;

    S9xFinalizeSamples ();

    bytes = (S9xGetSampleCount () << (Settings.SixteenBitSound ? 1 : 0));

    if (bytes <= 128)
        return;

    if (buffer_size < bytes || buffer == NULL)
    {
        delete[] buffer;
        buffer = new uint8[bytes];
        buffer_size = bytes;
    }

    S9xMixSamples (buffer, bytes >> (Settings.SixteenBitSound ? 1 : 0));

    /* PulseAudio-simple has no write indicator, so we don't know when the
       buffer is full. So we just drop the audio when we're in Turbo Mode. */
    if (Settings.TurboMode)
        return;

    pa_simple_write (pulse, buffer, bytes, &err);

    return;
}
