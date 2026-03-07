/*
 * RF Driver CC1101 TI
 *
 *  Created on: Mar 11, 2020
 *      Author: suleyman.eskil but the library has Mr. Ilynx
 *      https://www.freelancer.com/u/ilynx?ref_project_id=24212020
 * 
 *  Updated 2024 Malyshev Sergey
 *      https://github.com/sergey12malyshev
 * 
 *  Attention! The speed of the SPI bus is no more than 10 MHz!
 */

#include <stdbool.h>
#include <stdint.h>

#include "cc1101.h"
#include "dw_stm32_delay.h"

#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_exti.h"

#define WRITE_BURST             0x40
#define READ_SINGLE             0x80
#define READ_BURST              0xC0


#define BYTES_IN_RXFIFO         0x7F
#define LQI                     1
#define RSSI                    0U
#define CRC_OK                  0x80


#define PKTSTATUS_CCA           0x10
#define PKTSTATUS_CS            0x40

#define TIMEOUT_SPI_MS          250U


#ifndef CC1101_GDO_EXTI_LINE
#define CC1101_GDO_EXTI_LINE LL_EXTI_LINE_12
#endif

#ifndef CC1101_GDO_IRQn
#define CC1101_GDO_IRQn     EXTI15_10_IRQn
#endif

static SPI_HandleTypeDef* hal_spi;

typedef struct {
  GPIO_TypeDef *port;
  uint32_t pin;
} CC1101_pin_t;

static CC1101_pin_t cs_pin;
static CC1101_pin_t miso_pin;
static CC1101_pin_t gdo_pin;

static volatile bool GDO0_flag;

void CC1101_GDO0_flag_clear(void)
{
  LL_EXTI_ClearFlag_0_31(CC1101_GDO_EXTI_LINE); //GDO
  NVIC_EnableIRQ(CC1101_GDO_IRQn); //GDO
  GDO0_flag = false;
}

bool CC1101_GDO0_flag_get(void)
{
  return GDO0_flag;
}

void CC1101_GDO0_flag_set(void)
{
  GDO0_flag = true;
}

static inline void __spi_cs_set(void)
{
  LL_GPIO_SetOutputPin(cs_pin.port, cs_pin.pin);
}

static inline void __spi_cs_reset(void)
{
  LL_GPIO_ResetOutputPin(cs_pin.port, cs_pin.pin);
}

static inline bool __spi_miso_isSet(void)
{
  return LL_GPIO_IsInputPinSet(miso_pin.port, miso_pin.pin);
}

static inline bool __gdo_pin_isSet(void)
{
  return LL_GPIO_IsInputPinSet(gdo_pin.port, gdo_pin.pin);
}

void CC1101_IRQHandler(void)
{
  if (!__gdo_pin_isSet())
  {
    CC1101_GDO0_flag_set();
  }
}

static HAL_StatusTypeDef __spi_write(uint8_t *addr, uint8_t *pData, uint16_t size)
{
  HAL_StatusTypeDef status;
  uint32_t tickstart = HAL_GetTick();

  __spi_cs_reset();

  while(__spi_miso_isSet())
  {
    if (((HAL_GetTick() - tickstart) >= (uint32_t) TIMEOUT_SPI_MS))
    {
      return HAL_TIMEOUT;
    }
  };

  status = HAL_SPI_Transmit(hal_spi, addr, 1, (uint32_t) TIMEOUT_SPI_MS);
  if (status == HAL_OK && pData != NULL)
  {
    status = HAL_SPI_Transmit(hal_spi, pData, size, (uint32_t) TIMEOUT_SPI_MS);
  }
    
  __spi_cs_set();
  
  return status;
}

