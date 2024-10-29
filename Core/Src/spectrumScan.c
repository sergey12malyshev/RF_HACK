#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#include "main.h"
#include "spectrumScan.h"
#include "subGHz_RX_Thread.h"
#include "application_Thread.h"
#include "cli_driver.h"
#include "cc1101.h"
#include "time.h"

#include "display.h"
#include "ili9341.h"

extern volatile uint8_t GDO0_FLAG;

extern LCD_Handler *lcd;
extern RF_t CC1101;


#define DIFFERENCE_WITH_CARRIER 0.985

static int8_t scanDat[128][1];
static uint8_t j;


static uint16_t interferenceLevel;

static float freqStep = 0.025;
static float startFreq = 433.075 - DIFFERENCE_WITH_CARRIER; // LPD 1 start - BASE и CARRIER имеют сдвиг

/*
 Функция выдернута из arduino-библиотеки

 *FUNCTION NAME:Frequency Calculator
 *FUNCTION     :Calculate the basic frequency.
 *INPUT        :Frequency
*/
void CC1101_setMHZ(float mhz)
{
  uint8_t freq2 = 0;
  uint8_t freq1 = 0;
  uint8_t freq0 = 0;

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

void scanRSSI(float freqSet)
{
  for (uint8_t i = 0; i < 128; i++)
  {
    CC1101_setMHZ(freqSet);
    uint8_t rssi_raw = TI_read_status(CCxxx0_RSSI);
    scanDat[i][j] = RSSIconvert(rssi_raw);
    freqSet += freqStep;
  }
}

void spectumDraw(void)
{
  const uint16_t start_y = 150;
  const uint16_t end_y = 50;
  const uint16_t offset_x = 15;

  const int16_t min_RSSI = 138;


  uint32_t summLevel = 0;

  for (uint8_t i = 0; i < 128; i++) // clear
  {
    LCD_DrawLine(lcd, offset_x + i, end_y, offset_x + i, start_y, COLOR_BLACK);
  }

  for (uint8_t i = 0; i < 128; i++)
  {

    uint16_t y2 = start_y - (min_RSSI + scanDat[i][j]);
    if (y2 < end_y)
    {
      y2 = end_y;
    }

    summLevel += y2;

    if (y2 > interferenceLevel - 10)
    {
      LCD_DrawLine(lcd, offset_x + i, y2, offset_x + i, start_y, COLOR_BLUE);
    }
    else
    {
      LCD_DrawLine(lcd, offset_x + i, y2, offset_x + i, start_y, COLOR_PURPLE);
    }
  }

  interferenceLevel = summLevel / 128;
  CC1101.RSSI_main = ((int32_t) start_y - interferenceLevel) - min_RSSI;
}

__UNUSED static void waterfallDraw(void)
{
  const uint16_t start_y = 155;
  const uint16_t offset_x = 15;

  uint32_t color = COLOR_BLUE;

  
  for(uint8_t j = 0; j < 10; j++) // clear
  {
    for (uint8_t i = 0; i < 128; i++)
    {
      
      const int16_t min_RSSI = 138;

      uint16_t y2 = 150 - (min_RSSI + scanDat[i][j]);
      if (y2 < 50)
      {
        y2 = 50;
      }

      if(y2 < interferenceLevel - 10)
      {
        color = COLOR_PURPLE;
      }
      LCD_DrawPixel(lcd, offset_x + i, start_y + j, color);
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

  PT_DELAY_MS(pt, &timer1, 250);

  clearWindow();
  LCD_WriteString(lcd, 0, 0, "SCAN mode", &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  char str[25] = {0};
  sprintf(str, "%.3f-%.3f", startFreq + DIFFERENCE_WITH_CARRIER, startFreq + DIFFERENCE_WITH_CARRIER + freqStep * 128);
  LCD_WriteString(lcd, 15, 25, str, &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_12); //GDO
  NVIC_EnableIRQ(EXTI15_10_IRQn); //GDO
  GDO0_FLAG = 0;

  CC1101_reinit();

  TI_write_reg(CCxxx0_IOCFG0, 0x06); // GDO0 Output Pin Configuration
  TI_strobe(CCxxx0_SFRX); // Flush the buffer
  TI_strobe(CCxxx0_SRX);  // Set RX Mode



  while (1)
  {

    PT_WAIT_UNTIL(pt, timer(&timer1, 250));

    //uint8_t rssi_raw = TI_read_status(CCxxx0_RSSI);
    //CC1101.RSSI_main = RSSIconvert(rssi_raw);

    scanRSSI(startFreq);
#if 0
    debugPrintf("%f "CLI_NEW_LINE);
    for(uint8_t i = 0; i < 128; i++)
    {
      debugPrintf("%d ",scanDat[i][j]);
    }
    debugPrintf("%f "CLI_NEW_LINE, startFreq + 128*freqStep);
#endif
    spectumDraw();
    //waterfallDraw();

    sprintf(str, "Noise: %ld dBm", CC1101.RSSI_main);
    LCD_WriteString(lcd, 15, 240, str, &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

    //debugPrintf("%d "CLI_NEW_LINE, interferenceLevel);


    PT_YIELD(pt);
  }

  PT_END(pt);
}