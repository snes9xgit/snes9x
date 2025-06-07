/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once
#include <cstdint>
#include <tuple>

class S9xSoundDriver
{
  public:
    virtual ~S9xSoundDriver() = default;
    virtual bool write_samples(int16_t *data, int samples) = 0;
    virtual int space_free() = 0;
    virtual std::pair<int, int> buffer_level() = 0;
    virtual void init() = 0;
    virtual void deinit() = 0;
    virtual bool open_device(int playback_rate, int buffer_size) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};