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
#include "displayInit.h"
#include "ili9341.h"

#include "encoderDriver.h"
#include "frequencyChannelsTable.h"

extern volatile uint8_t GDO0_FLAG;

extern RF_t CC1101;

#define FREQ_START 433.075
#define DIFFERENCE_WITH_CARRIER 0.985

static int8_t scanDat[128][1];
static uint8_t j;


static uint16_t interferenceLevel;

static float freqStep = 0.025;
static float startFreq = FREQ_START - DIFFERENCE_WITH_CARRIER; // LPD 1 start - BASE и CARRIER имеют сдвиг

static uint16_t cursor_x;

void scanRSSI(float freqSet)
{
  for (uint8_t i = 0; i < 128; i++)
  {
    CC1101_setMHZ(freqSet);
    
    for (uint8_t i = 0; i < 5; i++)
    {
      __ASM volatile ("NOP");
    }

    scanDat[i][j] = CC1101_RSSIconvert(CC1101_getRssiRaw());
    freqSet += freqStep;
  }
}

const uint16_t start_y = 150;
const uint16_t end_y = 50;
const uint16_t offset_x = 15;

static void drawCursor(uint16_t cursor_x)
{
  LCD_DrawLine(lcd, cursor_x, end_y, cursor_x, start_y, COLOR_RED);
}

static void cursorProcess(void)
{
  cursor_x = encoder_getRotaryNum();

  if (cursor_x < offset_x) cursor_x = offset_x;
  if (cursor_x > offset_x + 128) cursor_x = offset_x + 127;

  drawCursor(cursor_x);
}

void spectumDraw(void)
{
  const int16_t min_RSSI = 138;

  uint32_t summLevel = 0;

  for (uint8_t i = 0; i < 128; i++) // clear
  {
    LCD_DrawLine(lcd, offset_x + i, end_y, offset_x + i, start_y, COLOR_BLACK);
  }

  cursorProcess();

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

  screen_clear();
  LCD_WriteString(lcd, 0, 0, "SCAN mode", &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  char str[25] = {0};
  sprintf(str, "%.3f-%.3f", startFreq + DIFFERENCE_WITH_CARRIER, startFreq + DIFFERENCE_WITH_CARRIER + freqStep * 128);
  LCD_WriteString(lcd, 15, 25, str, &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);


  LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_12); //GDO
  NVIC_EnableIRQ(EXTI15_10_IRQn); //GDO
  GDO0_FLAG = 0;

  CC1101_reinit();

  TI_strobe(CCxxx0_SFRX); // Flush the buffer
  TI_strobe(CCxxx0_SRX);  // Set RX Mod
  
  encoder_setRotaryNum(offset_x + 68);

  while (1)
  {

    PT_WAIT_UNTIL(pt, timer(&timer1, 275));
    
    scanRSSI(startFreq);

    spectumDraw();

    float freqCursor = FREQ_START + (cursor_x - offset_x) * freqStep;

    uint8_t LPD_channel;
    for(LPD_channel = 0; LPD_channel < (sizeof(freqLpdList) / sizeof(float)); LPD_channel++)
    {
      if(freqLpdList[LPD_channel] == freqCursor)
      {
        LPD_channel += 1;
        break;
      }
    }
    
    static float freqCursorTmp;
    if (freqCursor != freqCursorTmp)
    {
      freqCursorTmp = freqCursor;
      sprintf(str, "%.3f", freqCursor);
      LCD_WriteString(lcd, 55, 165, str, &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

      sprintf(str, "LPD %02d", LPD_channel);
      LCD_WriteString(lcd, 55, 185, str, &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
    }


    sprintf(str, "Noise: %ld dBm", CC1101.RSSI_main);
    LCD_WriteString(lcd, 15, 240, str, &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);


    TI_strobe(CCxxx0_SFRX); // Flush the buffer
    TI_strobe(CCxxx0_SRX);  // Set RX Mod


    PT_YIELD(pt);
  }

  PT_END(pt);
}