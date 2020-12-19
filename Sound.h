// Sound.h
// Runs on TM4C123 or LM4F120
// Tim Reynolds
// 5/5/2020
#ifndef __SOUND_H__ // do not include more than once
#define __SOUND_H__
#include <stdint.h>

void SoundTask(void);
void Sound_Init(void);
void Sound_Play(const uint8_t *pt, uint32_t count);
void Sound_Gun(void);
void Sound_Footstep(void);
void Sound_Zombie(void);

#endif


