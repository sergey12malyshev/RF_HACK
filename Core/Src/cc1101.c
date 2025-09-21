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

#include "cc1101.h"
#include "dw_stm32_delay.h"

#include "cli_driver.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef CC1101_CLI_ENABLE
  #include "cli_driver.h"
#endif //CC1101_CLI_ENABLE

#define WRITE_BURST             0x40
#define READ_SINGLE             0x80
#define READ_BURST              0xC0


#define BYTES_IN_RXFIFO         0x7F
#define LQI                     1
#define RSSI                    0U
#define CRC_OK                  0x80


#define PKTSTATUS_CCA           0x10
#define PKTSTATUS_CS            0x40

#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_exti.h"
// TODO: configure ports via a function call !!!
#define PORT_MISO GPIOB
#define PIN_MISO LL_GPIO_PIN_14

#define PORT_GDO GPIOB
#define PIN_GDO LL_GPIO_PIN_12

volatile uint8_t GDO0_flag;

static SPI_HandleTypeDef* hal_spi;
static uint16_t CS_Pin;
static GPIO_TypeDef* CS_GPIO_Port;

void CC1101_GDO0_flag_clear(void)
{
  LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_12); //GDO
  NVIC_EnableIRQ(EXTI15_10_IRQn); //GDO
  GDO0_flag = 0;
}

HAL_StatusTypeDef __spi_write(uint8_t *addr, uint8_t *pData, uint16_t size)
{
  HAL_StatusTypeDef status;

  LL_GPIO_ResetOutputPin(CS_GPIO_Port, CS_Pin);
  while(LL_GPIO_IsInputPinSet(PORT_MISO, PIN_MISO)){};

  status = HAL_SPI_Transmit(hal_spi, addr, 1, 0xFFFF);
  if (status == HAL_OK && pData != NULL)
  {
    status = HAL_SPI_Transmit(hal_spi, pData, size, 0xFFFF);
  }
    
  LL_GPIO_SetOutputPin(CS_GPIO_Port, CS_Pin);
  
  return status;
}

HAL_StatusTypeDef __spi_read(uint8_t *addr, uint8_t *pData, uint16_t size)
{
  HAL_StatusTypeDef status;

  LL_GPIO_ResetOutputPin(CS_GPIO_Port, CS_Pin);
  while(LL_GPIO_IsInputPinSet(PORT_MISO, PIN_MISO)){};


  status = HAL_SPI_Transmit(hal_spi, addr, 1, 0xFFFF);
  status = HAL_SPI_Receive(hal_spi, pData, size, 0xFFFF);


  LL_GPIO_SetOutputPin(CS_GPIO_Port, CS_Pin);

  return status;
}

void TI_write_reg(UINT8 addr, UINT8 value)
{
  __spi_write(&addr, &value, 1);
}

void TI_write_burst_reg(uint8_t addr, uint8_t* buffer, uint8_t count)
{
  addr = (addr | WRITE_BURST);
  __spi_write(&addr, buffer, count);
}

void TI_strobe(uint8_t strobe)
{
  __spi_write(&strobe, 0, 0);
}


uint8_t TI_read_reg(uint8_t addr)
{
  uint8_t data;
  addr = (addr | READ_SINGLE);
  __spi_read(&addr, &data, 1);
  return data;
}

uint8_t TI_read_status(uint8_t addr)
{
  uint8_t data;
  addr = (addr | READ_BURST);
  __spi_read(&addr, &data, 1);
  return data;
}

void TI_read_burst_reg(uint8_t addr, uint8_t* buffer, uint8_t count)
{
  addr = (addr | READ_BURST);
  __spi_read(&addr, buffer, count);
}

static uint8_t rssi = 0;

unsigned char get_RSSI(void)
{
  return rssi;
}

