#ifndef __GTK_SOUND_DRIVER_H
#define __GTK_SOUND_DRIVER_H

#include "gtk_s9x.h"

class S9xSoundDriver
{
    public:
        virtual ~S9xSoundDriver () {}
        virtual void init () = 0;
        virtual void terminate () = 0;
        virtual bool8 open_device () = 0;
        virtual void start () = 0;
        virtual void stop () = 0;
};


#endif /* __GTK_SOUND_DRIVER_H */
