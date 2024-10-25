/*
 *  Copyright (C) 2019, VadRov, all right reserved.
 *
 *  Калибровка тачскрина по 5 точкам
 *
 *  Допускается свободное распространение без целей коммерческого использования.
 *  При коммерческом использовании необходимо согласование с автором.
 *  Распространятся по типу "как есть", то есть использование осуществляете на свой страх и риск.
 *  Автор не предоставляет никаких гарантий.
 *
 *  https://www.youtube.com/@VadRov
 *  https://dzen.ru/vadrov
 *  https://vk.com/vadrov
 *  https://t.me/vadrov_channel
 *
 */

#ifndef CALIBRATE_TOUCH_H_
#define CALIBRATE_TOUCH_H_

#include "main.h"
#include "display.h"
#include "xpt2046.h"

/* Калибровка тачскрина по 5 точкам */
void XPT2046_CalibrateTouch(XPT2046_Handler *t, LCD_Handler *lcd);

/* Преобразование координат тачскрина в дисплейные координаты */
void XPT2046_ConvertPoint(tPoint *p_display, volatile tPoint *p_touch, tCoef *coef);

#endif /* CALIBRATE_TOUCH_H_ */
