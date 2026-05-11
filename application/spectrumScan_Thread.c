#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#include "main.h"
#include "spectrumScan_Thread.h"
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

extern RF_t CC1101;
extern LCD_Handler *lcd;

static int8_t scanDat[128][1];
static uint8_t j;
static uint16_t interferenceLevel;
static float freqStep = 0.025;
static float startFreq = LPD1 - DIFFERENCE_WITH_CARRIER;
static uint16_t cursor_x;

// Automatic mode variables 
static bool autoModeEnabled = false;
static bool isJamming = false;
static uint32_t jamStartTime = 0;
static float targetFreq = 0;
static uint8_t targetChannel = 0;
static int16_t targetRSSI = 0;

// RSSI averaging 
#define AVG_SCANS_COUNT 5
static int32_t rssiSum[128];
static uint8_t avgCounter = 0;
static int16_t avgRSSI[128];

// Settings 
#define AUTO_JAM_DURATION_MS 3000
#define RSSI_THRESHOLD -75

// Variables for packet transmission (jamming) 
static char txPacket[7] = "JAM";
static uint8_t txPacketIndex = 3;

// Message area coordinates for "Press encoder for auto jammer"
#define MSG_AREA_X 0
#define MSG_AREA_Y 260
#define MSG_AREA_W 200
#define MSG_AREA_H 16

// Random character generator 
static char generateRandomChar(void)
{
  static char randomChars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  static uint8_t idx = 0;
  idx = (idx + 1) % (sizeof(randomChars) - 1);
  return randomChars[rand() % (sizeof(randomChars) - 1)];
}

// Original scanning functions 
static void scanRSSI(float freqSet)
{
  for (uint8_t i = 0; i < 128; i++)
  {
    CC1101_setMHZ(freqSet);
    for (uint16_t d = 0; d < 100; d++)
    {
      __ASM volatile("NOP");
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
  if (cursor_x < offset_x)
  {
    cursor_x = offset_x;
    encoder_setRotaryNum(cursor_x);
  }
  if (cursor_x > offset_x + 128)
  {
    cursor_x = offset_x + 127;
    encoder_setRotaryNum(cursor_x);
  }
  drawCursor(cursor_x);
}

// Draw live spectrum from scanDat (manual mode) 
static void drawLiveSpectrum(void)
{
  const int16_t min_RSSI = 138;
  uint32_t summLevel = 0;
  for (uint8_t i = 0; i < 128; i++)
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
    uint32_t color = (y2 > interferenceLevel - 10) ? COLOR_BLUE : COLOR_PURPLE;
    LCD_DrawLine(lcd, offset_x + i, y2, offset_x + i, start_y, color);
  }
  interferenceLevel = summLevel / 128;
  CC1101.RSSI_main = ((int32_t)start_y - interferenceLevel) - min_RSSI;
}

// Draw averaged spectrum from avgRSSI (auto mode, including jamming) 
static void drawAvgSpectrum(void)
{
  const int16_t min_RSSI = 138;
  uint32_t summLevel = 0;
  for (uint8_t i = 0; i < 128; i++)
  {
    LCD_DrawLine(lcd, offset_x + i, end_y, offset_x + i, start_y, COLOR_BLACK);
  }
  cursorProcess();
  for (uint8_t i = 0; i < 128; i++)
  {
    uint16_t y2 = start_y - (min_RSSI + avgRSSI[i]);
    if (y2 < end_y)
    {
      y2 = end_y;
    }
    summLevel += y2;
    uint32_t color = (y2 > interferenceLevel - 10) ? COLOR_BLUE : COLOR_PURPLE;
    LCD_DrawLine(lcd, offset_x + i, y2, offset_x + i, start_y, color);
  }
  interferenceLevel = summLevel / 128;
  CC1101.RSSI_main = ((int32_t)start_y - interferenceLevel) - min_RSSI;
}

// Find maximum from averaged array 
static void findMaxFromAvg(float *freq, uint8_t *channel, int16_t *rssi)
{
  int16_t maxVal = -120;
  float bestFreq = 0;
  uint8_t bestChan = 0;
  for (uint8_t i = 0; i < 128; i++)
  {
    if (avgRSSI[i] > maxVal)
    {
      maxVal = avgRSSI[i];
      bestFreq = LPD1 + i * freqStep;
      bestChan = 0;
      for (uint8_t ch = 0; ch < (sizeof(freqLpdList) / sizeof(float)); ch++)
      {
        if (fabs(freqLpdList[ch] - bestFreq) < 0.0125)
        {
          bestChan = ch + 1;
          break;
        }
      }
    }
  }

  *freq = bestFreq;
  *channel = bestChan;
  *rssi = maxVal;
}

static void LCD_ClearRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color)
{
  for (uint16_t i = 0; i < w; i++)
  {
    LCD_DrawLine(lcd, x + i, y, x + i, y + h, color);
  }
}

