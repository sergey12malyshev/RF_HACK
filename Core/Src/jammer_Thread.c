#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#include "main.h"
#include "jammer_Thread.h"
#include "frequencyChannelsTable.h"

#include "application_Thread.h"
#include "cli_driver.h"
#include "cc1101.h"
#include "time.h"
#include "adc.h"
#include "encoderDriver.h"

#include "display.h"
#include "displayInit.h"
#include "ili9341.h"

extern volatile uint8_t GDO0_flag;

/*
 * Protothread jammer_Thread
 *
 */
PT_THREAD(jammer_Thread(struct pt *pt))
{
  static uint32_t timer1;
  __attribute__((unused)) uint8_t s;
  char str[25] = {0};


  PT_BEGIN(pt);

  PT_DELAY_MS(pt, &timer1, 250);

  screen_clear();
  LCD_WriteString(lcd, 0, 0, "Jammer mode", &Font_8x13, COLOR_RED, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
  LCD_WriteString(lcd, 0, 30, "Push the encoder!", &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  CC1101_GDO0_flag_clear();
  CC1101_reinit();

  setTime(&timer1);

  CC1101_setMHZ(LPD17 - DIFFERENCE_WITH_CARRIER);

  encoder_init();
  encoder_setRotaryNum(16);

  while (1)
  {
      /*You can connect only GDO0, if you are using asynchronous serial mode. 
      The pin will switch automatically from INPUT to OUTPUT when you call setTX() and vice versa.*/
    static bool runJamm;
    bool stateSwitch = encoder_getStateSwitch();

    if (stateSwitch)
    {
      runJamm = !runJamm;
      if (runJamm)
      {
        LCD_WriteString(lcd, 55, 100, "TX", &Font_12x20, COLOR_WHITE, COLOR_RED, LCD_SYMBOL_PRINT_FAST);
      }
      else
      {
        LCD_WriteString(lcd, 55, 100, "no", &Font_12x20, COLOR_WHITE, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
      }
    }


    static char packet[7] = "om5q3z"; // Reserve one character for a null terminator!!

    static uint8_t i;
    packet[i++] = generateRandomChar();

    if (i > 5) 
    {
      i = 0;
    }

    if (runJamm)
    {
#if 0
      debugPrintf("%s %d"CLI_NEW_LINE, packet, packet[i]);
#endif
      s = CC1101_transmittRF(packet, sizeof(packet)); // sending the data
      LCD_WriteString(lcd, 15, 65, packet, &Font_12x20, COLOR_RED, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
      
      PT_WAIT_UNTIL(pt, (GDO0_flag)); // GDO low lowel - end transmitt
      GDO0_flag = 0;
    }
    else
    {
      int16_t num = encoder_getRotaryNum();
      
      if (num < 0)
      {
        num = 0;
        encoder_setRotaryNum(0);
      }
      else
      {
        if (num > 68)
        {
          num = 68;
          encoder_setRotaryNum(68);
        } 
      }

      float freq = freqLpdList[num];
      sprintf(str, "LPD%02d %.3f", num + 1, freq);
      LCD_WriteString(lcd, 10, 175, str, &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);


      CC1101_setMHZ(freq - DIFFERENCE_WITH_CARRIER);

      PT_WAIT_UNTIL(pt, timer(&timer1, 250));
    }

    PT_YIELD(pt);
  }

  PT_END(pt);
}