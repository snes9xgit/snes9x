/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "s9x_sound_driver_sdl.hpp"
#include "SDL_audio.h"

void S9xSDLSoundDriver::write_samples(int16_t *data, int samples)
{
    mutex.lock();
    if (samples > buffer.space_empty())
        samples = buffer.space_empty();
    buffer.push(data, samples);
    mutex.unlock();
}

void S9xSDLSoundDriver::mix(unsigned char *output, int bytes)
{
    mutex.lock();
    if (buffer.avail() >= bytes >> 1)
        buffer.read((int16_t *)output, bytes >> 1);
    mutex.unlock();
}

S9xSDLSoundDriver::S9xSDLSoundDriver()
{
}

S9xSDLSoundDriver::~S9xSDLSoundDriver()
{
    deinit();
}

void S9xSDLSoundDriver::init()
{
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    stop();
}

void S9xSDLSoundDriver::deinit()
{
    stop();
    SDL_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void S9xSDLSoundDriver::start()
{
    SDL_PauseAudio(0);
}

void S9xSDLSoundDriver::stop()
{
    SDL_PauseAudio(1);
}

bool S9xSDLSoundDriver::open_device(int playback_rate, int buffer_size)
{
    audiospec = {};
    audiospec.freq = playback_rate;
    audiospec.channels = 2;
    audiospec.format = AUDIO_S16SYS;
    audiospec.samples = audiospec.freq * 4 / 1000; // 4ms per sampling
    audiospec.callback = [](void *userdata, uint8_t *stream, int len) {
        ((S9xSDLSoundDriver *)userdata)->mix((unsigned char *)stream, len);
    };

    audiospec.userdata = this;

    printf("SDL sound driver initializing...\n");
    printf("    --> (Frequency: %dhz, Latency: %dms)...",
           audiospec.freq,
           (audiospec.samples * 1000 / audiospec.freq));

    if (SDL_OpenAudio(&audiospec, NULL) < 0)
    {
        printf("Failed\n");
        return false;
    }

    printf("OK\n");

    buffer.resize(buffer_size * audiospec.freq / 1000);

    return true;
}

int S9xSDLSoundDriver::space_free()
{
    mutex.lock();
    auto space_empty = buffer.space_empty();
    mutex.unlock();
    return space_empty;
}

std::pair<int, int> S9xSDLSoundDriver::buffer_level()
{
    mutex.lock();
    std::pair<int, int> level = { buffer.space_empty(), buffer.buffer_size };
    mutex.unlock();
    return level;
}