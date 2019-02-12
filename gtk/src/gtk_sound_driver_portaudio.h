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
    void init();
    void terminate();
    bool open_device();
    void start();
    void stop();
    void samples_available();

  private:
    PaStream *audio_stream;
    int sound_buffer_size;
    uint8 *sound_buffer;
    int output_buffer_size;
};

#endif /* __GTK_SOUND_DRIVER_PORTAUDIO_H */
