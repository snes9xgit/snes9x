/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "s9x_sound_driver_sdl3.hpp"
#include "SDL3/SDL_audio.h"
#include <cstdio>
#include <vector>

bool S9xSDL3SoundDriver::write_samples(int16_t *data, int samples)
{
    bool retval = true;

    mutex.lock();
    auto empty = buffer.space_empty();
    if (samples > empty)
    {
        retval = false;
        buffer.dump(buffer.buffer_size / 2 - empty);
    }
    buffer.push(data, samples);
    mutex.unlock();

    return retval;
}

void S9xSDL3SoundDriver::mix(int req, int total)
{
    if (tmp.size() < req / 2)
        tmp.resize(req / 2);

    mutex.lock();
    if (buffer.avail() >= req / 2)
    {

        buffer.read((int16_t *)(tmp.data()), req / 2);
        mutex.unlock();
        SDL_PutAudioStreamData(stream, tmp.data(), req);
    }
    else
    {
        auto avail = buffer.avail();
        buffer.read((int16_t *)(tmp.data()), avail);
        buffer.add_silence(buffer.buffer_size / 2);
        mutex.unlock();
        SDL_PutAudioStreamData(stream, tmp.data(), avail * 2);

    }
}

S9xSDL3SoundDriver::S9xSDL3SoundDriver()
{
    stream = nullptr;
}

S9xSDL3SoundDriver::~S9xSDL3SoundDriver()
{
    S9xSDL3SoundDriver::deinit();
}

void S9xSDL3SoundDriver::init()
{
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    stop();
}

void S9xSDL3SoundDriver::deinit()
{
    stop();
    SDL_DestroyAudioStream(stream);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void S9xSDL3SoundDriver::start()
{
    SDL_ResumeAudioStreamDevice(stream);
}

void S9xSDL3SoundDriver::stop()
{
    SDL_PauseAudioStreamDevice(stream);
}

bool S9xSDL3SoundDriver::open_device(int playback_rate, int buffer_size)
{
    audiospec = {};
    audiospec.freq = playback_rate;
    audiospec.channels = 2;
    audiospec.format = SDL_AUDIO_S16;

    printf("SDL sound driver initializing...\n");

    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                       &audiospec,
                                       [](void *userdata, SDL_AudioStream *stream, int addl, int total) {
        ((S9xSDL3SoundDriver *)userdata)->mix(addl, total);
    }, this);

    printf("OK\n");
    if (buffer_size < 32)
        buffer_size = 32;

    buffer.resize(buffer_size * 2 * audiospec.freq / 1000);

    return true;
}

int S9xSDL3SoundDriver::space_free()
{
    auto space_empty = buffer.space_empty();
    return space_empty;
}

std::pair<int, int> S9xSDL3SoundDriver::buffer_level()
{
    std::pair<int, int> level = { buffer.space_empty(), buffer.buffer_size };
    return level;
}
