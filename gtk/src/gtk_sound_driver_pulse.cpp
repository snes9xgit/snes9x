/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_sound_driver_pulse.h"
#include "gtk_s9x.h"
#include "snes9x.h"
#include "apu/apu.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>

S9xPulseSoundDriver::S9xPulseSoundDriver()
{
    init();
}

void S9xPulseSoundDriver::init()
{
    mainloop = {};
    context = {};
    stream = {};
    buffer_size = {};
}

void S9xPulseSoundDriver::terminate()
{
    S9xSetSamplesAvailableCallback(nullptr, nullptr);

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
    init();

    pa_sample_spec ss;
    ss.channels = 2;
    ss.format = PA_SAMPLE_S16NE;
    ss.rate = Settings.SoundPlaybackRate;

    pa_buffer_attr buffer_attr;
    buffer_attr.fragsize = -1;
    buffer_attr.tlength = pa_usec_to_bytes(gui_config->sound_buffer_size * 1000, &ss);
    buffer_attr.maxlength = buffer_attr.tlength * 2;
    buffer_attr.minreq = -1;
    buffer_attr.prebuf = -1;

    printf("PulseAudio sound driver initializing...\n");

    printf("    --> (%dhz, 16-bit Stereo, %dms)...",
           Settings.SoundPlaybackRate,
           gui_config->sound_buffer_size);

    int err = PA_ERR_UNKNOWN;
    mainloop = pa_threaded_mainloop_new();
    context = pa_context_new(pa_threaded_mainloop_get_api(mainloop), "Snes9x");
    pa_context_set_state_callback(context, context_state_cb, this);
    pa_context_connect(context, nullptr, PA_CONTEXT_NOFLAGS, nullptr);

    lock();
    pa_threaded_mainloop_start(mainloop);
    wait();

    if ((err = pa_context_get_state(context)) != PA_CONTEXT_READY)
        return false;

    stream = pa_stream_new(context, "Game", &ss, nullptr);

    pa_stream_set_state_callback(stream, stream_state_callback, this);

    if (pa_stream_connect_playback(stream,
                                   nullptr,
                                   &buffer_attr,
                                   PA_STREAM_ADJUST_LATENCY,
                                   nullptr,
                                   nullptr) < 0)
    {
        return false;
    }

    wait();

    if (pa_stream_get_state(stream) != PA_STREAM_READY)
        return false;

    auto actual_buffer_attr = pa_stream_get_buffer_attr(stream);
    unlock();

    buffer_size = actual_buffer_attr->tlength;

    printf("OK\n");

    S9xSetSamplesAvailableCallback([](void *userdata) {
        ((decltype(this)) userdata)->samples_available();;
    }, this);


    return true;
}

void S9xPulseSoundDriver::samples_available()
{
    lock();
    size_t bytes = pa_stream_writable_size(stream);
    auto buffer_attr = pa_stream_get_buffer_attr(stream);
    unlock();

    buffer_size = buffer_attr->tlength;

    if (Settings.DynamicRateControl)
    {
        S9xUpdateDynamicRate(bytes, buffer_size);
    }

    size_t samples = S9xGetSampleCount();

    int frames_available = samples / 2;
    int frames_writable = bytes / 4;

    if (frames_writable < frames_available)
    {
        if (Settings.DynamicRateControl && !Settings.SoundSync)
        {
            S9xClearSamples();
            return;
        }

        if (Settings.SoundSync && !Settings.TurboMode && !Settings.Mute)
        {
            int usec_to_sleep = (frames_available - frames_writable) * 10000 /
                                (Settings.SoundPlaybackRate / 100);
            usleep(usec_to_sleep > 0 ? usec_to_sleep : 0);
            lock();
            bytes = pa_stream_writable_size(stream);
            unlock();
        }
    }

    bytes = MIN(bytes, samples * 2) & ~1;

    if (!bytes)
        return;

    lock();

    void *output_buffer;;
    if (pa_stream_begin_write(stream, &output_buffer, &bytes) != 0)
    {
        pa_stream_flush(stream, nullptr, nullptr);
        unlock();
        return;
    }

    if (bytes <= 0 || !output_buffer)
    {
        unlock();
        return;
    }

    S9xMixSamples((uint8_t *)output_buffer, bytes >> 1);
    pa_stream_write(stream, output_buffer, bytes, nullptr, 0, PA_SEEK_RELATIVE);

    unlock();
}
