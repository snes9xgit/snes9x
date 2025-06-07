/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once
#include "SDL3/SDL.h"
// SDL.h may include altivec.h which redefines vector and bool
#undef vector
#undef bool

#include "s9x_sound_driver.hpp"
#include "../../apu/resampler.h"

#include <mutex>
#include <vector>

class S9xSDL3SoundDriver : public S9xSoundDriver
{
  public:
    S9xSDL3SoundDriver();
    ~S9xSDL3SoundDriver() override;
    void init() override;
    void deinit() override;
    bool open_device(int playback_rate, int buffer_size) override;
    void start() override;
    void stop() override;
    bool write_samples(int16_t *data, int samples) override;
    int space_free() override;
    std::pair<int, int> buffer_level() override;

  private:
    void mix(int req, int total);

    SDL_AudioStream *stream;
    SDL_AudioSpec audiospec;
    Resampler buffer;
    std::mutex mutex;
    std::vector<int16_t> tmp;
};