static HAL_StatusTypeDef __spi_read(uint8_t *addr, uint8_t *pData, uint16_t size)
{
  if ((pData == NULL) || (size == 0U))
  {
    return HAL_ERROR;
  }

  __spi_cs_reset();

  uint32_t tickstart = HAL_GetTick();

  while(__spi_miso_isSet())
  {
    if (((HAL_GetTick() - tickstart) >= (uint32_t) TIMEOUT_SPI_MS))
    {
      return HAL_TIMEOUT;
    }
  };

  HAL_StatusTypeDef status = HAL_SPI_Transmit(hal_spi, addr, 1, (uint32_t) TIMEOUT_SPI_MS);
  if (status == HAL_OK)
  {
    status = HAL_SPI_Receive(hal_spi, pData, size, (uint32_t) TIMEOUT_SPI_MS);
  }
  __spi_cs_set();

  return status;
}

static void cc1101_write_reg(uint8_t addr, uint8_t value)
{
  __spi_write(&addr, &value, 1);
}

static HAL_StatusTypeDef cc1101_write_burst_reg(uint8_t addr, uint8_t* buffer, uint8_t count)
{
  addr = (addr | WRITE_BURST);
  return __spi_write(&addr, buffer, count);
}

void CC1101_strobe(uint8_t strobe)
{
  __spi_write(&strobe, 0, 0);
}


static uint8_t cc1101_read_reg(uint8_t addr)
{
  uint8_t data;
  addr = (addr | READ_SINGLE);
  __spi_read(&addr, &data, 1);
  return data;
}

uint8_t CC1101_read_status(uint8_t addr)
{
  uint8_t data;
  addr = (addr | READ_BURST);
  __spi_read(&addr, &data, 1);
  return data;
}

static void cc1101_read_burst_reg(uint8_t addr, uint8_t* buffer, uint8_t count)
{
  addr = (addr | READ_BURST);
  __spi_read(&addr, buffer, count);
}

static uint8_t rssi = 0;

uint8_t CC1101_get_RSSI(void)
{
  return rssi;
}

ReceiveState_t CC1101_receive_packet(uint8_t* rxBuffer, uint8_t *length)
{
  uint8_t status[2] = {0};
  uint8_t packet_len = 0;
  // This status register is safe to read since it will not be updated after
  // the packet has been received (See the CC1100 and 2500 Errata Note)
  if (CC1101_read_status(CCxxx0_RXBYTES) & BYTES_IN_RXFIFO)
  {
    // Read length byte
    packet_len = cc1101_read_reg(CCxxx0_RXFIFO);

    // Read data from RX FIFO and store in rxBuffer
    if (packet_len <= *length)
    {
      cc1101_read_burst_reg(CCxxx0_RXFIFO, rxBuffer, packet_len);
      *length = packet_len;

      // Read the 2 appended status bytes (status[0] = RSSI, status[1] = LQI)
      cc1101_read_burst_reg(CCxxx0_RXFIFO, status, 2);

      // MSB of LQI is the CRC_OK bit
      rssi = status[RSSI];

      return(status[LQI] & CRC_OK);
    }
    else
    {
      *length = packet_len;

      // Make sure that the radio is in IDLE state before flushing the FIFO
      // (Unless RXOFF_MODE has been changed, the radio should be in IDLE state at this point)
      CC1101_strobe(CCxxx0_SIDLE);

      // Flush RX FIFO
      CC1101_strobe(CCxxx0_SFRX);

      return RX_ERR_LENGHT;
    }
  }
  else
  {
    return RX_ERR_RX;
  } 
}

#define FIFO_LEN                64U

static CC1101_Status_t CC1101_send_packet(uint8_t* txBuffer, uint8_t size)
{
  if (txBuffer == NULL)
  {
    return CC1101_ERROR_NO_MESSAGE;
  }

  CC1101_strobe(CCxxx0_SIDLE);

  cc1101_write_reg(CCxxx0_TXFIFO, size);

  if (CC1101_read_status(CCxxx0_TXBYTES) > FIFO_LEN) 
  {
    return CC1101_ERROR_OVERFLOW;
  }

  HAL_StatusTypeDef status = cc1101_write_burst_reg(CCxxx0_TXFIFO, txBuffer, size);

  if (status != HAL_OK) 
  {
    return CC1101_ERROR_SPI;
  }

  if (CC1101_read_status(CCxxx0_TXBYTES) > FIFO_LEN)
  {
    return CC1101_ERROR_OVERFLOW;
  }

  CC1101_strobe(CCxxx0_STX);

  return CC1101_OK;
}

