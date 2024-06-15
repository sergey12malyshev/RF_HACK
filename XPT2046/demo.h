/*
 *  Copyright (C) 2019, VadRov, all right reserved.
 *
 *  Демки работы с контроллером XPT2046
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

#ifndef DEMO_H_
#define DEMO_H_

#include "xpt2046.h"
#include "calibrate_touch.h"
#include "display.h"

/* Демка рисования на дисплее с помощью тачскрина (возможность смены цвета пера) */
void Draw_TouchPenDemo(XPT2046_Handler *t, LCD_Handler *lcd);

/*
 * Демка рисует примитивы случайного размера в случайной области дисплея, отображает
 * температуру и позволяет перемещать окружность белого цвета по дисплею.
 * При удержании касания дисплей окрашивается случайным цветом
 */
void RoadCircleDemo(XPT2046_Handler *t, LCD_Handler *lcd);

#endif /* DEMO_H_ */
