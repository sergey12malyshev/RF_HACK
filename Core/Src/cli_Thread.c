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

#include "cc1101.h"

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


/* queue UART */
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
static char input_mon_buff[CLI_INPUT_BUFF_LENGTH] = {0};

static uint8_t uart_cli_data[1] = {0};

static void uart_clear_buff(void)
{
  memset(input_mon_buff, 0, sizeof(input_mon_buff));
}

static void uart_receve_IT(void)
{
  HAL_UART_Receive_IT(&huart1, (uint8_t *)uart_cli_data, 1);
}

void cli_uart_callBack(void) 
{
  if (HAL_UART_Receive_IT(&huart1, (uint8_t*)&uart_cli_data, 1U) == HAL_OK)
  {
    cli_enque((uint8_t*)&uart_cli_data); // add it to the queue
#if DEBUG_QUEUE
    debugPrintf("e_ l:%d e:%d b:%d\r\n", queue1.current_load, queue1.begin, queue1.end);
#endif
  }
}

//---------------------------------------

static void cli_send_symbolTerm(void)
{
  debugPrintf(CLI_PROMPT_STR);
}

static void cli_send_SN_version(void)
{
  debugPrintf("Version SW: %d.%d.%d"CLI_NEW_LINE, SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_PATCH);
}

static void cli_send_hello(void)
{
  debugPrintf("RF_HACK project started!"CLI_NEW_LINE);
  cli_send_SN_version();
  DEBUG_PRINT(YEL_CLR"Debug Version"RST_CLR CLI_NEW_LINE);
  debugPrintf("Enter 'HELP' for list of commands...."CLI_NEW_LINE);
  checkResetSourse();
  cli_send_symbolTerm();
}

static void cli_clearScreen(void)
{
  CLI_RESET_CURSOR();
  CLI_DISPLAY_CLEAR();
}

static void cli_send_help(void)
{
  debugPrintf(mon_comand);
}

static void cli_send_ok(void)
{
  debugPrintf("Ok"CLI_NEW_LINE);
}

static void cli_new_line(void)
{
  debugPrintf(CLI_NEW_LINE);
}

static void cli_incorrect_enter(void)
{
  debugPrintf("incorrect enter"CLI_NEW_LINE);
}

static void cli_backspace(void)
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

static void monitorParser(uint8_t input_char)
{
  static uint8_t rec_len = 0U;
  const uint8_t enter = 13U;
  const uint8_t backspace = 0x08;
  const uint8_t backspacePuTTY = 127U;

#if LOCAL_ECHO_EN
    HAL_UART_Transmit(&huart1, (uint8_t*)&input_char, 1, 25); // Local echo
#endif
    if (input_char == enter)
    {
      convertToUppercase();
      cli_new_line();
      if (MON_STRCMP(input_mon_buff, "HELP"))
      {
        cli_send_help();
      }
      else if (MON_STRCMP(input_mon_buff, "CLS"))
      {
        cli_clearScreen();
      }
      else if (MON_STRCMP(input_mon_buff, "TEST"))
      { // enter TEST
        cli_setTest(TEST);
        cli_send_ok();
      }
      else if (memcmp(input_mon_buff, "TX", 2) == 0)
      { // enter TX [msg]
        cli_send_ok();

        CC1101_GDO0_flag_clear();

        LL_mDelay(1);
        CC1101_reinit();

        char packet[9] = " ";
        uint8_t a = sizeof("TX"); //3

        for(uint8_t i = 0; i < sizeof(packet); i++)
        {
          packet[i] = input_mon_buff[a++];
        }

        CC1101_transmittRF(packet, strlen(packet)); // the function is sending the data

        debugPrintf("send: %s"CLI_NEW_LINE, packet);
      }
       else if (MON_STRCMP(input_mon_buff, "ADC"))
      {
        cli_send_ok();
        cli_setTest(ADC_T);
      }
      else if ((input_mon_buff[0] == 'R')&&(input_mon_buff[1] == 0))
      { // enter R
        cli_send_ok();
        while (1);
      }
      else if (MON_STRCMP(input_mon_buff, "RST"))
      {
        cli_send_ok();
        HAL_NVIC_SystemReset();
      }
      else if (MON_STRCMP(input_mon_buff, "BOOT"))
      {
        cli_send_ok();
        runBootloader();
      }
      else if (MON_STRCMP(input_mon_buff, "GPS"))
      {
        cli_send_ok();
        cli_setTest(GPS_C);
      }
      else if (MON_STRCMP(input_mon_buff, "INFO"))
      {
        cli_send_ok();
        debugPrintf("https://github.com/sergey12malyshev/RF_HACK.git"CLI_NEW_LINE);
        cli_new_line();
        debugPrintf("HAL: ");
        debugPrintf("%d", HAL_GetHalVersion());
        cli_new_line();
        debugPrintf("Data build: "__DATE__ CLI_NEW_LINE);
        debugPrintf("Time build: "__TIME__ CLI_NEW_LINE CLI_PROMPT_STR);
      }
      else
      {
        if (input_mon_buff[0] == 0)
        {
          cli_send_symbolTerm();
          uart_clear_buff();
          rec_len = 0;
          cli_resetTest();
        }
        else
        {
          cli_incorrect_enter();
          cli_send_symbolTerm();
        }
      }
      uart_clear_buff();
      rec_len = 0;
    }
    else
    {
      if ((input_char == backspace)||(input_char == backspacePuTTY))
      {
        if (rec_len != 0)
        {
          input_mon_buff[rec_len - 1] = 0;
          rec_len--;
          cli_backspace();
        }
      }
      else
      {
        if (rec_len < CLI_INPUT_BUFF_LENGTH)
        {
          if((input_char > 0) && (input_char <= 127)) // ASCII check
          {
            input_mon_buff[rec_len++] = input_char; // load char do string
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
    {
      debugPrintf(CLI_CLEAR_LINE"%ld"CLI_TAB, getAdcVDDA());
      debugPrintf("%d"CLI_TAB, getVoltageVDDA());
      debugPrintf("%d", getVoltageVDDA_Av());
      break;
    }
    case GPS_C:
    {
      GPSTest();
      break;
    }
    case TEST:
    {
      debugPrintf("Test OK");
      cli_resetTest();
      break;
    }
    default:
    {
      break;
    }
  }
}

/*
 * Protothread CLI_Thread
 *
 */
PT_THREAD(CLI_Thread(struct pt *pt))
{
  static uint32_t timer1;

  PT_BEGIN(pt);
  
  uart_clear_buff();
  uart_receve_IT();
  cli_init_queue();
  cli_resetTest();
  cli_send_hello();

  while (1)
  {
    PT_WAIT_UNTIL(pt, timer(&timer1, 50));

    if (cli_deque((uint8_t*)&queueOutMsg))
    {
      monitorParser(queueOutMsg);
    }
    monitor_out_test();

    PT_YIELD(pt);
  }

  PT_END(pt);
}