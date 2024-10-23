#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#include "main.h"
#include "spectrumScan.h"
#include "RF_Thread.h"
#include "cli_driver.h"
#include "cc1101.h"
#include "time.h"

#include "display.h"
#include "ili9341.h"

extern volatile uint8_t GDO0_FLAG;

extern LCD_Handler *lcd;
extern RF_t CC1101;

/****************************************************************
 *FUNCTION NAME:Frequency Calculator
 *FUNCTION     :Calculate the basic frequency.
 *INPUT        :none
 *OUTPUT       :none
 ****************************************************************/
void CC1101_setMHZ(float mhz)
{
  uint8_t freq2 = 0;
  uint8_t freq1 = 0;
  uint8_t freq0 = 0;

  // MHz = mhz;

  for (bool i = 0; i == 0;)
  {
    if (mhz >= 26)
    {
      mhz -= 26;
      freq2 += 1;
    }
    else if (mhz >= 0.1015625)
    {
      mhz -= 0.1015625;
      freq1 += 1;
    }
    else if (mhz >= 0.00039675)
    {
      mhz -= 0.00039675;
      freq0 += 1;
    }
    else
    {
      i = 1;
    }
  }
  if (freq0 > 255)
  {
    freq1 += 1;
    freq0 -= 256;
  }

  TI_write_reg(CCxxx0_FREQ2, freq2);
  TI_write_reg(CCxxx0_FREQ1, freq1);
  TI_write_reg(CCxxx0_FREQ0, freq0);
  // Calibrate();
}

int8_t scanDat[128];
float freqStep = 0.01;
float startFreq = 432.7;

void scanRSSI(float freqSet)
{
  for (uint8_t i = 0; i < 128; i++)
  {
    CC1101_setMHZ(freqSet);
    uint8_t rssi_raw = TI_read_status(CCxxx0_RSSI);
    scanDat[i] = RSSIconvert(rssi_raw);
    freqSet += freqStep;
  }
}

#include "display.h"
#include "ili9341.h"
#include "xpt2046.h"

extern LCD_Handler *lcd;
void spectumDraw(void)
{
  const uint16_t start_y = 310;
  const uint16_t offset_x = 90;

  for (uint8_t i = 0; i < 128; i++) // clear
  {
    LCD_DrawLine(lcd, offset_x + i, 210, offset_x + i, start_y, COLOR_BLACK);
  }

  for (uint8_t i = 0; i < 128; i++)
  {
    const int16_t min_RSSI = 138;
    uint16_t y2 = start_y - (min_RSSI + scanDat[i]);
    if (y2 < 210)
    {
      y2 = 210;
    }

    if (y2 > 260)
    {
      LCD_DrawLine(lcd, offset_x + i, y2, offset_x + i, start_y, COLOR_BLUE);
    }
    else
    {
      LCD_DrawLine(lcd, offset_x + i, y2, offset_x + i, start_y, COLOR_PURPLE);
    }
  }
}



/*
 * Протопоток spectrumScan_Thread
 *
 */
PT_THREAD(spectrumScan_Thread(struct pt *pt))
{
  static uint32_t timer1;

  PT_BEGIN(pt);

  PT_DELAY_MS(pt, &timer1, 200);

  LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_12); //GDO
  NVIC_DisableIRQ(EXTI15_10_IRQn); //GDO
  GDO0_FLAG = 0;

  char str[25] = {0};
  sprintf(str, "%.3f-%.3f", startFreq, startFreq + freqStep * 128);
  LCD_WriteString(lcd, 90, 195, str, &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  while (1)
  {

    PT_WAIT_UNTIL(pt, timer(&timer1, 250));

    uint8_t rssi_raw = TI_read_status(CCxxx0_RSSI);
    CC1101.RSSI_main = RSSIconvert(rssi_raw);

    scanRSSI(startFreq);
    // debugPrintf("%f "CLI_NEW_LINE);
    // for(uint8_t i = 0; i < 128; i++)
    //{
    //  debugPrintf("%d ",scanDat[i]);
    // }
    // debugPrintf("%f "CLI_NEW_LINE, startFreq + 128*freqStep);

    //CC1101_setMHZ(422.999817);
    spectumDraw();


    PT_YIELD(pt);
  }

  PT_END(pt);
}