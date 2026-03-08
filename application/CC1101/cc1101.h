/*
 * CC1101.H
 *
 *  Created on: Mar 11, 2020
 *      Author: suleyman.eskil
 * 
 *  Updated 2024 Malyshev Sergey
 *      https://github.com/sergey12malyshev
 */
#pragma once

#ifndef CC1101_H
#define CC1101_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "stm32f4xx_ll_gpio.h"

#ifndef __STM32F4xx_HAL_H
#include "stm32f4xx_hal.h"
#endif


#define DIFFERENCE_WITH_CARRIER       0.985  // BASE and CARRIER have a shift

/* CC2500/CC1100 STROBE, CONTROL AND STATUS REGSITER */
#define CCxxx0_IOCFG2       0x00        // GDO2 output pin configuration
#define CCxxx0_IOCFG1       0x01        // GDO1 output pin configuration
#define CCxxx0_IOCFG0       0x02        // GDO0 output pin configuration
#define CCxxx0_FIFOTHR      0x03        // RX FIFO and TX FIFO thresholds
#define CCxxx0_SYNC1        0x04        // Sync word, high byte
#define CCxxx0_SYNC0        0x05        // Sync word, low byte
#define CCxxx0_PKTLEN       0x06        // Packet length
#define CCxxx0_PKTCTRL1     0x07        // Packet automation control
#define CCxxx0_PKTCTRL0     0x08        // Packet automation control
#define CCxxx0_ADDR         0x09        // Device address
#define CCxxx0_CHANNR       0x0A        // Channel number
#define CCxxx0_FSCTRL1      0x0B        // Frequency synthesizer control
#define CCxxx0_FSCTRL0      0x0C        // Frequency synthesizer control
#define CCxxx0_FREQ2        0x0D        // Frequency control word, high byte
#define CCxxx0_FREQ1        0x0E        // Frequency control word, middle byte
#define CCxxx0_FREQ0        0x0F        // Frequency control word, low byte
#define CCxxx0_MDMCFG4      0x10        // Modem configuration
#define CCxxx0_MDMCFG3      0x11        // Modem configuration
#define CCxxx0_MDMCFG2      0x12        // Modem configuration
#define CCxxx0_MDMCFG1      0x13        // Modem configuration
#define CCxxx0_MDMCFG0      0x14        // Modem configuration
#define CCxxx0_DEVIATN      0x15        // Modem deviation setting
#define CCxxx0_MCSM2        0x16        // Main Radio Control State Machine configuration
#define CCxxx0_MCSM1        0x17        // Main Radio Control State Machine configuration
#define CCxxx0_MCSM0        0x18        // Main Radio Control State Machine configuration
#define CCxxx0_FOCCFG       0x19        // Frequency Offset Compensation configuration
#define CCxxx0_BSCFG        0x1A        // Bit Synchronization configuration
#define CCxxx0_AGCCTRL2     0x1B        // AGC control
#define CCxxx0_AGCCTRL1     0x1C        // AGC control
#define CCxxx0_AGCCTRL0     0x1D        // AGC control
#define CCxxx0_WOREVT1      0x1E        // High byte Event 0 timeout
#define CCxxx0_WOREVT0      0x1F        // Low byte Event 0 timeout
#define CCxxx0_WORCTRL      0x20        // Wake On Radio control
#define CCxxx0_FREND1       0x21        // Front end RX configuration
#define CCxxx0_FREND0       0x22        // Front end TX configuration
#define CCxxx0_FSCAL3       0x23        // Frequency synthesizer calibration
#define CCxxx0_FSCAL2       0x24        // Frequency synthesizer calibration
#define CCxxx0_FSCAL1       0x25        // Frequency synthesizer calibration
#define CCxxx0_FSCAL0       0x26        // Frequency synthesizer calibration
#define CCxxx0_RCCTRL1      0x27        // RC oscillator configuration
#define CCxxx0_RCCTRL0      0x28        // RC oscillator configuration
#define CCxxx0_FSTEST       0x29        // Frequency synthesizer calibration control
#define CCxxx0_PTEST        0x2A        // Production test
#define CCxxx0_AGCTEST      0x2B        // AGC test
#define CCxxx0_TEST2        0x2C        // Various test settings
#define CCxxx0_TEST1        0x2D        // Various test settings
#define CCxxx0_TEST0        0x2E        // Various test settings

// Strobe commands
#define CCxxx0_SRES             0x30    // Reset chip.
#define CCxxx0_SFSTXON          0x31    // Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1).
                                        // If in RX/TX: Go to a wait state where only the synthesizer is
                                        // running (for quick RX / TX turnaround).
#define CCxxx0_SXOFF            0x32    // Turn off crystal oscillator.
#define CCxxx0_SCAL             0x33    // Calibrate frequency synthesizer and turn it off
                                        // (enables quick start).
#define CCxxx0_SRX              0x34    // Enable RX. Perform calibration first if coming from IDLE and
                                        // MCSM0.FS_AUTOCAL=1.
#define CCxxx0_STX              0x35    // In IDLE state: Enable TX. Perform calibration first if
                                        // MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled:
                                        // Only go to TX if channel is clear.
