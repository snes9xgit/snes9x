/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_SOUND_DRIVER_PORTAUDIO_H
#define __GTK_SOUND_DRIVER_PORTAUDIO_H

#include <errno.h>
#include <portaudio.h>

#include "gtk_sound.h"
#include "gtk_sound_driver.h"

class S9xPortAudioSoundDriver : public S9xSoundDriver
{
  public:
    S9xPortAudioSoundDriver();
    void init() override;
    void deinit() override;
    bool open_device(int playback_rate, int buffer_size) override;
    void start() override;
    void stop() override;
    void write_samples(int16_t *data, int samples) override;
    int space_free() override;
    std::pair<int, int> buffer_level() override;
    void samples_available();

  private:
    PaStream *audio_stream;
    int output_buffer_size;
};

#endif /* __GTK_SOUND_DRIVER_PORTAUDIO_H */
