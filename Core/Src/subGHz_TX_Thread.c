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

static char packet[7] = "QWERTY";; // Резерв одного символа под нуль-терминатор!!

static uint8_t transmittRF(const char *packet_loc, uint8_t len)
{
    uint8_t status = 0;
    
    status = TI_read_status(CCxxx0_VERSION);         // it is for checking only (it must be 0x14)
    status = TI_read_status(CCxxx0_TXBYTES);         // it is too
    TI_strobe(CCxxx0_SFTX);                          // flush the buffer

    __ASM volatile ("NOP");

    TI_send_packet((uint8_t *)packet_loc, len); // the function is sending the data
    //DEBUG_PRINT(CLI_TX"%s %d"CLI_NEW_LINE, packet, len);

    while (HAL_GPIO_ReadPin(CC_GDO_GPIO_Port, CC_GDO_Pin))
    {
      __ASM volatile ("NOP");
    }
    while (!HAL_GPIO_ReadPin(CC_GDO_GPIO_Port, CC_GDO_Pin))
    {
      __ASM volatile ("NOP");
    }

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
    __attribute__((unused)) uint8_t s;


    PT_BEGIN(pt);

    PT_DELAY_MS(pt, &timer1, 250);

    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_12); //GDO
    NVIC_DisableIRQ(EXTI15_10_IRQn); //GDO
    GDO0_FLAG = 0;

    CC1101_reinit();

    setTime(&timer1);

    while (1)
    {
        static uint8_t count_tx = 0;
        if(count_tx >= 99)
        {
          count_tx = 0;
        }

        sprintf(packet, "TST %02d", count_tx++);

        s = transmittRF(packet, sizeof(packet)); // the function is sending the data
        
        PT_WAIT_UNTIL(pt, timer(&timer1, 750));

        PT_YIELD(pt);
    }

    PT_END(pt);
}