ResiveSt TI_receive_packet(uint8_t* rxBuffer, UINT8 *length)
{
  uint8_t status[2];
  UINT8 packet_len;
  // This status register is safe to read since it will not be updated after
  // the packet has been received (See the CC1100 and 2500 Errata Note)
  if (TI_read_status(CCxxx0_RXBYTES) & BYTES_IN_RXFIFO)
  {
    // Read length byte
    packet_len = TI_read_reg(CCxxx0_RXFIFO);

    // Read data from RX FIFO and store in rxBuffer
    if (packet_len <= *length)
    {
      TI_read_burst_reg(CCxxx0_RXFIFO, rxBuffer, packet_len);
      *length = packet_len;

      // Read the 2 appended status bytes (status[0] = RSSI, status[1] = LQI)
      TI_read_burst_reg(CCxxx0_RXFIFO, status, 2);

      // MSB of LQI is the CRC_OK bit
      rssi = status[RSSI];

      return(status[LQI] & CRC_OK);
    }
    else
    {
      *length = packet_len;

      // Make sure that the radio is in IDLE state before flushing the FIFO
      // (Unless RXOFF_MODE has been changed, the radio should be in IDLE state at this point)
      TI_strobe(CCxxx0_SIDLE);

      // Flush RX FIFO
      TI_strobe(CCxxx0_SFRX);
      return(RX_ERR_LENGHT);
    }
  }
  else
  {
    return(RX_ERR_RX);
  } 
}

void TI_send_packet(uint8_t* txBuffer, UINT8 size)
{
  __attribute__((unused)) uint8_t status;

    TI_strobe(CCxxx0_SIDLE);

    TI_write_reg(CCxxx0_TXFIFO, size);

    status = TI_read_status(CCxxx0_TXBYTES);

    TI_write_burst_reg(CCxxx0_TXFIFO, txBuffer, 7);

    status = TI_read_status(CCxxx0_TXBYTES);

    TI_strobe(CCxxx0_STX);
}

/*
  FSK is better than GFSK in range
  4FSK - to get the highest data transfer rate, but you will lose the range.
  The band for 2FSK: bitrate + 2* deviation
*/
void TI_write_settings(void)
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
#define PA_TABLE {0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
//
// Rf settings for CC1101
//
TI_write_reg(CCxxx0_IOCFG0,0x06);  //GDO0 Output Pin Configuration
TI_write_reg(CCxxx0_FIFOTHR,0x47); //RX FIFO and TX FIFO Thresholds
TI_write_reg(CCxxx0_SYNC1,0x7A);   //Sync Word, High Byte
TI_write_reg(CCxxx0_SYNC0,0x0E);   //Sync Word, Low Byte
TI_write_reg(CCxxx0_PKTLEN,0x14);  //Packet Length
TI_write_reg(CCxxx0_PKTCTRL0,0x05);//Packet Automation Control
TI_write_reg(CCxxx0_CHANNR,0x0A);  //Channel Number
TI_write_reg(CCxxx0_FSCTRL1,0x06); //Frequency Synthesizer Control
TI_write_reg(CCxxx0_FREQ2,0x10);   //Frequency Control Word, High Byte
TI_write_reg(CCxxx0_FREQ1,0xA7);   //Frequency Control Word, Middle Byte
TI_write_reg(CCxxx0_FREQ0,0x62);   //Frequency Control Word, Low Byte
TI_write_reg(CCxxx0_MDMCFG4,0xF6); //Modem Configuration
TI_write_reg(CCxxx0_MDMCFG3,0xE4); //Modem Configuration
TI_write_reg(CCxxx0_MDMCFG2,0x06); //Modem Configuration
TI_write_reg(CCxxx0_MDMCFG1,0x21); //Modem Configuration
TI_write_reg(CCxxx0_DEVIATN,0x07); //Modem Deviation Setting
TI_write_reg(CCxxx0_MCSM0,0x18);   //Main Radio Control State Machine Configuration
TI_write_reg(CCxxx0_FOCCFG,0x16);  //Frequency Offset Compensation Configuration
TI_write_reg(CCxxx0_AGCCTRL2,0x43);//AGC Control
TI_write_reg(CCxxx0_AGCCTRL1,0x49);//AGC Control
TI_write_reg(CCxxx0_WORCTRL,0xFB); //Wake On Radio Control
TI_write_reg(CCxxx0_FSCAL3,0xE9);  //Frequency Synthesizer Calibration
TI_write_reg(CCxxx0_FSCAL2,0x2A);  //Frequency Synthesizer Calibration
TI_write_reg(CCxxx0_FSCAL1,0x00);  //Frequency Synthesizer Calibration
TI_write_reg(CCxxx0_FSCAL0,0x1F);  //Frequency Synthesizer Calibration
TI_write_reg(CCxxx0_TEST2,0x81);   //Various Test Settings
TI_write_reg(CCxxx0_TEST1,0x35);   //Various Test Settings
TI_write_reg(CCxxx0_TEST0,0x09);   //Various Test Settings

}


