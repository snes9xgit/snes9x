/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_SOUND_DRIVER_H
#define __GTK_SOUND_DRIVER_H

#include "gtk_s9x.h"

class S9xSoundDriver
{
  public:
    virtual ~S9xSoundDriver()
    {
    }
    virtual void init() = 0;
    virtual void terminate() = 0;
    virtual bool open_device() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};

#endif /* __GTK_SOUND_DRIVER_H */
