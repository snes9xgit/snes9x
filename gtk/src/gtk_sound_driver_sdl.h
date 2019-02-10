/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_SOUND_DRIVER_SDL_H
#define __GTK_SOUND_DRIVER_SDL_H

#include "SDL.h"

#include "gtk_sound.h"
#include "gtk_sound_driver.h"
#include "../../apu/resampler.h"

#include <mutex>
#include <cstdint>

class S9xSDLSoundDriver : public S9xSoundDriver
{
  public:
    S9xSDLSoundDriver();
    void init();
    void terminate();
    bool open_device();
    void start();
    void stop();
    void mix(unsigned char *output, int bytes);
    void samples_available();

  private:
    SDL_AudioSpec *audiospec;
    Resampler *buffer;
    std::mutex mutex;
    int16_t temp[512];
};

#endif /* __GTK_SOUND_DRIVER_SDL_H */
