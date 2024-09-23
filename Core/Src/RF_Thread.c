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

  uint8_t rssi_dec = (uint8_t) raw_rssi;

  if (rssi_dec >= 128)
  {
    return ((int)( rssi_dec - 256) / 2) - rssi_offset;
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
 
/*
 * Протопоток RX_Thread
 *
 */
PT_THREAD(RF_Thread(struct pt *pt))
{
    PT_BEGIN(pt);

    char buffer[64];
    char massage[7];
    static const char control_str[7] = {'T', 'S', 'T', ' ', 'X', 'X', '0'};
    uint8_t length;
    static uint8_t counter_RX, counter_Error = 0;

    TI_write_reg(CCxxx0_IOCFG0, 0x06);

    //__HAL_GPIO_EXTI_CLEAR_IT(CC_GDO_Pin);
    //HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    // printRSSImessage_OLED(0);
    // printRXcountOLED(counter_RX);

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

            //printInputStringOLED(massage);
            if (++counter_RX > 99)
            {
                counter_RX = 0;
            }

            if (errorData == 0)
            {
                //userLedToggle(); // Data is correct
            }
        }
        else
        {
            status = TI_read_status(CCxxx0_PKTSTATUS); // if it isnt, check pktstatus // değilse, paket durumunu control_str eder
            GDO0_FLAG = 0;
            debugPrintf(CLI_ERROR "CRC" CLI_NEW_LINE);
        }

            CC1101.countMessage = counter_RX;
            CC1101.RSSI = RSSIconvert(get_RSSI());
            CC1101.countError = counter_Error;

        PT_YIELD(pt);
    }

    PT_END(pt);
}

/*
uint8_t transmittRF(char *packet, uint8_t len)
{
    uint8_t status = TI_read_status(CCxxx0_VERSION); // it is for checking only //it must be 0x14
    status = TI_read_status(CCxxx0_TXBYTES);         // it is too
    TI_strobe(CCxxx0_SFTX);                          // flush the buffer
    // userLEDShow();
    TI_send_packet((uint8_t *)packet, len); // the function is sending the data

    while (HAL_GPIO_ReadPin(CC_GDO_GPIO_Port, CC_GDO_Pin))
        ;
    while (!HAL_GPIO_ReadPin(CC_GDO_GPIO_Port, CC_GDO_Pin))
        ;

    // if the pass to this function, the data was sent.

    status = TI_read_status(CCxxx0_TXBYTES); // it is checking to send the data
                                             // userLEDHide();

    return status;
}

*/

/*
 * Протопоток RF_Thread
 *
 *
 */

/*
PT_THREAD(RF_Thread(struct pt *pt))
{
    static uint32_t timer1;

    PT_BEGIN(pt);

    PT_DELAY_MS(pt, &timer1, 100);

    setTime(&timer1);

    while (1)
    {
        PT_WAIT_UNTIL(pt, timer(&timer1, 750)); // Запускаем преобразования ~ раз в 4s50 мс

        char packet[7]; // Резерв одного символа под нуль-терминатор!!
        sprintf(packet, "TST %02d", 4);
        transmittRF(packet, sizeof(packet)); // the function is sending the data

        PT_YIELD(pt);
    }

    PT_END(pt);
}
*/