CC1101_Status_t CC1101_transmitt_packet(const char *packet_loc, uint8_t len)
{
  assert_param(packet_loc != NULL);
  assert_param(len > 0);
  assert_param(len <= 61); // CC1101 FIFO size

  uint8_t tx_bytes = CC1101_read_status(CCxxx0_TXBYTES);

  if (tx_bytes > 0)
  {
    CC1101_strobe(CCxxx0_SFTX); // flush the buffer

    DWT_Delay_us(1);
  }
  
  CC1101_Status_t status = CC1101_send_packet((uint8_t *)packet_loc, len);

  if (status > CC1101_OK)
  {
    return status;
  }

    CC1101_GDO0_flag_clear();
    uint32_t tickstart = HAL_GetTick();
    while (!CC1101_GDO0_flag_get())
    {
      if ((HAL_GetTick() - tickstart) >= TIMEOUT_SPI_MS) // например, 100 мс
      {
        CC1101_strobe(CCxxx0_SIDLE); // reset cc1101
        CC1101_strobe(CCxxx0_SFTX);

        return CC1101_TIMEOUT;
      }
    }

  uint8_t status_tx = CC1101_read_status(CCxxx0_TXBYTES);     // it is checking to send the data
  
  if (status_tx > 0)
  {
    CC1101_strobe(CCxxx0_SFTX);
  }

  return CC1101_OK;
}


/*
  FSK is better than GFSK in range
  4FSK - to get the highest data transfer rate, but you will lose the range.
  The band for 2FSK: bitrate + 2* deviation
*/
void CC1101_write_settings(void)
{
// Address Config = No address check 
// Base Frequency = 432.999817 
// CRC Autoflush = false 
// CRC Enable = true 
// Carrier Frequency = 433.999573 
// Channel Number = 10 
// Channel Spacing = 99.975586 
// Data Format = Normal mode 
// Data Rate = 3.00026 
// Deviation = 2.975464 
// Device Address = 0 
// Manchester Enable = false 
// Modulated = true 
// Modulation Format = 2-FSK 
// PA Ramping = false 
// Packet Length = 20 
// Packet Length Mode = Variable packet length mode. Packet length configured by the first byte after sync word 
// Preamble Count = 4 
// RX Filter BW = 58.035714 
// Sync Word Qualifier Mode = 16/16 + carrier-sense above threshold 
// TX Power = 10 
// Whitening = false 
// PA table 
#define PA_TABLE {0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
//
// Rf settings for CC1101
//
cc1101_write_reg(CCxxx0_IOCFG0, 0x06);  //GDO0 Output Pin Configuration
cc1101_write_reg(CCxxx0_FIFOTHR, 0x47); //RX FIFO and TX FIFO Thresholds
cc1101_write_reg(CCxxx0_SYNC1, 0x7A);   //Sync Word, High Byte
cc1101_write_reg(CCxxx0_SYNC0, 0x0E);   //Sync Word, Low Byte
cc1101_write_reg(CCxxx0_PKTLEN, 0x14);  //Packet Length
cc1101_write_reg(CCxxx0_PKTCTRL0, 0x05);//Packet Automation Control
cc1101_write_reg(CCxxx0_CHANNR, 0x0A);  //Channel Number
cc1101_write_reg(CCxxx0_FSCTRL1, 0x06); //Frequency Synthesizer Control
cc1101_write_reg(CCxxx0_FREQ2, 0x10);   //Frequency Control Word, High Byte
cc1101_write_reg(CCxxx0_FREQ1, 0xA7);   //Frequency Control Word, Middle Byte
cc1101_write_reg(CCxxx0_FREQ0, 0x62);   //Frequency Control Word, Low Byte
cc1101_write_reg(CCxxx0_MDMCFG4, 0xF6); //Modem Configuration
cc1101_write_reg(CCxxx0_MDMCFG3, 0xE4); //Modem Configuration
cc1101_write_reg(CCxxx0_MDMCFG2, 0x06); //Modem Configuration
cc1101_write_reg(CCxxx0_MDMCFG1, 0x21); //Modem Configuration
cc1101_write_reg(CCxxx0_DEVIATN, 0x07); //Modem Deviation Setting
cc1101_write_reg(CCxxx0_MCSM0, 0x18);   //Main Radio Control State Machine Configuration
cc1101_write_reg(CCxxx0_FOCCFG, 0x16);  //Frequency Offset Compensation Configuration
cc1101_write_reg(CCxxx0_AGCCTRL2, 0x43);//AGC Control
cc1101_write_reg(CCxxx0_AGCCTRL1, 0x49);//AGC Control
cc1101_write_reg(CCxxx0_WORCTRL, 0xFB); //Wake On Radio Control
cc1101_write_reg(CCxxx0_FSCAL3, 0xE9);  //Frequency Synthesizer Calibration
cc1101_write_reg(CCxxx0_FSCAL2, 0x2A);  //Frequency Synthesizer Calibration
cc1101_write_reg(CCxxx0_FSCAL1, 0x00);  //Frequency Synthesizer Calibration
cc1101_write_reg(CCxxx0_FSCAL0, 0x1F);  //Frequency Synthesizer Calibration
cc1101_write_reg(CCxxx0_TEST2, 0x81);   //Various Test Settings
cc1101_write_reg(CCxxx0_TEST1, 0x35);   //Various Test Settings
cc1101_write_reg(CCxxx0_TEST0, 0x09);   //Various Test Settings

}


