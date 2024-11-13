#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#include "main.h"
#include "adc.h"
#include "application_Thread.h"
#include "button_Thread.h"
#include "gpio.h"
#include "cli_driver.h"

#include "display.h"
#include "displayInit.h"
#include "ili9341.h"
#include "xpt2046.h"
#include "calibrate_touch.h"
#include "demo.h"
#include "subGHz_RX_Thread.h"

#include "encoderDriver.h"

#include "gps.h"
#include "time.h"

extern RF_t CC1101;

static bool bootingScreenMode = true;

bool getBootingScreenMode(void)
{
  return bootingScreenMode;
}

void screen_clear(void)
{
  LCD_FillWindow(lcd, 0, 0, 175, 320 - 1, COLOR_BLACK);
}

static void screen_booting(void)
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

static void screen_voltage(uint32_t voltage)
{
  char str[100] = "";
  sprintf(str, "%1lu.%03luV", voltage/1000, voltage%1000);

  uint32_t color = COLOR_WHITE;
  if(voltage < 3260)
  {
    color = COLOR_RED;
  }
  LCD_WriteString(lcd, 185, 5, str, &Font_8x13, color, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
}

void screen_bootload(void)
{
  LCD_Fill(lcd, COLOR_BLACK);
  LCD_WriteString(lcd, 5, 35, "Bootload run...", &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
}

/*
 * Протопоток StartApplication_Thread
 *
 */

PT_THREAD(StartApplication_Thread(struct pt *pt))
{
  static uint32_t timer1;

  PT_BEGIN(pt);
  
  screen_booting();

  PT_DELAY_MS(pt, &timer1, 2500);

  LCD_Fill(lcd, COLOR_BLACK);

  setTime(&timer1);

  bootingScreenMode = false;
  
  while (1)
  {
    PT_WAIT_UNTIL(pt, timer(&timer1, 350));
    
    encoder_process();

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

    PT_YIELD(pt);
  }

  PT_END(pt);
}
