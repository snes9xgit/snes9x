#ifndef __GTK_SOUND_DRIVER_PORTAUDIO_H
#define __GTK_SOUND_DRIVER_PORTAUDIO_H

#include <portaudio.h>
#include <errno.h>

#include "gtk_sound.h"
#include "gtk_sound_driver.h"

class S9xPortAudioSoundDriver : public S9xSoundDriver
{
    public:
        S9xPortAudioSoundDriver (void);
        void init (void);
        void terminate (void);
        bool8 open_device (void);
        void start (void);
        void stop (void);
        void samples_available (void);

    private:
        PaStream *audio_stream;
        int sound_buffer_size;
        uint8 *sound_buffer;
        int output_buffer_size;
};


#endif /* __GTK_SOUND_DRIVER_PORTAUDIO_H */
