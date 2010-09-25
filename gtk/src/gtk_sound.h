#ifndef __GTK_SOUND_H
#define __GTK_SOUND_H

void S9xPortSoundInit (void);
void S9xPortSoundDeinit (void);
void S9xPortSoundReinit (void);
void S9xSoundStart (void);
void S9xSoundStop (void);
void S9xMixSound (void);

int S9xSoundBase2log (int num);
int S9xSoundPowerof2 (int num);

extern int playback_rates[8];
extern int buffer_sizes[8];
extern double d_playback_rates[8];

#endif /* __GTK_SOUND_H */
