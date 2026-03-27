#ifndef BUZZER_H_
#define BUZZER_H_

#include "hw_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _USE_HW_BUZZER

void buzzerInit(void);
void buzzerOn(void);
void buzzerOff(void);
void buzzerBeep(uint32_t ms);
void buzzerTone(uint32_t freq, uint16_t duty); // 원하는 주파수, 듀티(0~1000)
bool buzzerIsOn(void);

#endif

#ifdef __cplusplus
}
#endif

#endif // BUZZER_H_
