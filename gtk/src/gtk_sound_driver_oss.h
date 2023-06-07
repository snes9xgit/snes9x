/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_SOUND_DRIVER_OSS_H
#define __GTK_SOUND_DRIVER_OSS_H

#include "gtk_sound.h"
#include "gtk_sound_driver.h"

class S9xOSSSoundDriver : public S9xSoundDriver
{
  public:
    S9xOSSSoundDriver();
    void init() override;
    void deinit() override;
    bool open_device(int playback_rate, int buffer_size_ms) override;
    void start() override;
    void stop() override;
    void write_samples(int16_t *data, int samples) override;
    int space_free() override;
    std::pair<int, int> buffer_level() override;

  private:
    int filedes;
    int output_buffer_size_bytes;
};

#endif /* __GTK_SOUND_DRIVER_OSS_H */
