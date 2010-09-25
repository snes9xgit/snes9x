#ifndef __GTK_SOUND_DRIVER_ALSA_H
#define __GTK_SOUND_DRIVER_ALSA_H

#include "gtk_sound.h"
#include "gtk_sound_driver.h"
#include "alsa/asoundlib.h"

class S9xAlsaSoundDriver : public S9xSoundDriver
{
    public:
        S9xAlsaSoundDriver (void);
        void init (void);
        void terminate (void);
        bool8 open_device (void);
        void start (void);
        void stop (void);
        void mix (void);
        void samples_available (void);

    private:
        snd_pcm_t *pcm;
        int sound_buffer_size;
        uint8 *sound_buffer;
};

#endif /* __GTK_SOUND_DRIVER_ALSA_H */
