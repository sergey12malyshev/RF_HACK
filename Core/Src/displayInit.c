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
#include "cli_driver.h"

LCD_Handler *lcd = NULL;     //Указатель на первый дисплей в списке

XPT2046_Handler touch1;

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

  /* ----------------------------------- Настройка тачскрина ------------------------------------------*/
  //Будем обмениваться данными с XPT2046 на скорости 2.625 Мбит/с (по спецификации максимум 2.0 Мбит/с).
XPT2046_ConnectionData cnt_touch = { .spi    = SPI1,   //используемый spi
                                   .speed    = 4,        //Скорость spi 0...7 (0 - clk/2, 1 - clk/4, ..., 7 - clk/256)
                     .cs_port  = T_CS_GPIO_Port,  //Порт для управления T_CS
                     .cs_pin    = T_CS_Pin,       //Вывод порта для управления T_CS
                     .irq_port = T_IRQ_GPIO_Port, //Порт для управления T_IRQ
                     .irq_pin  = T_IRQ_Pin,       //Вывод порта для управления T_IRQ
                     .exti_irq = T_IRQ_EXTI_IRQn  //Канал внешнего прерывания
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

void calibrateTouchEnable(void)
{
  XPT2046_CalibrateTouch(&touch1, lcd); //Запускаем процедуру калибровки

  char b[100];
  convert64bit_to_hex((uint8_t*)(&touch1.coef.D), b);
  LCD_WriteString(lcd, 0, 0, b, &Font_12x20, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
  debugPrintf(b);
  convert64bit_to_hex((uint8_t*)(&touch1.coef.Dx1), b);
  LCD_WriteString(lcd, 0, 20, b, &Font_12x20, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
  debugPrintf(b);
  convert64bit_to_hex((uint8_t*)(&touch1.coef.Dx2), b);
  LCD_WriteString(lcd, 0, 40, b, &Font_12x20, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
  debugPrintf(b);
  convert64bit_to_hex((uint8_t*)(&touch1.coef.Dx3), b);
  LCD_WriteString(lcd, 0, 60, b, &Font_12x20, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
  debugPrintf(b);
  convert64bit_to_hex((uint8_t*)(&touch1.coef.Dy1), b);
  LCD_WriteString(lcd, 0, 80, b, &Font_12x20, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
  debugPrintf(b);
  convert64bit_to_hex((uint8_t*)(&touch1.coef.Dy2), b);
  LCD_WriteString(lcd, 0, 100, b, &Font_12x20, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
  debugPrintf(b);
  convert64bit_to_hex((uint8_t*)(&touch1.coef.Dy3), b);
  LCD_WriteString(lcd, 0, 120, b, &Font_12x20, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
  debugPrintf(b);
  while(1) {} //Выведем коэффициенты на дисплей и в консоль
}



