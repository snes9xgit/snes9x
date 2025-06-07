/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once
#include "s9x_sound_driver.hpp"
#include <cstdint>
#include "cubeb/cubeb.h"
#include "../../apu/resampler.h"

class S9xCubebSoundDriver : public S9xSoundDriver
{
  public:
    ~S9xCubebSoundDriver() override;
    void init() override;
    void deinit() override;
    bool open_device(int playback_rate, int buffer_size) override;
    void start() override;
    void stop() override;
    long data_callback(cubeb_stream *stream, void const *input_buffer, void *output_buffer, long nframes);
    bool write_samples(int16_t *data, int samples) override;
    int space_free() override;
    std::pair<int, int> buffer_level() override;

  private:
    Resampler buffer;
    cubeb *context = nullptr;
    cubeb_stream *stream = nullptr;
};