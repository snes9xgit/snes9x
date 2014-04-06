#ifndef __GTK_SOUND_DRIVER_H
#define __GTK_SOUND_DRIVER_H

#include "gtk_s9x.h"

class S9xSoundDriver
{
    public:
        virtual ~S9xSoundDriver () {}
        virtual void init (void) = 0;
        virtual void terminate (void) = 0;
        virtual bool8 open_device (void) = 0;
        virtual void start (void) = 0;
        virtual void stop (void) = 0;
        virtual void mix (void) = 0;
};


#endif /* __GTK_SOUND_DRIVER_H */