//For 433MHz, +10dBm
//it is also high
//uint8_t paTable[] = {0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0};

uint8_t paTable[] = PA_TABLE;


/**
 * setCarrierFreq
 * 
 * Set carrier frequency
 * 
 * 'freq'  New carrier frequency
 */
void setCarrierFreqRegister(const uint8_t freq)
{
// Carrier frequency = 868 MHz
#define CCxxx0_DEFVAL_FREQ2_868  0x21        // Frequency Control Word, High Byte
#define CCxxx0_DEFVAL_FREQ1_868  0x62        // Frequency Control Word, Middle Byte
#define CCxxx0_DEFVAL_FREQ0_868  0x76        // Frequency Control Word, Low Byte
// Carrier frequency = 902 MHz
#define CCxxx0_DEFVAL_FREQ2_915  0x22        // Frequency Control Word, High Byte
#define CCxxx0_DEFVAL_FREQ1_915  0xB1        // Frequency Control Word, Middle Byte
#define CCxxx0_DEFVAL_FREQ0_915  0x3B        // Frequency Control Word, Low Byte
// Carrier frequency = 918 MHz
#define CCxxx0_DEFVAL_FREQ2_918  0x23        // Frequency Control Word, High Byte
#define CCxxx0_DEFVAL_FREQ1_918  0x4E        // Frequency Control Word, Middle Byte
#define CCxxx0_DEFVAL_FREQ0_918  0xC4        // Frequency Control Word, Low Byte
// Carrier frequency = 433 MHz
#define CCxxx0_DEFVAL_FREQ2_433  0x10        // Frequency Control Word, High Byte
#define CCxxx0_DEFVAL_FREQ1_433  0xB4        // Frequency Control Word, Middle Byte
#define CCxxx0_DEFVAL_FREQ0_433  0x2E        // Frequency Control Word, Low Byte

  switch(freq)
  {
    case CFREQ_915:
      cc1101_write_reg(CCxxx0_FREQ2,  CCxxx0_DEFVAL_FREQ2_915);
      cc1101_write_reg(CCxxx0_FREQ1,  CCxxx0_DEFVAL_FREQ1_915);
      cc1101_write_reg(CCxxx0_FREQ0,  CCxxx0_DEFVAL_FREQ0_915);
      break;
    case CFREQ_433:
      cc1101_write_reg(CCxxx0_FREQ2,  CCxxx0_DEFVAL_FREQ2_433);
      cc1101_write_reg(CCxxx0_FREQ1,  CCxxx0_DEFVAL_FREQ1_433);
      cc1101_write_reg(CCxxx0_FREQ0,  CCxxx0_DEFVAL_FREQ0_433);
      break;
    case CFREQ_918:
      cc1101_write_reg(CCxxx0_FREQ2,  CCxxx0_DEFVAL_FREQ2_918);
      cc1101_write_reg(CCxxx0_FREQ1,  CCxxx0_DEFVAL_FREQ1_918);
      cc1101_write_reg(CCxxx0_FREQ0,  CCxxx0_DEFVAL_FREQ0_918);
      break;
    default:
      cc1101_write_reg(CCxxx0_FREQ2,  CCxxx0_DEFVAL_FREQ2_868);
      cc1101_write_reg(CCxxx0_FREQ1,  CCxxx0_DEFVAL_FREQ1_868);
      cc1101_write_reg(CCxxx0_FREQ0,  CCxxx0_DEFVAL_FREQ0_868);
      break;
  }
}

