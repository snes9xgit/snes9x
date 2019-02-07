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
    void init();
    void terminate();
    bool open_device();
    void start();
    void stop();
    void samples_available();
    void lock();
    void unlock();
    void wait();

    pa_threaded_mainloop *mainloop;
    pa_context *context;
    pa_stream *stream;

  private:
    int buffer_size;
};

#endif /* __GTK_SOUND_DRIVER_PULSE_H */
