#ifndef __GTK_SOUND_H
#define __GTK_SOUND_H

void S9xPortSoundInit (void);
void S9xPortSoundDeinit (void);
void S9xPortSoundReinit (void);
void S9xSoundStart (void);
void S9xSoundStop (void);

int S9xSoundBase2log (int num);
int S9xSoundPowerof2 (int num);

#endif /* __GTK_SOUND_H */