// Main protothread 
PT_THREAD(spectrumScan_Thread(struct pt *pt))
{
  static uint32_t scanDelayTimer;
  static bool lastEncoderPress = false;
  static uint32_t jamPacketTimer;
  static uint32_t lastDisplayUpdate;

  PT_BEGIN(pt);

  PT_DELAY_MS(pt, &scanDelayTimer, 250);

  screen_clear();
  LCD_WriteString(lcd, 0, 0, "SCAN mode", &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
  char str[30];
  sprintf(str, "%.3f-%.3f", startFreq + DIFFERENCE_WITH_CARRIER,
          startFreq + DIFFERENCE_WITH_CARRIER + freqStep * 128);
  LCD_WriteString(lcd, 15, 25, str, &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  CC1101_reinit();
  CC1101_enter_rx_mode();
  encoder_setRotaryNum(offset_x + 68);

  memset(rssiSum, 0, sizeof(rssiSum));
  avgCounter = 0;

  while (1)
  {
    if (!isJamming)
    {
      // WAIT/SCAN MODE
      PT_WAIT_UNTIL(pt, timer(&scanDelayTimer, 100));

      scanRSSI(startFreq);

      float freqCursor = LPD1 + (cursor_x - offset_x) * freqStep;
      uint8_t cursorLpdChannel = 0;
      for (uint8_t ch = 0; ch < (sizeof(freqLpdList) / sizeof(float)); ch++)
      {
        if (fabs(freqLpdList[ch] - freqCursor) < 0.0125)
        {
          cursorLpdChannel = ch + 1;
          break;
        }
      }

      static float lastFreqCursor = 0;
      if (freqCursor != lastFreqCursor)
      {
        lastFreqCursor = freqCursor;
        sprintf(str, "%.3f", freqCursor);
        LCD_WriteString(lcd, 55, 165, str, &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
        sprintf(str, "LPD %02d", cursorLpdChannel);
        LCD_WriteString(lcd, 55, 185, str, &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
      }

      bool currentEncoderPress = encoder_getStateSwitch();
      if (currentEncoderPress && !lastEncoderPress)
      {
        autoModeEnabled = !autoModeEnabled;
        if (autoModeEnabled)
        {
          memset(rssiSum, 0, sizeof(rssiSum));
          avgCounter = 0;
          LCD_WriteString(lcd, 10, 200, "AUTO ON", &Font_8x13, COLOR_GREEN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
        }
        else
        {
          LCD_ClearRect(0, 200, 100, 20, COLOR_BLACK);
          if (isJamming)
          {
            isJamming = false;
            CC1101_enter_rx_mode();
          }
        }
      }
      lastEncoderPress = currentEncoderPress;

      if (!autoModeEnabled)
      {
        drawLiveSpectrum();
      }
      else
      {
        for (uint8_t i = 0; i < 128; i++)
        {
          rssiSum[i] += scanDat[i][j];
        }
        avgCounter++;

        char prog[16];
        sprintf(prog, "Avg: %d/5", avgCounter);
        LCD_WriteString(lcd, 0, 300, prog, &Font_8x13, COLOR_GREEN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

        if (avgCounter >= AVG_SCANS_COUNT)
        {
          for (uint8_t i = 0; i < 128; i++)
          {
            avgRSSI[i] = rssiSum[i] / AVG_SCANS_COUNT;
          }
          drawAvgSpectrum();

          float bestFreq = 0;
          uint8_t bestChan = 0;
          int16_t bestRSSI = 0;
          findMaxFromAvg(&bestFreq, &bestChan, &bestRSSI);

          if (bestChan > 0 && bestRSSI > RSSI_THRESHOLD && !isJamming)
          {
            isJamming = true;
            targetFreq = bestFreq;
            targetChannel = bestChan;
            targetRSSI = bestRSSI;
            jamStartTime = HAL_GetTick();

            CC1101_setMHZ(targetFreq - DIFFERENCE_WITH_CARRIER);
            DEBUG_PRINT("[AUTO] Jamming %s LPD%02d %.3f (RSSI=%d)"CLI_NEW_LINE, "on", targetChannel, targetFreq, targetRSSI);
          }

          memset(rssiSum, 0, sizeof(rssiSum));
          avgCounter = 0;
        }
        else
        {
          if (avgRSSI[0] != 0)
          {
            drawAvgSpectrum();
          }
        }
      }

      // Display hint for manual mode: press encoder to enable auto jammer (two lines)
      if (!autoModeEnabled && !isJamming)
      {
        LCD_WriteString(lcd, 0, 260, "Press encoder", &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
        LCD_WriteString(lcd, 0, 275, "for auto jammer", &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
      }
      else
      {
        LCD_ClearRect(0, 260, 150, 28, COLOR_BLACK);
      }

      sprintf(str, "Noise: %ld dBm", CC1101.RSSI_main);
      LCD_WriteString(lcd, 10, 240, str, &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

      CC1101_enter_rx_mode();
    }
    else
    {
      // JAMMING MODE
      uint32_t currentTime = HAL_GetTick();
      if (currentTime - jamStartTime >= AUTO_JAM_DURATION_MS)
      {
        isJamming = false;
        CC1101_enter_rx_mode();
        LCD_ClearRect(0, 200, 180, 60, COLOR_BLACK);
        memset(rssiSum, 0, sizeof(rssiSum));
        avgCounter = 0;
        DEBUG_PRINT("[AUTO] Jam finished"CLI_NEW_LINE);
      }
      else
      {
        char info[30] = {0};
        uint32_t remaining = (AUTO_JAM_DURATION_MS - (currentTime - jamStartTime)) / 1000;
        sprintf(info, "JAM: %lu s ", remaining);
        LCD_WriteString(lcd, 10, 200, info, &Font_8x13, COLOR_RED, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
        sprintf(info, "Target:LPD%02d %.3f", targetChannel, targetFreq);
        LCD_WriteString(lcd, 10, 220, info, &Font_8x13, COLOR_YELLOW, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

        if (currentTime - lastDisplayUpdate > 200)
        {
          drawAvgSpectrum();
          lastDisplayUpdate = currentTime;
        }

        txPacket[txPacketIndex++] = generateRandomChar();
        if (txPacketIndex >= 6)
        {
          txPacketIndex = 3;
        }

        uint8_t result = CC1101_transmitt_packet(txPacket, sizeof(txPacket));
        if (result)
        {
          DEBUG_PRINT("JAM ERR: %d"CLI_NEW_LINE, result);
        }
        LCD_WriteString(lcd, 15, 65, txPacket, &Font_12x20, COLOR_RED, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

        static uint32_t tx_timeout;
        tx_timeout = HAL_GetTick() + 100;

        PT_WAIT_UNTIL(pt, (CC1101_GDO0_flag_get() || (HAL_GetTick() > tx_timeout)));
        if (!CC1101_GDO0_flag_get())
        {
          DEBUG_PRINT("TX TIMEOUT"CLI_NEW_LINE);
        }
        CC1101_GDO0_flag_clear();

        PT_WAIT_UNTIL(pt, timer(&jamPacketTimer, 10));
      }
    }

    PT_YIELD(pt);
  }

  PT_END(pt);
}