static uint8_t carrierFreq = CFREQ_433;

void CC1101_setCarrierFreq(uint8_t f)
{
  carrierFreq = f;
}

/**
 * setDevAddress
 * 
 * Set device address
 * 
 * @param addr  Device address
 */
void CC1101_setDevAddressRegister(uint8_t addr) 
{
  cc1101_write_reg(CCxxx0_ADDR, addr);    //Device Address
}

static uint8_t devAddress = 0;

void CC1101_setDevAddress(uint8_t a) 
{
  devAddress = a;
}

void CC1101_write_settingsOld(void)
{
#define ADRESS_CHECK_EN   0
#define LOWSPEED_EN       1

  // Address Config = No address check
  // Base Frequency = 432.999817
  // CRC Autoflush = false
  // CRC Enable = true
  // Carrier Frequency = 432.999817
  // Channel Number = 0
  // Channel Spacing = 199.951172
  // Data Format = Normal mode
  // Data Rate = 1.19948
  // Deviation = 25.390625
  // Device Address = 0
  // Manchester Enable = false
  // Modulated = true
  // Modulation Format = GFSK
  // PA Ramping = false
  // Packet Length = 20
  // Packet Length Mode = Variable packet length mode. Packet length configured by the first byte after sync word
  // Preamble Count = 4
  // RX Filter BW = 101.562500
  // Sync Word Qualifier Mode = 16/16 + carrier-sense above threshold
  // TX Power = 0
  // Whitening = false
  //
  // Rf settings for CC1101
  //

  //i checked in smartRF studio 7 of Mr. ilynx's code // the setting is yours
  cc1101_write_reg(CCxxx0_IOCFG2, 0x29);  //GDO2 Output Pin Configuration
  cc1101_write_reg(CCxxx0_IOCFG1, 0x2E);  //GDO1 Output Pin Configuration
  cc1101_write_reg(CCxxx0_IOCFG0, 0x06);  //GDO0 Output Pin Configuration
  cc1101_write_reg(CCxxx0_FIFOTHR, 0x47); //RX FIFO and TX FIFO Thresholds
  cc1101_write_reg(CCxxx0_SYNC1, 0xD3);   //Sync Word, High Byte
  cc1101_write_reg(CCxxx0_SYNC0, 0x91);   //Sync Word, Low Byte
  cc1101_write_reg(CCxxx0_PKTLEN, 0xFF);  //Packet Length
#if ADRESS_CHECK_EN
  cc1101_write_reg(CCxxx0_PKTCTRL1, 0x06);//Packet Automation Control, Enable address check
#else
  cc1101_write_reg(CCxxx0_PKTCTRL1, 0x04);//Packet Automation Control, Disable address check
#endif
  cc1101_write_reg(CCxxx0_PKTCTRL0, 0x05);//Packet Automation Control

  CC1101_setDevAddressRegister(devAddress); //Device Address

  cc1101_write_reg(CCxxx0_CHANNR, 0x00);  //Channel Number
  cc1101_write_reg(CCxxx0_FSCTRL1, 0x08); //Frequency Synthesizer Control
  cc1101_write_reg(CCxxx0_FSCTRL0, 0x00); //Frequency Synthesizer Control

  setCarrierFreqRegister(carrierFreq);

#define CCxxx0_DEFVAL_MDMCFG4_4800     0xC7   // Modem configuration. Speed = 4800 bps
#define CCxxx0_DEFVAL_MDMCFG4_38400    0xCA   // Modem configuration. Speed = 38 Kbps

#if LOWSPEED_EN
    cc1101_write_reg(CCxxx0_MDMCFG4, CCxxx0_DEFVAL_MDMCFG4_4800); //Modem Configuration
#else
  cc1101_write_reg(CCxxx0_MDMCFG4, CCxxx0_DEFVAL_MDMCFG4_38400); //Modem Configuration
#endif
  cc1101_write_reg(CCxxx0_MDMCFG3, 0x83); //Modem Configuration
  cc1101_write_reg(CCxxx0_MDMCFG2, 0x93); //Modem Configuration
  cc1101_write_reg(CCxxx0_MDMCFG1, 0x22); //Modem Configuration
  cc1101_write_reg(CCxxx0_MDMCFG0, 0xF8); //Modem Configuration
  cc1101_write_reg(CCxxx0_DEVIATN, 0x34); //Modem Deviation Setting
  cc1101_write_reg(CCxxx0_MCSM2, 0x07);   //Main Radio Control State Machine Configuration
  cc1101_write_reg(CCxxx0_MCSM1, 0x30);   //Main Radio Control State Machine Configuration
  cc1101_write_reg(CCxxx0_MCSM0, 0x18);   //Main Radio Control State Machine Configuration
  cc1101_write_reg(CCxxx0_FOCCFG, 0x16);  //Frequency Offset Compensation Configuration
  cc1101_write_reg(CCxxx0_BSCFG, 0x6C);   //Bit Synchronization Configuration
  cc1101_write_reg(CCxxx0_AGCCTRL2, 0x43);//AGC Control
  cc1101_write_reg(CCxxx0_AGCCTRL1, 0x40);//AGC Control
  cc1101_write_reg(CCxxx0_AGCCTRL0, 0x91);//AGC Control
  cc1101_write_reg(CCxxx0_WOREVT1, 0x87); //High Byte Event0 Timeout
  cc1101_write_reg(CCxxx0_WOREVT0, 0x6B); //Low Byte Event0 Timeout
  cc1101_write_reg(CCxxx0_WORCTRL, 0xF8); //Wake On Radio Control
  cc1101_write_reg(CCxxx0_FREND1, 0x56);  //Front End RX Configuration
  cc1101_write_reg(CCxxx0_FREND0, 0x10);  //Front End TX Configuration
  cc1101_write_reg(CCxxx0_FSCAL3, 0xE9);  //Frequency Synthesizer Calibration
  cc1101_write_reg(CCxxx0_FSCAL2, 0x2A);  //Frequency Synthesizer Calibration
  cc1101_write_reg(CCxxx0_FSCAL1, 0x00);  //Frequency Synthesizer Calibration
  cc1101_write_reg(CCxxx0_FSCAL0, 0x1F);  //Frequency Synthesizer Calibration
  cc1101_write_reg(CCxxx0_RCCTRL1, 0x41); //RC Oscillator Configuration
  cc1101_write_reg(CCxxx0_RCCTRL0, 0x00); //RC Oscillator Configuration
  cc1101_write_reg(CCxxx0_FSTEST, 0x59);  //Frequency Synthesizer Calibration Control
  cc1101_write_reg(CCxxx0_PTEST, 0x7F);   //Production Test
  cc1101_write_reg(CCxxx0_AGCTEST, 0x3F); //AGC Test
  cc1101_write_reg(CCxxx0_TEST2, 0x81);   //Various Test Settings
  cc1101_write_reg(CCxxx0_TEST1, 0x35);   //Various Test Settings
  cc1101_write_reg(CCxxx0_TEST0, 0x09);   //Various Test Settings
}

