#pragma once

#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "stm32f4xx_ll_gpio.h"

void buzzer_enable(void);
void buzzer_disable(void);

bool buzzer_init(GPIO_TypeDef *GPIOx, uint32_t PinMask);

#endif /*__BUZZER_H__ */