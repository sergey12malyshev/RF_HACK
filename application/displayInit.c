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
#include "iwdg.h"

LCD_Handler *lcd = NULL;     // Pointer to the first display in the list

XPT2046_Handler touch1;

//Backlight Data
LCD_BackLight_data bkl_data = 
{
  .htim_bk        = TIM3,       // Timer - for backlight with PWM (changing the backlight brightness)
  .channel_htim_bk = LL_TIM_CHANNEL_CH1, // Timer channel - for backlight with PWM (changing backlight brightness)
  .blk_port       = 0,          // GPIO port - on/off backlight
  .blk_pin        = 0,          // The port output is an on/off backlight
  .bk_percent     = 60          // Backlight brightness, in %
}; 

/*
  Setting up the touchscreen
  We will exchange data with XPT2046 at a speed of 2.625 Mbit/s (according to the specification, a maximum of 2.0 Mbit/s).
*/

XPT2046_ConnectionData cnt_touch = 
{ 
  .spi    = SPI1,              //the SPI interface used
  .speed    = 4,               //Speed SPI 0...7 (0 - clk/2, 1 - clk/4, ..., 7 - clk/256)
  .cs_port  = T_CS_GPIO_Port,  //port T_CS
  .cs_pin    = T_CS_Pin,       //pin T_CS
  .irq_port = T_IRQ_GPIO_Port, //port T_IRQ
  .irq_pin  = T_IRQ_Pin,       //pin T_IRQ
  .exti_irq = T_IRQ_EXTI_IRQn  //exti channel
};

/* Setting up the display */
static LCD_SPI_Connected_data spi_con =     
{ 
  .spi        = SPI1,
  .dma_tx     = {DMA2, LL_DMA_STREAM_3},        // data DMA
  .reset_port = LCD_RESET_GPIO_Port,
  .reset_pin  = LCD_RESET_Pin,
  .dc_port    = LCD_DC_GPIO_Port,
  .dc_pin     = LCD_DC_Pin,
  .cs_port    = LCD_CS_GPIO_Port,
  .cs_pin     = LCD_CS_Pin
};

LCD_SPI_Connected_data * displayInit_getSpiPortSettings(void)
{
  return &spi_con;
}

static tCoef coef = 
{ .D = 0x00022b4253626d37,
  .Dx1 = 0xffffd9e9e85d81b6,
  .Dx2 = 0x0000005a555c98ab,
  .Dx3 = 0x022dd7f0419e66b7,
  .Dy1 = 0xffffff6065e10c98,
  .Dy2 = 0x0000343b820dc8bf,
  .Dy3 = 0xff9cc25725238e55 
};

tCoef * displayInit_getCalibrationCoefficient(void)
{
  return &coef;
}

/* For those who don't know how to use the debugger or
for those who don't have it working. */

void convert64bit_to_hex(uint8_t *v, char *b)
{
  b[0] = 0;
  sprintf(&b[strlen(b)], "0x");
  for (int i = 0; i < 8; i++) 
  {
    sprintf(&b[strlen(b)], "%02x", v[7 - i]);
  }
}

void calibrateTouchEnable(void) //Starting the calibration procedure
{
  XPT2046_CalibrateTouch(&touch1, lcd);

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

  while(true)   //We will display the coefficients on the display and in the console
  { 
    IWDG_reload();
  }
}