CC1101_Status_t CC1101_init(SPI_HandleTypeDef* hspi, 
                    GPIO_TypeDef* cs_port, uint16_t _cs_pin,
                    GPIO_TypeDef* miso_port, uint16_t _miso_pin,
                    GPIO_TypeDef* gdo_port, uint16_t _gdo_pin)
{
  hal_spi = hspi;
  
  cs_pin.port = cs_port;
  cs_pin.pin = _cs_pin;
  
  miso_pin.port = miso_port;
  miso_pin.pin = _miso_pin;
  
  gdo_pin.port = gdo_port;
  gdo_pin.pin = _gdo_pin;

  if ((hal_spi == NULL) || (cs_pin.port == NULL) || (cs_pin.pin == 0))
  {
    return CC1101_ERROR_CONFIG;
  }

    // Power-up reset
  if (CC1101_power_up_reset())
  {
    return CC1101_ERROR;
  }

    // check version
  for(int i = 0; i < 20; i++)
  {
    uint8_t version = CC1101_read_status(CCxxx0_VERSION);
  
    if (version != 0x04 && version != 0x14 && version != 0x17)
    {
      if (version == 0x00) 
      {
        return CC1101_ERROR;
      }
      return CC1101_ERROR_VERSION;
    }
    else
    {
      break;
    }

    if (i == 19)
    {
      return CC1101_ERROR;
    }
  }

  CC1101_strobe(CCxxx0_SFRX); //RX FIFO
  CC1101_strobe(CCxxx0_SFTX); //TX FIFO
  CC1101_write_settings();
  cc1101_write_burst_reg(CCxxx0_PATABLE, paTable, 8);

  cc1101_write_reg(CCxxx0_FIFOTHR, 0x07);

  CC1101_strobe(CCxxx0_SIDLE);
  CC1101_strobe(CCxxx0_SFRX);
  CC1101_strobe(CCxxx0_SFTX);

  CC1101_strobe(CCxxx0_SIDLE);

  return CC1101_OK;
}

