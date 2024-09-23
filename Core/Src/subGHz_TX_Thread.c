#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#include "main.h"
#include "subGHz_TX_Thread.h"
#include "cli_driver.h"
#include "cc1101.h"
#include "time.h"

extern volatile uint8_t GDO0_FLAG;

static uint8_t transmittRF(char *packet, uint8_t len)
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

/*
 * Протопоток subGHz_TX_Thread
 *
 *
 */
PT_THREAD(subGHz_TX_Thread(struct pt *pt))
{
    static uint32_t timer1;

    PT_BEGIN(pt);

    PT_DELAY_MS(pt, &timer1, 100);

    setTime(&timer1);

    while (1)
    {
        PT_WAIT_UNTIL(pt, timer(&timer1, 450));

        char packet[7]; // Резерв одного символа под нуль-терминатор!!
        sprintf(packet, "TST %02d", 4);
        transmittRF(packet, sizeof(packet)); // the function is sending the data

        PT_YIELD(pt);
    }

    PT_END(pt);
}