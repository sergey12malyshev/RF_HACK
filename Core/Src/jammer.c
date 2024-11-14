#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#include "main.h"
#include "jammer.h"
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

extern volatile uint8_t GDO0_FLAG;


/*
 * Протопоток jammer_Thread
 *
 */
PT_THREAD(jammer_Thread(struct pt *pt))
{
    static uint32_t timer1;
    __attribute__((unused)) uint8_t s;


    PT_BEGIN(pt);

    PT_DELAY_MS(pt, &timer1, 250);

    screen_clear();
    LCD_WriteString(lcd, 0, 0, "Jammer mode", &Font_8x13, COLOR_RED, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
    LCD_WriteString(lcd, 0, 30, "Push the encoder!", &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_12); //GDO
    NVIC_EnableIRQ(EXTI15_10_IRQn); //GDO

    CC1101_reinit();

    setTime(&timer1);

    CC1101_setMHZ(LPD17 - DIFFERENCE_WITH_CARRIER);

    GDO0_FLAG = 0;

    encoder_init();

    while (1)
    {
      /*You can connect only GDO0, if you are using asynchronous serial mode. 
      The pin will switch automatically from INPUT to OUTPUT when you call setTX() and vice versa.*/
      static bool runJamm;
      bool stateSwitch = encoder_getStateSwitch();

      if (stateSwitch)
      {
        runJamm = !runJamm;
        if(runJamm)
        {
          LCD_WriteString(lcd, 55, 100, "TX", &Font_12x20, COLOR_WHITE, COLOR_RED, LCD_SYMBOL_PRINT_FAST);
        }
        else
        {
          LCD_WriteString(lcd, 55, 100, "no", &Font_12x20, COLOR_WHITE, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
        }
      }


      static char packet[7] = "om5q3z"; // Резерв одного символа под нуль-терминатор!!

      static uint8_t i;
      packet[i++] = generateRandomChar();

      if (i > 5) i = 0;

      if (runJamm)
      {
        //debugPrintf("%s %d"CLI_NEW_LINE, packet, packet[i]);

        s = CC1101_transmittRF(packet, sizeof(packet)); // sending the data
        LCD_WriteString(lcd, 15, 65, packet, &Font_12x20, COLOR_RED, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
      
        PT_WAIT_UNTIL(pt, (GDO0_FLAG)); //GDO low lowel - end transmitt
        GDO0_FLAG = 0;
      }

      PT_YIELD(pt);
    }

    PT_END(pt);
}