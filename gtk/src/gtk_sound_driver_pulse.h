/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_SOUND_DRIVER_PULSE_H
#define __GTK_SOUND_DRIVER_PULSE_H

#include "gtk_sound.h"
#include "gtk_sound_driver.h"
#include "pulse/pulseaudio.h"

class S9xPulseSoundDriver : public S9xSoundDriver
{
  public:
    S9xPulseSoundDriver();
    void init() override;
    void deinit() override;
    void write_samples(int16_t *data, int samples) override;
    bool open_device(int playback_rate, int buffer_size) override;
    void start() override;
    void stop() override;
    int space_free() override;
    std::pair<int, int> buffer_level() override;
    pa_threaded_mainloop *mainloop;
    pa_context *context;
    pa_stream *stream;

  private:
    void lock();
    void unlock();
    void wait();

    int buffer_size;
};

#endif /* __GTK_SOUND_DRIVER_PULSE_H */
