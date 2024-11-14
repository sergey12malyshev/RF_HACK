#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#include "main.h"
#include "gps_Thread.h"
#include "cli_driver.h"
#include "application_Thread.h"

#include "display.h"
#include "displayInit.h"
#include "ili9341.h"
#include "xpt2046.h"
#include "calibrate_touch.h"
#include "demo.h"

#include "gps.h"
#include "time.h"


static void GPS_DataScreen(void)
{
  uint16_t start_x = 10;
  uint16_t start_y = 0;

  uint16_t offset_y = 18;

  char str[100] = "";
  sprintf(str, "Utc time: %.2f", GPS.utc_time);
  LCD_WriteString(lcd, start_x, offset_y + start_y, str, &Font_8x13, COLOR_YELLOW, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  sprintf(str, "Longitude: %.4f", GPS.dec_longitude);
  LCD_WriteString(lcd, start_x, offset_y*2 + start_y, str, &Font_8x13, COLOR_YELLOW, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  sprintf(str, "Latitude: %.4f", GPS.dec_latitude);
  LCD_WriteString(lcd, start_x, offset_y*3 + start_y, str, &Font_8x13, COLOR_YELLOW, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  sprintf(str, "Altitude_ft: %.4f",  GPS.msl_altitude);
  LCD_WriteString(lcd, start_x, offset_y*4 + start_y, str, &Font_8x13, COLOR_YELLOW, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  sprintf(str, "Speed km: %f",  GPS.speed_km);
  LCD_WriteString(lcd, start_x, offset_y*5 + start_y, str, &Font_8x13, COLOR_YELLOW, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
  
  sprintf(str, "Satelites: %d",  GPS.satelites);
  LCD_WriteString(lcd, start_x, offset_y*6 + start_y, str, &Font_8x13, COLOR_YELLOW, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
}

/*
 * Протопоток gps_Thread
 *
 * Вывод данных от модуля Gps Ublox Neo-6M
 */

PT_THREAD(gps_Thread(struct pt *pt))
{
  static uint32_t timer1;

  PT_BEGIN(pt);
  

  PT_DELAY_MS(pt, &timer1, 250);

  screen_clear();

  GPS_Init();
  LCD_WriteString(lcd, 0, 0, "GPS Data:", &Font_8x13, COLOR_YELLOW, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
  setTime(&timer1);

  while (1)
  {
    PT_WAIT_UNTIL(pt, timer(&timer1, 350));
    
    GPS_DataScreen();

    PT_YIELD(pt);
  }

  PT_END(pt);
}
