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
    void init();
    void terminate();
    bool open_device();
    void start();
    void stop();
    void samples_available();

  private:
    int filedes;
    uint8 *sound_buffer;
    int sound_buffer_size;
    int output_buffer_size;
};

#endif /* __GTK_SOUND_DRIVER_OSS_H */