//For 433MHz, +10dBm
//it is also high
//uint8_t paTable[] = {0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0};

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
      TI_write_reg(CCxxx0_FREQ2,  CCxxx0_DEFVAL_FREQ2_915);
      TI_write_reg(CCxxx0_FREQ1,  CCxxx0_DEFVAL_FREQ1_915);
      TI_write_reg(CCxxx0_FREQ0,  CCxxx0_DEFVAL_FREQ0_915);
      break;
    case CFREQ_433:
      TI_write_reg(CCxxx0_FREQ2,  CCxxx0_DEFVAL_FREQ2_433);
      TI_write_reg(CCxxx0_FREQ1,  CCxxx0_DEFVAL_FREQ1_433);
      TI_write_reg(CCxxx0_FREQ0,  CCxxx0_DEFVAL_FREQ0_433);
      break;
    case CFREQ_918:
      TI_write_reg(CCxxx0_FREQ2,  CCxxx0_DEFVAL_FREQ2_918);
      TI_write_reg(CCxxx0_FREQ1,  CCxxx0_DEFVAL_FREQ1_918);
      TI_write_reg(CCxxx0_FREQ0,  CCxxx0_DEFVAL_FREQ0_918);
      break;
    default:
      TI_write_reg(CCxxx0_FREQ2,  CCxxx0_DEFVAL_FREQ2_868);
      TI_write_reg(CCxxx0_FREQ1,  CCxxx0_DEFVAL_FREQ1_868);
      TI_write_reg(CCxxx0_FREQ0,  CCxxx0_DEFVAL_FREQ0_868);
      break;
  }
}

static uint8_t carrierFreq = CFREQ_433;

void TI_setCarrierFreq(uint8_t f)
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
void TI_setDevAddressRegister(uint8_t addr) 
{
  TI_write_reg(CCxxx0_ADDR, addr);    //Device Address
}

static uint8_t devAddress = 0;
void TI_setDevAddress(uint8_t a) 
{
  devAddress = a;
}

void TI_write_settingsOld(void)
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
  TI_write_reg(CCxxx0_IOCFG2,0x29);  //GDO2 Output Pin Configuration
  TI_write_reg(CCxxx0_IOCFG1,0x2E);  //GDO1 Output Pin Configuration
  TI_write_reg(CCxxx0_IOCFG0,0x06);  //GDO0 Output Pin Configuration
  TI_write_reg(CCxxx0_FIFOTHR,0x47); //RX FIFO and TX FIFO Thresholds
  TI_write_reg(CCxxx0_SYNC1,0xD3);   //Sync Word, High Byte
  TI_write_reg(CCxxx0_SYNC0,0x91);   //Sync Word, Low Byte
  TI_write_reg(CCxxx0_PKTLEN,0xFF);  //Packet Length
#if ADRESS_CHECK_EN
  TI_write_reg(CCxxx0_PKTCTRL1,0x06);//Packet Automation Control, Enable address check
#else
  TI_write_reg(CCxxx0_PKTCTRL1,0x04);//Packet Automation Control, Disable address check
#endif
  TI_write_reg(CCxxx0_PKTCTRL0,0x05);//Packet Automation Control

  TI_setDevAddressRegister(devAddress); //Device Address

  TI_write_reg(CCxxx0_CHANNR,0x00);  //Channel Number
  TI_write_reg(CCxxx0_FSCTRL1,0x08); //Frequency Synthesizer Control
  TI_write_reg(CCxxx0_FSCTRL0,0x00); //Frequency Synthesizer Control

   setCarrierFreqRegister(carrierFreq);

