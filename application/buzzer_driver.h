#pragma once
#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "stm32f4xx_ll_gpio.h"

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#define BUZZ_SOUND_FAST     3U
#define BUZZ_SOUND_TEST     15U

bool buzzer_init(GPIO_TypeDef *GPIOx, uint32_t PinMask);
void buzzer_soundOn(uint32_t time);

PT_THREAD(Buzzer_Thread(struct pt *pt));

#endif /*__BUZZER_H__ */