#ifndef DISPLAY_INIT_H__
#define DISPLAY_INIT_H__

#include "display.h"
//https://microsin.net/programming/arm-troubleshooting-faq/how-use-extern.html
extern LCD_DMA_TypeDef dma_tx_1;
extern LCD_BackLight_data bkl_data; 

extern LCD_Handler *lcd;     //Указатель на первый дисплей в списке

void convert64bit_to_hex(uint8_t *v, char *b);

#endif // DISPLAY_INIT_H__

