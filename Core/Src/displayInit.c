#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "displayInit.h"
#include "main.h"
#include "display.h"
#include "ili9341.h"
#include "xpt2046.h"
#include "calibrate_touch.h"

LCD_Handler *lcd = NULL;     //Указатель на первый дисплей в списке

//Данные DMA
LCD_DMA_TypeDef dma_tx_1 = 
{ 
  .dma    = DMA2,           // Контроллер DMA
  .stream = LL_DMA_STREAM_3 // Поток контроллера DMA
};  

  //Данные подсветки
LCD_BackLight_data bkl_data = 
{
  .htim_bk        = TIM3,       // Таймер - для подсветки с PWM (изменение яркости подсветки)
  .channel_htim_bk = LL_TIM_CHANNEL_CH1, // Канал таймера - для подсветки с PWM (изменение яркости подсветки)
  .blk_port       = 0,          // Порт gpio - подсветка по типу вкл./выкл.
  .blk_pin        = 0,          // Вывод порта - подсветка по типу вкл./выкл.
  .bk_percent     = 60          // Яркость подсветки, в %
};     


/* Для тех, кто не умеет пользоваться отладчиком или
тех, у кого он не работает */

void convert64bit_to_hex(uint8_t *v, char *b)
{
  b[0] = 0;
  sprintf(&b[strlen(b)], "0x");
  for (int i = 0; i < 8; i++) 
  {
    sprintf(&b[strlen(b)], "%02x", v[7 - i]);
  }
}




