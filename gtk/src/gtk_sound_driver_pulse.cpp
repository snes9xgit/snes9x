/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_sound_driver_pulse.h"
#include "gtk_s9x.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>

static void pulse_samples_available(void *data)
{
    ((S9xPulseSoundDriver *)data)->samples_available();
}

S9xPulseSoundDriver::S9xPulseSoundDriver()
{
    mainloop = NULL;
    context = NULL;
    stream = NULL;
    buffer_size = 0;
}

void S9xPulseSoundDriver::init()
{
}

void S9xPulseSoundDriver::terminate()
{
    S9xSetSamplesAvailableCallback(NULL, NULL);

    if (mainloop)
        pa_threaded_mainloop_stop(mainloop);

    if (stream)
    {
        pa_stream_disconnect(stream);
        pa_stream_unref(stream);
    }

    if (context)
    {
        pa_context_disconnect(context);
        pa_context_unref(context);
    }

    if (mainloop)
    {
        pa_threaded_mainloop_free(mainloop);
    }
}

void S9xPulseSoundDriver::start()
{
}

void S9xPulseSoundDriver::stop()
{
}

void S9xPulseSoundDriver::lock()
{
    pa_threaded_mainloop_lock(mainloop);
}

void S9xPulseSoundDriver::unlock()
{
    pa_threaded_mainloop_unlock(mainloop);
}

void S9xPulseSoundDriver::wait()
{
    pa_threaded_mainloop_wait(mainloop);
}

static void context_state_cb(pa_context *c, void *userdata)
{
    S9xPulseSoundDriver *driver = (S9xPulseSoundDriver *)userdata;
    int state;

    state = pa_context_get_state(c);

    if (state == PA_CONTEXT_READY ||
        state == PA_CONTEXT_FAILED ||
        state == PA_CONTEXT_TERMINATED)
    {
        pa_threaded_mainloop_signal(driver->mainloop, 0);
    }
}

static void stream_state_callback(pa_stream *p, void *userdata)
{
    S9xPulseSoundDriver *driver = (S9xPulseSoundDriver *)userdata;
    int state;

    state = pa_stream_get_state(p);

    if (state == PA_STREAM_READY ||
        state == PA_STREAM_FAILED ||
        state == PA_STREAM_TERMINATED)
    {
        pa_threaded_mainloop_signal(driver->mainloop, 0);
    }
}

bool S9xPulseSoundDriver::open_device()
{
    int err = PA_ERR_UNKNOWN;
    pa_sample_spec ss;
    pa_buffer_attr buffer_attr;
    const pa_buffer_attr *actual_buffer_attr;

    ss.channels = 2;
    ss.format = PA_SAMPLE_S16NE;
    ss.rate = Settings.SoundPlaybackRate;

    buffer_attr.fragsize = -1;
    buffer_attr.tlength = pa_usec_to_bytes(gui_config->sound_buffer_size * 1000, &ss);
    buffer_attr.maxlength = buffer_attr.tlength * 2;
    buffer_attr.minreq = -1;
    buffer_attr.prebuf = -1;

    printf("PulseAudio sound driver initializing...\n");

    printf("    --> (%dhz, 16-bit Stereo, %dms)...",
           Settings.SoundPlaybackRate,
           gui_config->sound_buffer_size);
    fflush(stdout);

    mainloop = pa_threaded_mainloop_new();
    if (!mainloop)
    {
        fprintf(stderr, "Failed to create mainloop.\n");
        goto error0;
    }

    context = pa_context_new(pa_threaded_mainloop_get_api(mainloop), "Snes9x");
    if (!context)
        goto error1;

    pa_context_set_state_callback(context, context_state_cb, this);
    if ((err = pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL)) != 0)
        goto error2;

    lock();

    if ((err = pa_threaded_mainloop_start(mainloop)) != 0)
        goto error2;
    wait();

    if ((err = pa_context_get_state(context)) != PA_CONTEXT_READY)
    {
        printf("Coundn't create context: State: %d\n", err);
        goto error2;
    }

    stream = pa_stream_new(context, "Game", &ss, NULL);

    if (!stream)
        goto error2;

    pa_stream_set_state_callback(stream, stream_state_callback, this);

    if (pa_stream_connect_playback(stream,
                                   NULL,
                                   &buffer_attr,
                                   PA_STREAM_ADJUST_LATENCY,
                                   NULL,
                                   NULL) < 0)
        goto error3;
    wait();

    if (pa_stream_get_state(stream) != PA_STREAM_READY)
    {
        goto error3;
    }

    actual_buffer_attr = pa_stream_get_buffer_attr(stream);

    buffer_size = actual_buffer_attr->tlength;

    printf("OK\n");

    S9xSetSamplesAvailableCallback(pulse_samples_available, this);

    unlock();

    return true;

error3:
    pa_stream_disconnect(stream);
    pa_stream_unref(stream);
error2:
    pa_context_disconnect(context);
    pa_context_unref(context);
    unlock();
error1:
    pa_threaded_mainloop_free(mainloop);
error0:
    printf("Failed: %s\n", pa_strerror(err));

    return false;
}

void S9xPulseSoundDriver::samples_available()
{
    size_t bytes;
    int samples;
    const pa_buffer_attr *buffer_attr;
    void *output_buffer = NULL;

    lock();
    bytes = pa_stream_writable_size(stream);
    buffer_attr = pa_stream_get_buffer_attr(stream);
    unlock();

    buffer_size = buffer_attr->tlength;

    if (Settings.DynamicRateControl)
    {
        S9xUpdateDynamicRate(bytes, buffer_size);
    }

    samples = S9xGetSampleCount();

    if (Settings.DynamicRateControl && !Settings.SoundSync)
    {
        if ((int)bytes < (samples * 2))
        {
            S9xClearSamples();
            return;
        }
    }

    if (Settings.SoundSync && !Settings.TurboMode && !Settings.Mute)
    {
        while ((int)bytes < samples * 2)
        {
            int usec_to_sleep = ((samples >> 1) - (bytes >> 2)) * 10000 /
                                (Settings.SoundPlaybackRate / 100);
            usleep(usec_to_sleep > 0 ? usec_to_sleep : 0);
            lock();
            bytes = pa_stream_writable_size(stream);
            unlock();
        }
    }

    bytes = MIN((int)bytes, samples * 2) & ~1;

    if (!bytes)
        return;

    lock();

    if (pa_stream_begin_write(stream, &output_buffer, &bytes) != 0)
    {
        pa_stream_flush(stream, NULL, NULL);
        unlock();
        return;
    }

    if (bytes <= 0 || !output_buffer)
    {
        unlock();
        return;
    }

    S9xMixSamples((uint8 *)output_buffer, bytes >> 1);
    pa_stream_write(stream, output_buffer, bytes, NULL, 0, PA_SEEK_RELATIVE);

    unlock();
}