#define CCxxx0_DEFVAL_MDMCFG4_4800     0xC7   // Modem configuration. Speed = 4800 bps
#define CCxxx0_DEFVAL_MDMCFG4_38400    0xCA   // Modem configuration. Speed = 38 Kbps

#if LOWSPEED_EN
    TI_write_reg(CCxxx0_MDMCFG4, CCxxx0_DEFVAL_MDMCFG4_4800); //Modem Configuration
#else
  TI_write_reg(CCxxx0_MDMCFG4, CCxxx0_DEFVAL_MDMCFG4_38400); //Modem Configuration
#endif
  TI_write_reg(CCxxx0_MDMCFG3,0x83); //Modem Configuration
  TI_write_reg(CCxxx0_MDMCFG2,0x93); //Modem Configuration
  TI_write_reg(CCxxx0_MDMCFG1,0x22); //Modem Configuration
  TI_write_reg(CCxxx0_MDMCFG0,0xF8); //Modem Configuration
  TI_write_reg(CCxxx0_DEVIATN,0x34); //Modem Deviation Setting
  TI_write_reg(CCxxx0_MCSM2,0x07);   //Main Radio Control State Machine Configuration
  TI_write_reg(CCxxx0_MCSM1,0x30);   //Main Radio Control State Machine Configuration
  TI_write_reg(CCxxx0_MCSM0,0x18);   //Main Radio Control State Machine Configuration
  TI_write_reg(CCxxx0_FOCCFG,0x16);  //Frequency Offset Compensation Configuration
  TI_write_reg(CCxxx0_BSCFG,0x6C);   //Bit Synchronization Configuration
  TI_write_reg(CCxxx0_AGCCTRL2,0x43);//AGC Control
  TI_write_reg(CCxxx0_AGCCTRL1,0x40);//AGC Control
  TI_write_reg(CCxxx0_AGCCTRL0,0x91);//AGC Control
  TI_write_reg(CCxxx0_WOREVT1,0x87); //High Byte Event0 Timeout
  TI_write_reg(CCxxx0_WOREVT0,0x6B); //Low Byte Event0 Timeout
  TI_write_reg(CCxxx0_WORCTRL,0xF8); //Wake On Radio Control
  TI_write_reg(CCxxx0_FREND1,0x56);  //Front End RX Configuration
  TI_write_reg(CCxxx0_FREND0,0x10);  //Front End TX Configuration
  TI_write_reg(CCxxx0_FSCAL3,0xE9);  //Frequency Synthesizer Calibration
  TI_write_reg(CCxxx0_FSCAL2,0x2A);  //Frequency Synthesizer Calibration
  TI_write_reg(CCxxx0_FSCAL1,0x00);  //Frequency Synthesizer Calibration
  TI_write_reg(CCxxx0_FSCAL0,0x1F);  //Frequency Synthesizer Calibration
  TI_write_reg(CCxxx0_RCCTRL1,0x41); //RC Oscillator Configuration
  TI_write_reg(CCxxx0_RCCTRL0,0x00); //RC Oscillator Configuration
  TI_write_reg(CCxxx0_FSTEST,0x59);  //Frequency Synthesizer Calibration Control
  TI_write_reg(CCxxx0_PTEST,0x7F);   //Production Test
  TI_write_reg(CCxxx0_AGCTEST,0x3F); //AGC Test
  TI_write_reg(CCxxx0_TEST2,0x81);   //Various Test Settings
  TI_write_reg(CCxxx0_TEST1,0x35);   //Various Test Settings
  TI_write_reg(CCxxx0_TEST0,0x09);   //Various Test Settings
}


bool TI_init(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_port, uint32_t cs_pin)
{
  uint8_t status;

  hal_spi = hspi;
  CS_GPIO_Port = cs_port;
  CS_Pin = cs_pin;

  for(int i = 0; i < 20; i++)
  {
    status = TI_read_status(CCxxx0_VERSION);
    if (status == 0x14)
    {
      break;
    }

    if (i == 18)
    {
      return true;
    }
  }

  TI_strobe(CCxxx0_SFRX); //RX FIFO
  TI_strobe(CCxxx0_SFTX); //TX FIFO
  TI_write_settings();
  TI_write_burst_reg(CCxxx0_PATABLE, paTable, 8);

  TI_write_reg(CCxxx0_FIFOTHR, 0x07);

  TI_strobe(CCxxx0_SIDLE);
  TI_strobe(CCxxx0_SFRX);
  TI_strobe(CCxxx0_SFTX);

  TI_strobe(CCxxx0_SIDLE);

  return false;
}


