/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __S9X_SOUND_DRIVER_SDL_HPP
#define __S9X_SOUND_DRIVER_SDL_HPP

#include "SDL.h"
// SDL.h may include altivec.h which redefines vector and bool
#undef vector
#undef bool

#include "s9x_sound_driver.hpp"
#include "../../apu/resampler.h"

#include <mutex>
#include <cstdint>

class S9xSDLSoundDriver : public S9xSoundDriver
{
  public:
    S9xSDLSoundDriver();
    ~S9xSDLSoundDriver();
    void init() override;
    void deinit() override;
    bool open_device(int playback_rate, int buffer_size) override;
    void start() override;
    void stop() override;
    void write_samples(int16_t *data, int samples) override;
    int space_free() override;
    std::pair<int, int> buffer_level() override;

  private:
    void mix(unsigned char *output, int bytes);

    SDL_AudioSpec audiospec;
    Resampler buffer;
    std::mutex mutex;
    int16_t temp[512];
};

#endif /* __S9X_SOUND_DRIVER_SDL_HPP */