#define CCxxx0_SIDLE            0x36    // Exit RX / TX, turn off frequency synthesizer and exit
                                        // Wake-On-Radio mode if applicable.
#define CCxxx0_SAFC             0x37    // Perform AFC adjustment of the frequency synthesizer
#define CCxxx0_SWOR             0x38    // Start automatic RX polling sequence (Wake-on-Radio)
#define CCxxx0_SPWD             0x39    // Enter power down mode when CSn goes high.
#define CCxxx0_SFRX             0x3A    // Flush the RX FIFO buffer.
#define CCxxx0_SFTX             0x3B    // Flush the TX FIFO buffer.
#define CCxxx0_SWORRST          0x3C    // Reset real time clock.
#define CCxxx0_SNOP             0x3D    // No operation. May be used to pad strobe commands to two
                                        // bytes for simpler software.
// Status registers
#define CCxxx0_PARTNUM          0x30    // Part number
#define CCxxx0_VERSION          0x31    // Current version number
#define CCxxx0_FREQEST          0x32    // Frequency offset estimate
#define CCxxx0_LQI              0x33    // Demodulator estimate for link quality
#define CCxxx0_RSSI             0x34    // Received signal strength indication
#define CCxxx0_MARCSTATE        0x35    // Control state machine state
#define CCxxx0_WORTIME1         0x36    // High byte of WOR timer
#define CCxxx0_WORTIME0         0x37    // Low byte of WOR timer
#define CCxxx0_PKTSTATUS        0x38    // Current GDOx status and packet status
#define CCxxx0_VCO_VC_DAC       0x39    // Current setting from PLL cal module
#define CCxxx0_TXBYTES          0x3A    // Underflow and # of bytes in TXFIFO
#define CCxxx0_RXBYTES          0x3B    // Overflow and # of bytes in RXFIFO
#define CCxxx0_RCCTRL1_STATUS   0x3C
#define CCxxx0_RCCTRL0_STATUS   0x3D
#define TI_CCxxx0_NUM_RXBYTES   0x7F    // Mask "# of bytes" field in _RXBYTES

#define CCxxx0_PATABLE          0x3E
#define CCxxx0_TXFIFO           0x3F
#define CCxxx0_RXFIFO           0x3F

// Masks for appended status bytes
#define TI_CCxxx0_LQI_RX       0x01     // Position of LQI byte
#define TI_CCxxx0_CRC_OK       0x80     // Mask "CRC_OK" bit within LQI byte

// Definitions to support burst/single access:
#define TI_CCxxx0_WRITE_BURST  0x40
#define TI_CCxxx0_READ_SINGLE  0x80
#define TI_CCxxx0_READ_BURST   0xC0

typedef enum
{
  RX_ERR_LENGHT,
  RX_ERR_RX
} ReceiveState_t;

/**
 * Carrier frequencies
 */
enum CFREQ
{
  CFREQ_868 = 0,
  CFREQ_915,
  CFREQ_433,
  CFREQ_918,
  CFREQ_LAST
};

typedef enum _Modulation
{
  _2_FSK = 0,
  _GFSK,
  _ASK,
  _4_FSK,
  _MSK
} Modulation_t;

typedef enum _CC1101_Status_t
{
  CC1101_OK = 0,
  CC1101_ERROR,
  CC1101_TIMEOUT,
  CC1101_ERROR_SPI,
  CC1101_ERROR_VERSION,
  CC1101_ERROR_OVERFLOW,
  CC1101_ERROR_NO_MESSAGE,
  CC1101_ERROR_CONFIG,
} CC1101_Status_t;


CC1101_Status_t CC1101_init(SPI_HandleTypeDef* hspi, 
                    GPIO_TypeDef* cs_port, uint16_t cs_pin,
                    GPIO_TypeDef* miso_port, uint16_t miso_pin,
                    GPIO_TypeDef* gdo_port, uint16_t gdo_pin);

CC1101_Status_t CC1101_reinit(void);
CC1101_Status_t CC1101_power_up_reset(void);
void CC1101_IRQHandler(void);

void CC1101_strobe(uint8_t strobe);
uint8_t CC1101_read_status(uint8_t addr);

CC1101_Status_t CC1101_enter_rx_mode(void);

ReceiveState_t CC1101_receive_packet(uint8_t* rxBuffer, uint8_t* length);
CC1101_Status_t CC1101_transmitt_packet(const char* packet_loc, uint8_t len);

void CC1101_write_settings();

uint8_t CC1101_get_RSSI(void);
void CC1101_setMHZ(float mhz);
CC1101_Status_t CC1101_set_deviation(uint32_t freq_dev_hz);

uint8_t CC1101_getRssiRaw(void);
int16_t CC1101_RSSIconvert(char raw_rssi);
uint16_t CC1101_autoCalibrate1(void);
uint8_t CC1101_getLqi(void);

void CC1101_GDO0_flag_clear(void);
bool CC1101_GDO0_flag_get(void);
void CC1101_GDO0_flag_set(void);

void CC1101_setCarrierFreq(uint8_t f);
void CC1101_setDevAddress(uint8_t a);

#endif /* CC1101_H */
