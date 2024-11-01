#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#include "main.h"
#include "jammer.h"
#include "spectrumScan.h"
#include "subGHz_RX_Thread.h"
#include "subGHz_TX_Thread.h"
#include "application_Thread.h"
#include "cli_driver.h"
#include "cc1101.h"
#include "time.h"
#include "adc.h"

#include "display.h"
#include "ili9341.h"

extern volatile uint8_t GDO0_FLAG;

extern LCD_Handler *lcd;
extern RF_t CC1101;

char generateRandom(void)
{
  return (char)(getAdcVDDA() & 127);
}

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

    clearWindow();
    LCD_WriteString(lcd, 0, 0, "Jammer mode", &Font_8x13, COLOR_RED, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_12); //GDO
    NVIC_DisableIRQ(EXTI15_10_IRQn); //GDO
    GDO0_FLAG = 0;

    CC1101_reinit();

    setTime(&timer1);

    CC1101_setMHZ(432.490);


    while (1)
    {
      PT_WAIT_UNTIL(pt, (HAL_GPIO_ReadPin(CC_GDO_GPIO_Port, CC_GDO_Pin) == GPIO_PIN_RESET)); // TODO: уточнить работу GDO


      static char packet[7] = "om5q3z"; // Резерв одного символа под нуль-терминатор!!

      static uint8_t i;
      packet[i++] = generateRandom();

      if(i > 5) i = 0;

      //debugPrintf("%s %d"CLI_NEW_LINE, packet, packet[i]);

      s = transmittRF(packet, sizeof(packet)); // the function is sending the data
      LCD_WriteString(lcd, 15, 40, packet, &Font_12x20, COLOR_RED, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
      

      PT_YIELD(pt);
    }

    PT_END(pt);
}