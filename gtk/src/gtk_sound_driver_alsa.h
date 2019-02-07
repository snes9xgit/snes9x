/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_SOUND_DRIVER_ALSA_H
#define __GTK_SOUND_DRIVER_ALSA_H

#include "gtk_sound.h"
#include "gtk_sound_driver.h"
#include "alsa/asoundlib.h"

class S9xAlsaSoundDriver : public S9xSoundDriver
{
  public:
    S9xAlsaSoundDriver();
    void init();
    void terminate();
    bool open_device();
    void start();
    void stop();
    void samples_available();

  private:
    snd_pcm_t *pcm;
    int sound_buffer_size;
    uint8 *sound_buffer;
    int output_buffer_size;
};

#endif /* __GTK_SOUND_DRIVER_ALSA_H */
