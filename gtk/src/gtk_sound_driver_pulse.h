#ifndef __GTK_SOUND_DRIVER_PULSE_H
#define __GTK_SOUND_DRIVER_PULSE_H

#include "gtk_sound.h"
#include "gtk_sound_driver.h"
#include "pulse/simple.h"
#include "pulse/error.h"

class S9xPulseSoundDriver : public S9xSoundDriver
{
    public:
        S9xPulseSoundDriver (void);
        void init (void);
        void terminate (void);
        bool8 open_device (void);
        void start (void);
        void stop (void);
        void mix (void);
        void samples_available (void);

    private:
        pa_simple *pulse;
        uint8 *buffer;
        int buffer_size;

};

#endif /* __GTK_SOUND_DRIVER_PULSE_H */
