#ifndef __GTK_SOUND_DRIVER_PULSE_H
#define __GTK_SOUND_DRIVER_PULSE_H

#include "gtk_sound.h"
#include "gtk_sound_driver.h"
#include "pulse/pulseaudio.h"

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
        void lock (void);
        void unlock (void);
        void wait (void);

        pa_threaded_mainloop *mainloop;
        pa_context *context;
        pa_stream *stream;

    private:
        int buffer_size;

};

#endif /* __GTK_SOUND_DRIVER_PULSE_H */
