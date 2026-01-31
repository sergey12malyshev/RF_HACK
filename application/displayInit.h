#pragma once
#ifndef DISPLAY_INIT_H__
#define DISPLAY_INIT_H__

#include "display.h"
#include "xpt2046.h"
//https://microsin.net/programming/arm-troubleshooting-faq/how-use-extern.html

extern LCD_BackLight_data bkl_data; 
extern XPT2046_ConnectionData cnt_touch;

extern LCD_Handler *lcd;     //Указатель на первый дисплей в списке
extern XPT2046_Handler touch1;

void convert64bit_to_hex(uint8_t *v, char *b);
void calibrateTouchEnable(void);

LCD_SPI_Connected_data * displayInit_getSpiPortSettings(void);
tCoef * displayInit_getCalibrationCoefficient(void);

#endif // DISPLAY_INIT_H__

