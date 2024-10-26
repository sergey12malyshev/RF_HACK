#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#include "main.h"
#include "adc.h"
#include "application_task.h"
#include "buttonDisplay.h"
#include "gpio.h"
#include "cli_driver.h"

#include "display.h"
#include "ili9341.h"
#include "xpt2046.h"
#include "calibrate_touch.h"
#include "demo.h"
#include "subGHz_RX_Thread.h"

#include "gps.h"
#include "time.h"

extern LCD_Handler *lcd;
extern XPT2046_Handler touch1;
extern GPS_t GPS;
extern RF_t CC1101;


static void bootingScreen(void)
{
  char str[100] = {0};
  uint16_t y = 0;
  const uint8_t shift = 25;

  LCD_Fill(lcd, COLOR_BLACK);

  LCD_WriteString(lcd, 0, y+=shift, "RF HACK 2024",
            &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
 
  LCD_WriteString(lcd, 0, y+=shift, "Prototreads ",
            &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
  
  sprintf(str, "HAL: %lu", HAL_GetHalVersion());
  LCD_WriteString(lcd, 0, y+=shift, str,
            &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  LCD_WriteString(lcd, 0, y+=shift, "Data build: "__DATE__,
            &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  LCD_WriteString(lcd, 0, y+=shift, "Time build: "__TIME__,
            &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
  
  LCD_WriteString(lcd, 0, y+=shift, "Version SW:"quoting(SOFTWARE_VERSION_MAJOR)"."quoting(SOFTWARE_VERSION_MINOR)"."quoting(SOFTWARE_VERSION_PATCH),
            &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

}

static void GPS_DataScreen(void)
{
  uint16_t start_x = 10;
  uint16_t start_y = 0;

  uint16_t offset_y = 16;

  char str[100] = "";
  sprintf(str, "Utc time: %.2f", GPS.utc_time);
  LCD_WriteString(lcd, start_x, offset_y + start_y, str, &Font_8x13, COLOR_YELLOW, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  sprintf(str, "Longitude: %.4f", GPS.dec_longitude);
  LCD_WriteString(lcd, start_x, offset_y*2 + start_y, str, &Font_8x13, COLOR_YELLOW, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  sprintf(str, "Latitude: %.4f", GPS.dec_latitude);
  LCD_WriteString(lcd, start_x, offset_y*3 + start_y, str, &Font_8x13, COLOR_YELLOW, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  sprintf(str, "Altitude_ft: %.4f",  GPS.msl_altitude);
  LCD_WriteString(lcd, start_x, offset_y*4 + start_y, str, &Font_8x13, COLOR_YELLOW, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
  
  sprintf(str, "Satelites: %d",  GPS.satelites);
  LCD_WriteString(lcd, start_x, offset_y*5 + start_y, str, &Font_8x13, COLOR_YELLOW, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
}

static void CC1101_DataScreen(void)
{
  uint16_t start_x = 10;
  uint16_t start_y = 100;

  uint16_t offset_y = 16;

  char str[100] = "";
  sprintf(str, "Count mesage: %d", CC1101.countMessage);
  LCD_WriteString(lcd, start_x, offset_y + start_y, str, &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  sprintf(str, "Count error: %d", CC1101.countError);
  LCD_WriteString(lcd, start_x, offset_y*2 + start_y, str, &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  sprintf(str, "RCCI: %d dBm", CC1101.RSSI);
  LCD_WriteString(lcd, start_x, offset_y*3 + start_y, str, &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  sprintf(str, "Message: %s", CC1101.dataString);
  LCD_WriteString(lcd, start_x, offset_y*4 + start_y, str, &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  sprintf(str, "RSSI M: %ld dBm", CC1101.RSSI_main);
  LCD_WriteString(lcd, start_x, offset_y*5 + start_y, str, &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
}

static void screen_voltage(uint32_t voltage)
{
  char str[100] = "";
  sprintf(str, "%lu.%luV", voltage/1000, voltage%1000);
  LCD_WriteString(lcd, 185, 5, str, &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
}

/*
 * Протопоток StartApplication_Thread
 *
 * 
 */

PT_THREAD(StartApplication_Thread(struct pt *pt))
{
  static uint32_t timer1;

  PT_BEGIN(pt);
  
  bootingScreen();

  PT_DELAY_MS(pt, &timer1, 1800);

  LCD_Fill(lcd, COLOR_BLACK);
  GPS_Init();
  LCD_WriteString(lcd, 0, 0, "GPS Data:", &Font_8x13, COLOR_YELLOW, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
  LCD_WriteString(lcd, 0, 100, "CC1101 Data:", &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
  setTime(&timer1);

  while (1)
  {
    PT_WAIT_UNTIL(pt, timer(&timer1, 350));
    
    heartbeatLedToggle();

    adcConvertProcess();
    screen_voltage(getVoltageVDDA());

#define TEST_COUNT   false
#if TEST_COUNT
    char str[100] = "   N = ";
    static uint16_t i;
    utoa(i++, &str[7], 10);
    strcat(str, " count    ");
    LCD_WriteString(lcd, 15, 15, str, &Font_8x13, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
#endif
    GPS_DataScreen();
    CC1101_DataScreen();

    PT_YIELD(pt);
  }

  PT_END(pt);
}