CC1101_Status_t CC1101_reinit(void)
{
  if ((hal_spi == NULL) || (cs_pin.port == NULL) || (cs_pin.pin == 0))
  {
    return CC1101_ERROR_CONFIG;
  }

  CC1101_GDO0_flag_clear();

  CC1101_strobe(CCxxx0_SFRX); //RX FIFO
  CC1101_strobe(CCxxx0_SFTX); //TX FIFO
  CC1101_write_settings();
  cc1101_write_burst_reg(CCxxx0_PATABLE, paTable, 8);

  cc1101_write_reg(CCxxx0_FIFOTHR, 0x07);

  CC1101_strobe(CCxxx0_SIDLE);
  CC1101_strobe(CCxxx0_SFRX);
  CC1101_strobe(CCxxx0_SFTX);

  CC1101_strobe(CCxxx0_SIDLE);

  return CC1101_OK;
}

CC1101_Status_t CC1101_power_up_reset(void)
{
  const uint32_t waiting = 450;

  if ((hal_spi == NULL) || (cs_pin.port == NULL) || (cs_pin.pin == 0))
  {
    return CC1101_ERROR_CONFIG;
  }

  DWT_Delay_Init();
  __spi_cs_set();
  DWT_Delay_us(1);
  __spi_cs_reset();
  DWT_Delay_us(1);
  __spi_cs_set();
  DWT_Delay_us(41);

  __spi_cs_reset();

  uint32_t timeStamp = HAL_GetTick();

  while(__spi_miso_isSet())
  {
    if (HAL_GetTick() - timeStamp > waiting)
    {
      return CC1101_TIMEOUT;
    }
  }

  CC1101_strobe(CCxxx0_SRES);
  __spi_cs_set();

  return CC1101_OK;
}

