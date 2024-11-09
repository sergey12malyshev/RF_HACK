#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "main.h"
#include "cli_thread.h"
#include "cli_driver.h"
#include "cli_queue.h"
#include "runBootloader.h" 

#include "subGHz_TX_Thread.h"

#include "gps.h"
#include "time.h"
#include "adc.h"

/*
  UART CLI 115200 Baud
  PA10 - RX
  PA9 - TX
*/

#define LOCAL_ECHO_EN  true

#define MON_STRCMP(ptr, cmd) (!strcmp(ptr, cmd))

extern UART_HandleTypeDef huart1, huart6;

typedef enum
{
  NONE = 0,
  RST,
  R,
  BOOT,
  TEST,
  TX,
  ADC_T,
  GPS_C,
  INFO
}Command;

static const char mon_comand[] =\
"Enter CLI command:"CLI_NEW_LINE
"HELP"CLI_TAB2    "See existing commands"CLI_NEW_LINE
"CLS" CLI_TAB2    "Clear the screen"CLI_NEW_LINE
"RST"CLI_TAB2     "Restart"CLI_NEW_LINE
"R"CLI_TAB2       "Restart using WDT"CLI_NEW_LINE
"BOOT"CLI_TAB2    "Run bootloader"CLI_NEW_LINE
"TX [msg]"CLI_TAB "Transmitt massage"CLI_NEW_LINE
"TEST"CLI_TAB2    "Switch test"CLI_NEW_LINE
"ADC"CLI_TAB2     "Show ADC chanel"CLI_NEW_LINE
"GPS"CLI_TAB2     "Show data gps"CLI_NEW_LINE
"INFO"CLI_TAB2    "Read about project"CLI_NEW_LINE
"-----------------------------------"CLI_NEW_LINE
CLI_PROMPT_STR;

_Static_assert((sizeof(mon_comand) + 1U) < CLI_SHELL_MAX_LENGTH, "Print buffer size is smaller than help command!");


static char input_mon_buff[CLI_INPUT_BUFF_LENGTH] = {0};

/* queue UART */
static QUEUE queue1 = {0};
static char queueOutMsg = {0};

/* Test API */
static Command monitorTest = NONE;

static void cli_resetTest(void)
{
  monitorTest = NONE;
}

static void cli_setTest(const Command c)
{
  monitorTest = c;
}

static Command cli_getTest(void)
{
  return monitorTest;
}

//-------------- UART RX start ------------------
static uint8_t input_mon[1] = {0};

static void uart_clear_buff(void)
{
  memset(input_mon_buff, 0, sizeof(input_mon_buff));
}

