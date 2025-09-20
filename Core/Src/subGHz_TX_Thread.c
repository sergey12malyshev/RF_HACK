#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#include "main.h"
#include "subGHz_TX_Thread.h"
#include "application_Thread.h"
#include "cli_driver.h"
#include "cc1101.h"
#include "time.h"

#include "display.h"
#include "displayInit.h"
#include "ili9341.h"

extern volatile uint8_t GDO0_flag;

static char packet[7] = "QWERTY";; // Reserve one character for a null terminator!

/*
 * Protothread subGHz_TX_Thread
 *
 *
 */
PT_THREAD(subGHz_TX_Thread(struct pt *pt))
{
  static uint32_t timer1;
  __attribute__((unused)) uint8_t s;


  PT_BEGIN(pt);

  PT_DELAY_MS(pt, &timer1, 250);
  
  screen_clear();
  LCD_WriteString(lcd, 0, 0, "TX mode", &Font_8x13, COLOR_RED, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_12); //GDO
  NVIC_EnableIRQ(EXTI15_10_IRQn); //GDO

  CC1101_reinit();

  setTime(&timer1);
  
  GDO0_flag = 0;

  while (1)
  {       
    PT_WAIT_UNTIL(pt, timer(&timer1, 350));

    static uint8_t count_tx = 0;

    if(count_tx >= 99)
    {
      count_tx = 0;
    }
    sprintf(packet, "TST %02d", count_tx++);

    s = CC1101_transmittRF(packet, sizeof(packet)); // the function is sending the data
    
    LCD_WriteString(lcd, 15, 40, packet, &Font_12x20, COLOR_RED, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

    PT_WAIT_UNTIL(pt, (GDO0_flag)); // TODO: уточнить работу GDO (low lowel - end transmitt)
    GDO0_flag = 0;

    PT_YIELD(pt);
  }

  PT_END(pt);
}