/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "s9x_sound_driver_cubeb.hpp"
#include <cstdio>

bool S9xCubebSoundDriver::write_samples(int16_t *sample_data, int num_samples)
{
    if (!buffer.push(sample_data, num_samples))
    {
        return buffer.push(sample_data, buffer.space_empty() / 2);
    }
    return true;
}

S9xCubebSoundDriver::~S9xCubebSoundDriver()
{
    S9xCubebSoundDriver::deinit();
}

void S9xCubebSoundDriver::init()
{
    if (!context)
        cubeb_init(&context, "Snes9x", nullptr);
    stop();
}

void S9xCubebSoundDriver::deinit()
{
    stop();
    if (stream)
    {
        cubeb_stream_destroy(stream);
        stream = nullptr;
    }

    if (context)
    {
        cubeb_destroy(context);
        context = nullptr;
    }
}

void S9xCubebSoundDriver::start()
{
    if (stream)
        cubeb_stream_start(stream);
}

void S9xCubebSoundDriver::stop()
{
    if (stream)
        cubeb_stream_stop(stream);
}

void state_callback(cubeb_stream *stream, void *user_ptr, cubeb_state state)
{
}

long data_callback(cubeb_stream *stream, void *user_ptr,
                   void const *input_buffer,
                   void *output_buffer, long nframes)
{
    return ((S9xCubebSoundDriver *)user_ptr)->data_callback(stream, input_buffer, output_buffer, nframes);
}

long S9xCubebSoundDriver::data_callback(cubeb_stream *stream, void const *input_buffer, void *output_buffer, long num_frames)
{
    auto available_samples = buffer.avail();
    if (available_samples >= num_frames * 2)
    {
        buffer.pull((int16_t *)output_buffer, num_frames * 2);
    }
    else
    {
        auto zeroed_samples = num_frames * 2 - available_samples;
        memset(output_buffer, 0, zeroed_samples);
        buffer.pull((int16_t *)output_buffer + zeroed_samples, num_frames * 2 - zeroed_samples);
    }

    return num_frames;
}

bool S9xCubebSoundDriver::open_device(int playback_rate, int buffer_size)
{
    cubeb_stream_params params{};
    params.channels = 2;
    params.format = CUBEB_SAMPLE_S16LE;
    params.layout = CUBEB_LAYOUT_UNDEFINED;
    params.rate = playback_rate;
    params.prefs = CUBEB_STREAM_PREF_NONE;

    uint32_t suggested_latency = playback_rate * buffer_size / 1000;
    uint32_t min_latency;
    cubeb_get_min_latency(context, &params, &min_latency);

    auto retval = cubeb_stream_init(context, &stream, "Snes9x",
                                    nullptr, nullptr,
                                    nullptr, &params,
                                    min_latency,
                                    &::data_callback,
                                    &state_callback,
                                    this);

    if (retval != CUBEB_OK)
    {
        printf("Failed to start stream. Error: %d!\n", retval);
        stream = nullptr;
        return false;
    }

    buffer.resize(suggested_latency * 2);

    return true;
}

int S9xCubebSoundDriver::space_free()
{
    return buffer.space_empty();
}

std::pair<int, int> S9xCubebSoundDriver::buffer_level()
{
    return { buffer.space_empty(), buffer.size() };
}