void CC1101_goSleep(void)
{
  CC1101_strobe(CCxxx0_SIDLE);
  CC1101_strobe(CCxxx0_SPWD);
}

uint8_t CC1101_getLqi(void)
{
  uint8_t lqi = CC1101_read_status(CCxxx0_LQI);
  return lqi;
}

uint8_t CC1101_getRssiRaw(void)
{
  uint8_t rssi_raw = CC1101_read_status(CCxxx0_RSSI);
  return rssi_raw;
}

int16_t CC1101_RSSIconvert(char raw_rssi)
{
  const uint8_t rssi_offset = 74;

  uint8_t rssi_dec = (uint8_t)raw_rssi;

  if (rssi_dec >= 128)
  {
    return ((int16_t)(rssi_dec - 256) / 2) - rssi_offset;
  }
  else
  {
    return (int16_t)(rssi_dec / 2) - rssi_offset;
  }
}

/*
Frequency Calculator
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

  cc1101_write_reg(CCxxx0_FREQ2, freq2);
  cc1101_write_reg(CCxxx0_FREQ1, freq1);
  cc1101_write_reg(CCxxx0_FREQ0, freq0);
}

uint16_t CC1101_autoCalibrate1(void)
{
  static uint16_t accumulatedOffset = 0;

  uint16_t offset = CC1101_read_status(CCxxx0_FREQEST);
  if (offset != 0)
  {
    accumulatedOffset += offset;
    cc1101_write_reg(CCxxx0_FSCTRL0, accumulatedOffset);
  }

  return accumulatedOffset;
}

uint16_t CC1101_autoCalibrate0(void)
{
  uint16_t offset = CC1101_read_status(CCxxx0_FREQEST);

  if (offset != 0)
  {
    cc1101_write_reg(CCxxx0_FSCTRL0, offset);
  }

  return offset;
}


/*
* PA Power set for 378 - 464 mhz!
*/

static uint8_t _PA_TABLE[8] = {0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

bool CC1101_setPower(int pa, float MHz, Modulation_t modulation)
{
  assert_param(MHz >= 378 && MHz <= 464);
  assert_param(modulation <= _MSK);

	        //                       -30  -20  -15  -10   0    5    7    10
  //const uint8_t PA_TABLE_315[8] = {0x12, 0x0D, 0x1C, 0x34, 0x51, 0x85, 0xCB, 0xC2,};             //300 - 348
  const uint8_t PA_TABLE_433[8] = {0x12, 0x0E, 0x1D, 0x34, 0x60, 0x84, 0xC8, 0xC0,};             //387 - 464

  int a = 0;

  if (MHz >= 378 && MHz <= 464)
  {
    if (pa <= -30)                   {a = PA_TABLE_433[0];}
    else if (pa > -30 && pa <= -20)  {a = PA_TABLE_433[1];}
    else if (pa > -20 && pa <= -15)  {a = PA_TABLE_433[2];}
    else if (pa > -15 && pa <= -10)  {a = PA_TABLE_433[3];}
    else if (pa > -10 && pa <= 0)    {a = PA_TABLE_433[4];}
    else if (pa > 0 && pa <= 5)      {a = PA_TABLE_433[5];}
    else if (pa > 5 && pa <= 7)      {a = PA_TABLE_433[6];}
    else if (pa > 7)                 {a = PA_TABLE_433[7];}
  }
  else
  {
    return true; // error freq!!
  }

  if (modulation == _ASK)
  {
    _PA_TABLE[0] = 0;  
    _PA_TABLE[1] = a;
  }
  else
  {
    _PA_TABLE[0] = a;  
    _PA_TABLE[1] = 0; 
  }

  cc1101_write_burst_reg(CCxxx0_PATABLE, _PA_TABLE, 8);
  
  return false;
}