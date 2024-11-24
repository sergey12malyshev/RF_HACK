#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#include "main.h"
#include "subGHz_RX_Thread.h"
#include "application_Thread.h"
#include "cli_driver.h"
#include "cc1101.h"
#include "time.h"

#include "display.h"
#include "displayInit.h"
#include "ili9341.h"
#include "xpt2046.h"
#include "calibrate_touch.h"
#include "demo.h"


#define MAX_PACKET_LENGTH   25U  // указываем максимальный размер принимаемой строки

extern volatile uint8_t GDO0_FLAG;

RF_t CC1101 = {0};

static void CC1101_DataScreen(void)
{
  uint16_t start_x = 10;
  uint16_t start_y = 25;

  uint16_t offset_y = 16;

  char str[100] = "";
  sprintf(str, "Count mesage: %d", CC1101.countMessage);
  LCD_WriteString(lcd, start_x, offset_y + start_y, str, &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  sprintf(str, "Count error: %d", CC1101.countError);
  LCD_WriteString(lcd, start_x, offset_y*2 + start_y, str, &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  sprintf(str, "RCCI: %d dBm", CC1101.RSSI);
  LCD_WriteString(lcd, start_x, offset_y*3 + start_y, str, &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  sprintf(str, "Message: %s", CC1101.dataString);
  LCD_WriteString(lcd, start_x, offset_y*5 + start_y, str, &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
}

/*
 * Протопоток subGHz_RX_Thread
 *
 */
PT_THREAD(subGHz_RX_Thread(struct pt *pt))
{
  static uint32_t timer1;

  static char buffer[64];
  char massage[7] = "";
  static const char control_str[7] = {'T', 'S', 'T', ' ', 'X', 'X', '0'};
  static uint8_t counter_RX, counter_Error = 0;

  PT_BEGIN(pt);

  PT_DELAY_MS(pt, &timer1, 250);

  screen_clear();
  LCD_WriteString(lcd, 0, 0, "RX Mode", &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
  LCD_WriteString(lcd, 0, 20, "CC1101 Data:", &Font_8x13, COLOR_CYAN, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
  CC1101_DataScreen();

  LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_12); //GDO
  NVIC_EnableIRQ(EXTI15_10_IRQn); //GDO
  GDO0_FLAG = 0;

  CC1101_reinit();

  while (1)
  {
    TI_strobe(CCxxx0_SFRX); // Flush the buffer
    TI_strobe(CCxxx0_SRX);  // Set RX Mode

    PT_WAIT_UNTIL(pt, GDO0_FLAG); // 0 - highLevel

    GDO0_FLAG = 0;
    uint8_t errorData = 0;

    uint8_t status = TI_read_status(CCxxx0_RXBYTES);

    if (!(status & 0x7f))
      continue;

    uint8_t LQI = CC1101_getLqi();

    if (LQI & 0x80 /*CRC_OK*/)
    {
      uint8_t length_packet = MAX_PACKET_LENGTH;

      status = TI_receive_packet((uint8_t *)buffer, &length_packet);

      if ((status == RX_ERR_LENGHT)||(status == RX_ERR_RX))
      {
        counter_Error++;
        debugPrintf(CLI_ERROR "status: %d, len: %d" CLI_NEW_LINE, status, length_packet);
      }
      else
      {
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
            break;
          }
        }

        uint16_t offset = CC1101_autoCalibrate1();

        debugPrintf("%s, RSSI: %d, offset: %d" CLI_NEW_LINE, massage, CC1101_RSSIconvert(get_RSSI()), offset);
      }
    }
    else
    {
      status = TI_read_status(CCxxx0_PKTSTATUS);
      GDO0_FLAG = 0;
      debugPrintf(CLI_ERROR "CRC" CLI_NEW_LINE);
      counter_Error++;
    }


    if (++counter_RX > 99)
    {
      counter_RX = 0;
    }
    CC1101.countMessage = counter_RX;
    CC1101.RSSI = CC1101_RSSIconvert(get_RSSI());
    CC1101.countError = counter_Error;

    CC1101_DataScreen();

    PT_YIELD(pt);
  }

  PT_END(pt);
}