/*
 New driver function:
*/
void CC1101_customSetCSpin(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_port, uint16_t cs_pin)
{
  hal_spi = hspi;
  CS_GPIO_Port = cs_port;
  CS_Pin = cs_pin;
}

bool CC1101_power_up_reset(void)
{
  const uint32_t waiting = 450;

  DWT_Delay_Init();
  LL_GPIO_SetOutputPin(CS_GPIO_Port, CS_Pin);
  DWT_Delay_us(1);
  LL_GPIO_ResetOutputPin(CS_GPIO_Port, CS_Pin);
  DWT_Delay_us(1);
  LL_GPIO_SetOutputPin(CS_GPIO_Port, CS_Pin);
  DWT_Delay_us(41);

  LL_GPIO_ResetOutputPin(CS_GPIO_Port, CS_Pin);

  uint32_t timeStamp = HAL_GetTick();

  while(LL_GPIO_IsInputPinSet(PORT_MISO, PIN_MISO))
  {
    if (HAL_GetTick() - timeStamp > waiting)
    {
      return true;
    }
  }

  TI_strobe(CCxxx0_SRES);
  LL_GPIO_SetOutputPin(CS_GPIO_Port, CS_Pin);

  return false;
}

void CC1101_goSleep(void)
{
  TI_strobe(CCxxx0_SIDLE);
  TI_strobe(CCxxx0_SPWD);
}

uint8_t CC1101_getLqi(void)
{
  uint8_t lqi = TI_read_status(CCxxx0_LQI);
  return lqi;
}

uint8_t CC1101_getRssiRaw(void)
{
  uint8_t rssi_raw = TI_read_status(CCxxx0_RSSI);
  return rssi_raw;
}

int CC1101_RSSIconvert(char raw_rssi)
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

  TI_write_reg(CCxxx0_FREQ2, freq2);
  TI_write_reg(CCxxx0_FREQ1, freq1);
  TI_write_reg(CCxxx0_FREQ0, freq0);
  // Calibrate();
}


uint8_t CC1101_transmittRF(const char *packet_loc, uint8_t len)
{
  uint8_t status = 0;
  
  assert_param(packet_loc != NULL);
  assert_param(len > 0);

  status = TI_read_status(CCxxx0_VERSION);       // it is for checking only (it must be 0x14)
  status = TI_read_status(CCxxx0_TXBYTES);       // it is too
  TI_strobe(CCxxx0_SFTX);                        // flush the buffer

  __ASM volatile ("NOP");

  TI_send_packet((uint8_t *)packet_loc, len);
  //DEBUG_PRINT(CLI_TX"%s %d"CLI_NEW_LINE, packet, len);

  while (LL_GPIO_IsInputPinSet(PORT_GDO, PIN_GDO)) // start transmitt
  {
    __ASM volatile ("NOP");
  }

  while (!LL_GPIO_IsInputPinSet(PORT_GDO, PIN_GDO)) // end transmitt
  {
    __ASM volatile ("NOP");
  }

  status = TI_read_status(CCxxx0_TXBYTES);     // it is checking to send the data

  return status;
}


uint16_t CC1101_autoCalibrate1(void)
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

uint16_t CC1101_autoCalibrate0(void)
{
  uint16_t offset = TI_read_status(CCxxx0_FREQEST);

  if (offset != 0)
  {
    TI_write_reg(CCxxx0_FSCTRL0, offset);
  }

  return offset;
}


/*
* PA Power set for 378 - 464 mhz!
*/

static uint8_t _PA_TABLE[8] = {0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00};

bool CC1101_setPower(int pa, float MHz, Modulation modulation)
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

  TI_write_burst_reg(CCxxx0_PATABLE, _PA_TABLE, 8);
  
  return false;
}