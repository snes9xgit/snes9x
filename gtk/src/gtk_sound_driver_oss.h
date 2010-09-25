#ifndef __GTK_SOUND_DRIVER_OSS_H
#define __GTK_SOUND_DRIVER_OSS_H

#include "gtk_sound.h"
#include "gtk_sound_driver.h"

class S9xOSSSoundDriver : public S9xSoundDriver
{
    public:
        S9xOSSSoundDriver (void);
        void init (void);
        void terminate (void);
        bool8 open_device (void);
        void start (void);
        void stop (void);
        void mix (void);
        void samples_available (void);

    private:
        int filedes;
        uint8 *sound_buffer;
        int sound_buffer_size;
};



#endif /* __GTK_SOUND_DRIVER_OSS_H */