static void uart_receve_IT(void)
{
  HAL_UART_Receive_IT(&huart1, (uint8_t *)input_mon, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) 
{
  if(huart == &huart1) 
  {
    if(HAL_UART_Receive_IT(&huart1, (uint8_t*)&input_mon, 1U) == HAL_OK)
    {
      cli_enque(&queue1,(MESSAGE*)&input_mon); // Запишем в очередь 
#if DEBUG_QUEUE
      debugPrintf("e_ l:%d e:%d b:%d\r\n", queue1.current_load, queue1.begin, queue1.end);
#endif
    }
  }

  if(huart == &huart6) 
  {
    GPS_UART_CallBack();
  }
}

//---------------------------------------

static void debugPrintf_symbolTerm(void)
{
  debugPrintf(CLI_PROMPT_STR);
}

static void sendSNversion(void)
{
 debugPrintf("Version SW: %d.%d.%d"CLI_NEW_LINE, SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_PATCH);
}

static void debugPrintf_hello(void)
{
  debugPrintf("RF_HACK project start"CLI_NEW_LINE);
  sendSNversion();
  DEBUG_PRINT(YEL_CLR"Debug Version"RST_CLR CLI_NEW_LINE);
  debugPrintf("Enter 'HELP' for list of commands...."CLI_NEW_LINE);
  checkResetSourse();
  debugPrintf_symbolTerm();
}

static void cli_clearScreen(void)
{
  CLI_RESET_CURSOR();
  CLI_DISPLAY_CLEAR();
}

static void debugPrintf_help(void)
{
  debugPrintf(mon_comand);
}

static void debugPrintf_OK(void)
{
  debugPrintf("Ok"CLI_NEW_LINE);
}

static void debugPrintf_r_n(void)
{
  debugPrintf(CLI_NEW_LINE);
}

static void debugPrintf_error(void)
{
  debugPrintf("incorrect enter"CLI_NEW_LINE);
}

static void sendBackspaceStr(void)
{
  debugPrintf(" \b");
}

static void convertToUppercase(void)
{
  static char *copy_ptr = NULL;

  copy_ptr = input_mon_buff;
  while (*copy_ptr != 0)
  {
    *copy_ptr = toupper(*copy_ptr);
    copy_ptr++;
  }
}

static void monitorParser(void)
{
  static uint8_t rec_len = 0U;
  const uint8_t enter = 13U;
  const uint8_t backspace = 0x08;
  const uint8_t backspacePuTTY = 127U;

#if LOCAL_ECHO_EN
    HAL_UART_Transmit(&huart1, (uint8_t*)&queueOutMsg, 1, 25); // Local echo
#endif
    if (queueOutMsg == enter)
    {
      convertToUppercase();
      debugPrintf_r_n();
      if (MON_STRCMP(input_mon_buff, "HELP"))
      {
        debugPrintf_help();
      }
      else if (MON_STRCMP(input_mon_buff, "CLS"))
      {
        cli_clearScreen();
      }
      else if (MON_STRCMP(input_mon_buff, "TEST"))
      { // enter TEST
        cli_setTest(TEST);
        debugPrintf_OK();
      }
      else if (memcmp(input_mon_buff, "TX", 2) == 0)
      { // enter TX [msg]
        debugPrintf_OK();

        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_12); //GDO
        NVIC_DisableIRQ(EXTI15_10_IRQn); //GDO

        extern volatile uint8_t GDO0_FLAG;
        GDO0_FLAG = 0;

        LL_mDelay(1);
        CC1101_reinit();

        char packet[9] = " ";
        uint8_t a = sizeof("TX"); //3

        for(uint8_t i = 0; i < sizeof(packet); i++)
        {
          packet[i] = input_mon_buff[a++];
        }

        transmittRF(packet, strlen(packet)); // the function is sending the data

        debugPrintf("send: %s"CLI_NEW_LINE, packet);
      }
       else if (MON_STRCMP(input_mon_buff, "ADC"))
      {
        debugPrintf_OK();
        cli_setTest(ADC_T);
      }
      else if ((input_mon_buff[0] == 'R')&&(input_mon_buff[1] == 0))
      { // enter R
        debugPrintf_OK();
        while (1);
      }
      else if (MON_STRCMP(input_mon_buff, "RST"))
      {
        debugPrintf_OK();
        HAL_NVIC_SystemReset();
      }
      else if (MON_STRCMP(input_mon_buff, "BOOT"))
      {
        debugPrintf_OK();
        runBootloader();
      }
      else if (MON_STRCMP(input_mon_buff, "GPS"))
      {
        debugPrintf_OK();
        cli_setTest(GPS_C);
      }
      else if (MON_STRCMP(input_mon_buff, "INFO"))
      {
        debugPrintf_OK();
        debugPrintf("https://github.com/sergey12malyshev/RF_HACK.git"CLI_NEW_LINE);
        debugPrintf_r_n();
        debugPrintf("HAL: ");
        debugPrintf("%d", HAL_GetHalVersion());
        debugPrintf_r_n();
        debugPrintf("Data build: "__DATE__ CLI_NEW_LINE);
        debugPrintf("Time build: "__TIME__ CLI_NEW_LINE CLI_PROMPT_STR);
      }
      else
      {
        if (input_mon_buff[0] == 0)
        {
          debugPrintf_symbolTerm();
          uart_clear_buff();
          rec_len = 0;
          cli_resetTest();
        }
        else
        {
          debugPrintf_error();
          debugPrintf_symbolTerm();
        }
      }
      uart_clear_buff();
      rec_len = 0;
    }
    else
    {
      if ((queueOutMsg == backspace)||(queueOutMsg == backspacePuTTY))
      {
        if (rec_len != 0)
        {
          input_mon_buff[rec_len - 1] = 0;
          rec_len--;
          sendBackspaceStr();
        }
      }
      else
      {
        if (rec_len < CLI_INPUT_BUFF_LENGTH)
        {
          if((queueOutMsg > 0) && (queueOutMsg <= 127)) // ASCII check
          {
            input_mon_buff[rec_len++] = queueOutMsg; // load char do string
          }
          else
          {
            debugPrintf(CLI_NEW_LINE"switch keyboard language"CLI_NEW_LINE);
          }
          
        }
        else
        {
          debugPrintf(CLI_NEW_LINE"overflow"CLI_NEW_LINE);
        }
      }
    }
}

static void GPSTest(void)
{
  debugPrintf("UTC time:%f"CLI_NEW_LINE, GPS.utc_time); 
}

static void monitor_out_test(void)
{
  switch (cli_getTest())
  {
    case ADC_T:
      debugPrintf(CLI_CLEAR_LINE"%ld"CLI_TAB, getAdcVDDA());
      debugPrintf("%d", getVoltageVDDA());
      break;
    case GPS_C:
      GPSTest();
      break;
    case TEST:
      debugPrintf("Test OK");
      cli_resetTest();
      break;

    default:
      break;
  }
}

/*
 * Протопоток StartCLI_Thread
 *
 */
PT_THREAD(StartCLI_Thread(struct pt *pt))
{
  static uint32_t timer1;

  PT_BEGIN(pt);
  
  uart_clear_buff();
  uart_receve_IT();
  cli_init_queue(&queue1);
  cli_resetTest();
  debugPrintf_hello();

  while (1)
  {
    PT_WAIT_UNTIL(pt, timer(&timer1, 50));

    if(cli_deque(&queue1, (MESSAGE*)&queueOutMsg))
    {
      monitorParser();
    }
    monitor_out_test();

    PT_YIELD(pt);
  }

  PT_END(pt);
}