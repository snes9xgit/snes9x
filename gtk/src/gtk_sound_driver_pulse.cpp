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
    mainloop = NULL;
    context = NULL;
    stream = NULL;
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
    S9xSetSamplesAvailableCallback (NULL, NULL);

    if (mainloop)
        pa_threaded_mainloop_stop (mainloop);

    if (stream)
    {
        pa_stream_disconnect (stream);
        pa_stream_unref (stream);
    }

    if (context)
    {
        pa_context_disconnect (context);
        pa_context_unref (context);
    }

    if (mainloop)
    {
        pa_threaded_mainloop_free (mainloop);
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

void
S9xPulseSoundDriver::lock (void)
{
    pa_threaded_mainloop_lock (mainloop);

    return;
}

void
S9xPulseSoundDriver::unlock (void)
{
    pa_threaded_mainloop_unlock (mainloop);

    return;
}

void
S9xPulseSoundDriver::wait (void)
{
    pa_threaded_mainloop_wait (mainloop);

    return;
}

static void
context_state_cb (pa_context *c, void *userdata)
{
    S9xPulseSoundDriver *driver = (S9xPulseSoundDriver *) userdata;
    int state;

    state = pa_context_get_state (c);

    if (state == PA_CONTEXT_READY      ||
        state == PA_CONTEXT_FAILED     ||
        state == PA_CONTEXT_TERMINATED)
    {
        pa_threaded_mainloop_signal (driver->mainloop, 0);
    }

    return;
}

static void
stream_state_callback (pa_stream *p, void *userdata)
{
    S9xPulseSoundDriver *driver = (S9xPulseSoundDriver *) userdata;
    int state;

    state = pa_stream_get_state (p);

    if (state == PA_STREAM_READY      ||
        state == PA_STREAM_FAILED     ||
        state == PA_STREAM_TERMINATED)
    {
        pa_threaded_mainloop_signal (driver->mainloop, 0);
    }

    return;
}

bool8
S9xPulseSoundDriver::open_device (void)
{
    int err = PA_ERR_UNKNOWN;
    pa_sample_spec ss;
    pa_buffer_attr buffer_attr;
    const pa_buffer_attr *actual_buffer_attr;

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
    fflush (stdout);

    mainloop = pa_threaded_mainloop_new ();
    if (!mainloop)
    {
        fprintf (stderr, "Failed to create mainloop.\n");
        goto error0;
    }

    context = pa_context_new (pa_threaded_mainloop_get_api (mainloop), "Snes9x");
    if (!context)
        goto error1;

    pa_context_set_state_callback (context, context_state_cb, this);
    if ((err = pa_context_connect (context, NULL, PA_CONTEXT_NOFLAGS, NULL)) != 0)
        goto error2;

    lock ();

    if ((err = pa_threaded_mainloop_start (mainloop)) != 0)
        goto error2;
    wait ();

    if ((err = pa_context_get_state (context)) != PA_CONTEXT_READY)
    {
        printf ("Coundn't create context: State: %d\n", err);
        goto error2;
    }

    stream = pa_stream_new (context, "Game", &ss, NULL);

    if (!stream)
        goto error2;

    pa_stream_set_state_callback (stream, stream_state_callback, this);

    if (pa_stream_connect_playback (stream,
                                    NULL,
                                    &buffer_attr,
                                    PA_STREAM_ADJUST_LATENCY,
                                    NULL,
                                    NULL) < 0)
        goto error3;
    wait ();

    if (pa_stream_get_state (stream) != PA_STREAM_READY)
    {
        goto error3;
    }

    actual_buffer_attr = pa_stream_get_buffer_attr (stream);

    buffer_size = actual_buffer_attr->tlength;

    printf ("OK\n");

    S9xSetSamplesAvailableCallback (pulse_samples_available, this);

    unlock ();

    return TRUE;

error3:
    pa_stream_disconnect (stream);
    pa_stream_unref (stream);
error2:
    pa_context_disconnect (context);
    pa_context_unref (context);
    unlock ();
error1:
    pa_threaded_mainloop_free (mainloop);
error0:
    printf ("Failed: %s\n", pa_strerror (err));

    return FALSE;
}

void
S9xPulseSoundDriver::samples_available (void)
{
    size_t bytes;
    int samples;
    const pa_buffer_attr *buffer_attr;
    void *output_buffer = NULL;

    lock ();
    bytes = pa_stream_writable_size (stream);
    buffer_attr = pa_stream_get_buffer_attr (stream);
    unlock ();

    buffer_size = buffer_attr->tlength;

    if (Settings.DynamicRateControl)
    {
        S9xUpdateDynamicRate (bytes, buffer_size);
    }

    S9xFinalizeSamples ();

    samples = S9xGetSampleCount ();

    if (Settings.DynamicRateControl)
    {
        if ((int) bytes < (samples << (Settings.SixteenBitSound ? 1 : 0)))
        {
            S9xClearSamples ();
            return;
        }
    }

    bytes = MIN (bytes, (samples << (Settings.SixteenBitSound ? 1 : 0)));

    if (!bytes)
        return;

    lock ();

    if (pa_stream_begin_write (stream, &output_buffer, &bytes) != 0)
    {
        pa_stream_flush (stream, NULL, NULL);
        unlock ();
        return;
    }
    if (bytes <= 0 || !output_buffer)
    {
        unlock ();
        return;
    }

    S9xMixSamples ((uint8 *) output_buffer, bytes >> (Settings.SixteenBitSound ? 1 : 0));
    pa_stream_write (stream, output_buffer, bytes, NULL, 0, PA_SEEK_RELATIVE);

    unlock ();

    return;
}
