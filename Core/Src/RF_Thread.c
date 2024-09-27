#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#include "main.h"
#include "RF_Thread.h"
#include "cli_driver.h"
#include "cc1101.h"
#include "time.h"

extern volatile uint8_t GDO0_FLAG;

RF_t CC1101 = {0};

static int RSSIconvert(char raw_rssi)
{
  const uint8_t rssi_offset = 74;

  uint8_t rssi_dec = (uint8_t)raw_rssi;

  if (rssi_dec >= 128)
  {
    return ((int)(rssi_dec - 256) / 2) - rssi_offset;
  }
  else
  {
    return (rssi_dec / 2) - rssi_offset;
  }
}

static uint16_t autoCalibrate(void)
{
  static uint16_t accumulatedOffset = 0;

  uint16_t offset = TI_read_status(CCxxx0_FREQEST);
  if (offset != 0)
  {
    accumulatedOffset += offset;
    TI_write_reg(CCxxx0_FSCTRL0, accumulatedOffset);
  }

  return accumulatedOffset;
}

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
 * Протопоток RX_Thread
 *
 */
PT_THREAD(RF_Thread(struct pt *pt))
{

#define SPECTRUM_EN true
#if SPECTRUM_EN
  static uint32_t timeCount;
  if ((HAL_GetTick() - timeCount) > 300u)
  {
    timeCount = HAL_GetTick();

    uint8_t rssi_raw = TI_read_status(CCxxx0_RSSI);
    CC1101.RSSI_main = RSSIconvert(rssi_raw);

    scanRSSI(startFreq);
    // debugPrintf("%f "CLI_NEW_LINE);
    // for(uint8_t i = 0; i < 128; i++)
    //{
    //  debugPrintf("%d ",scanDat[i]);
    // }
    // debugPrintf("%f "CLI_NEW_LINE, startFreq + 128*freqStep);
    CC1101_setMHZ(422.999817);
    spectumDraw();
  }
#endif

  PT_BEGIN(pt);

  static char buffer[64];
  char massage[7] = "";
  static const char control_str[7] = {'T', 'S', 'T', ' ', 'X', 'X', '0'};
  uint8_t length;
  static uint8_t counter_RX, counter_Error = 0;

  TI_write_reg(CCxxx0_IOCFG0, 0x06);

  //__HAL_GPIO_EXTI_CLEAR_IT(CC_GDO_Pin);
  // HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  static uint32_t timer1;
  PT_DELAY_MS(pt, &timer1, 1900);
  char str[25] = {0};
  sprintf(str, "%.3f-%.3f", startFreq, startFreq + freqStep * 128);
  LCD_WriteString(lcd, 90, 195, str, &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  while (1)
  {
    TI_strobe(CCxxx0_SFRX); // Flush the buffer //Bufferi temizle
    TI_strobe(CCxxx0_SRX);  // Set RX Mode //RX moda ayarlar

    PT_WAIT_UNTIL(pt, GDO0_FLAG != 0); // 0 - highLevel
    GDO0_FLAG = 0;
    uint8_t errorData = 0;

    uint8_t status = TI_read_status(CCxxx0_RXBYTES);

    if (!(status & 0x7f))
      continue;

    uint8_t LQI = TI_read_status(CCxxx0_LQI);

    if (LQI & 0x80 /*CRC_OK*/)
    {
      status = TI_receive_packet((uint8_t *)buffer, &length);

      if (status == false)
      {
        debugPrintf(CLI_ERROR "status: %d, len: %d" CLI_NEW_LINE, status, length);
      }

      for (uint8_t i = 0; i < 7; i++) // copy
      {
        massage[i] = buffer[i];
        CC1101.dataString[i] = buffer[i];
      }

      for (uint8_t i = 0; i < 4; i++) // to check for receiving data
      {
        if (massage[i] != control_str[i])
        {
          errorData = 1;
          counter_Error++;
          break;
        }
      }

      uint16_t offset = autoCalibrate();

      debugPrintf("%s, RSSI: %d, offset: %d" CLI_NEW_LINE, massage, RSSIconvert(get_RSSI()), offset);

      // printInputStringOLED(massage);
      if (++counter_RX > 99)
      {
        counter_RX = 0;
      }

      if (errorData == 0)
      {
        // userLedToggle(); // Data is correct
      }
    }
    else
    {
      status = TI_read_status(CCxxx0_PKTSTATUS); // if it isnt, check pktstatus // değilse, paket durumunu control_str eder
      GDO0_FLAG = 0;
      debugPrintf(CLI_ERROR "CRC" CLI_NEW_LINE);
      counter_Error++;
    }

    CC1101.countMessage = counter_RX;
    CC1101.RSSI = RSSIconvert(get_RSSI());
    CC1101.countError = counter_Error;

    PT_YIELD(pt);
  }

  PT_END(pt);
}