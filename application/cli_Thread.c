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

#include "power.h"

/*
  UART CLI 115200 Baud
  PA10 - RX
  PA9 - TX
*/

#define LOCAL_ECHO_EN  true

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
} Command;

static const char mon_comand[] =\
"Enter CLI command:"CLI_NEW_LINE
"HELP"CLI_TAB2    "See existing commands"CLI_NEW_LINE
"CLS" CLI_TAB2    "Clear the screen"CLI_NEW_LINE
"RST"CLI_TAB2     "Restart"CLI_NEW_LINE
"R"CLI_TAB2       "Restart using WDT"CLI_NEW_LINE
"BOOT"CLI_TAB2    "Run bootloader"CLI_NEW_LINE
"TX [msg]"CLI_TAB "Transmitt massage"CLI_NEW_LINE
"TEST"CLI_TAB2    "Switch test"CLI_NEW_LINE
"ADC"CLI_TAB2     "Show VDDA chanel: adc, mV, av mV"CLI_NEW_LINE
"GPS"CLI_TAB2     "Show data gps"CLI_NEW_LINE
"INFO"CLI_TAB2    "Read about project"CLI_NEW_LINE
"-----------------------------------"CLI_NEW_LINE
CLI_PROMPT_STR;

_Static_assert((sizeof(mon_comand) + 1U) < CLI_SHELL_MAX_LENGTH, "Print buffer size is smaller than help command!");

/* UART queue */
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

/* Input buffer and UART RX */
static char input_mon_buff[CLI_INPUT_BUFF_LENGTH] = {0};
static uint8_t uart_cli_data[1] = {0};
static uint8_t rec_len = 0U;

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
  }
}

/* Helper output functions */
static void cli_send_symbolTerm(void)
{
  debugPrintf(CLI_PROMPT_STR);
}

static void cli_send_version(void)
{
  debugPrintf("Version SW: %d.%d.%d"CLI_NEW_LINE, SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_PATCH);
  debugPrintf("Version HW: %d.%d.%d"CLI_NEW_LINE, 0, 1, 0);
}

static void cli_send_hello(void)
{
  debugPrintf("+------------------------------------------------+"CLI_NEW_LINE);
  debugPrintf("|                                                |"CLI_NEW_LINE);
  debugPrintf("|   #####  #####    #   #    ##     ###   #   #  |"CLI_NEW_LINE);
  debugPrintf("|   #   #  #        #   #   #  #   #      #  #   |"CLI_NEW_LINE);
  debugPrintf("|   #####  #####    #####   ####   #      ###    |"CLI_NEW_LINE);
  debugPrintf("|   #  #   #        #   #  #    #  #      #  #   |"CLI_NEW_LINE);
  debugPrintf("|   #   #  #        #   #  #    #   ###   #   #  |"CLI_NEW_LINE);
  debugPrintf("|                                                |"CLI_NEW_LINE);
  debugPrintf("|        Testing wireless transmissions          |"CLI_NEW_LINE);            
  debugPrintf("|              and radio interface               |"CLI_NEW_LINE);
  debugPrintf("+------------------------------------------------+"CLI_NEW_LINE);

  cli_send_version();
  DEBUG_PRINT(YEL_CLR"Debug Version"RST_CLR CLI_NEW_LINE);
  debugPrintf("Enter 'HELP' for list of commands...."CLI_NEW_LINE);
  checkResetSourse();
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
  char *copy_ptr = input_mon_buff;
  while (*copy_ptr != 0)
  {
    *copy_ptr = toupper(*copy_ptr);
    copy_ptr++;
  }
}

/* ---------- Command handlers (command table) ---------- */

static void cmd_help(char *arg)
{
  (void)arg;
  cli_send_help();
}

static void cmd_cls(char *arg)
{
  (void)arg;
  cli_clearScreen();
}

static void cmd_test(char *arg)
{
  (void)arg;
  cli_setTest(TEST);
  cli_send_ok();
}

static void cmd_tx(char *arg)
{
  /* arg points to the message after "TX " */
  if (arg == NULL || *arg == '\0')
  {
    debugPrintf("Error: no message" CLI_NEW_LINE);
    return;
  }
  cli_send_ok();
  LL_mDelay(1);
  CC1101_reinit();
  CC1101_transmitt_packet((char*)arg, (uint8_t)strlen(arg));
  debugPrintf("send: %s" CLI_NEW_LINE, arg);
}

static void cmd_adc(char *arg)
{
  (void)arg;
  cli_send_ok();
  cli_setTest(ADC_T);
}

static void cmd_r(char *arg)
{
  (void)arg;
  cli_send_ok();
  power_wdtReset();
}

static void cmd_rst(char *arg)
{
  (void)arg;
  cli_send_ok();
  power_systemReset();
}

static void cmd_boot(char *arg)
{
  (void)arg;
  cli_send_ok();
  runBootloader();
}

static void cmd_gps(char *arg)
{
  (void)arg;
  cli_send_ok();
  cli_setTest(GPS_C);
}

static void cmd_info(char *arg)
{
  (void)arg;
  cli_send_ok();
  debugPrintf("https://github.com/sergey12malyshev/RF_HACK.git"CLI_NEW_LINE);
  cli_new_line();
  debugPrintf("HAL: %d", HAL_GetHalVersion());
  cli_new_line();
  debugPrintf("Data build: "__DATE__ CLI_NEW_LINE);
  debugPrintf("Time build: "__TIME__ CLI_NEW_LINE);
  cli_send_symbolTerm();
}

/* Command table */
typedef struct
{
  const char *name;
  void (*handler)(char *arg);
} cli_command_t;

static const cli_command_t commands[] =
{
  {"HELP", cmd_help},
  {"CLS",  cmd_cls},
  {"TEST", cmd_test},
  {"TX",   cmd_tx},
  {"ADC",  cmd_adc},
  {"R",    cmd_r},
  {"RST",  cmd_rst},
  {"BOOT", cmd_boot},
  {"GPS",  cmd_gps},
  {"INFO", cmd_info},
  {NULL, NULL}
};

/* Find and execute command by name */
static bool execute_command(char *cmd_line)
{
  char *cmd_name = cmd_line;
  char *arg = cmd_line;

  /* Split command name and arguments (first space) */
  while (*arg && *arg != ' ') arg++;
  if (*arg == ' ')
  {
    *arg = '\0';        /* replace space with terminator */
    arg++;              /* now arg points to argument start */
    while (*arg == ' ') arg++; /* skip leading spaces */
  }
  else
  {
    arg = NULL;         /* no arguments */
  }

  /* Lookup in command table */
  for (const cli_command_t *cmd = commands; cmd->name != NULL; cmd++)
  {
    if (!strcmp(cmd_name, cmd->name))
    {
      cmd->handler(arg);
      return true;
    }
  }
  return false;
}

/* Main parser (replaces long if-else chain) */
static void monitorParser(uint8_t input_char)
{
  const uint8_t enter = 13U;
  const uint8_t backspace = 0x08;
  const uint8_t backspacePuTTY = 127U;

#if LOCAL_ECHO_EN
  HAL_UART_Transmit(&huart1, (uint8_t*)&input_char, 1, 25);
#endif

  if (input_char == enter)
  {
    convertToUppercase();
    cli_new_line();

    if (rec_len == 0)
    {
      cli_send_symbolTerm();
      cli_resetTest();
    }
    else if (execute_command(input_mon_buff))
    {
      /* Command executed successfully */
    }
    else
    {
      cli_incorrect_enter();
      cli_send_symbolTerm();
    }

    uart_clear_buff();
    rec_len = 0;
  }
  else
  {
    if ((input_char == backspace) || (input_char == backspacePuTTY))
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
        if ((input_char > 0) && (input_char <= 127))
        {
          input_mon_buff[rec_len++] = input_char;
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

/* Asynchronous test output (unchanged) */
static void test_adc(void)
{
  debugPrintf(CLI_CLEAR_LINE"%ld"CLI_TAB, getAdcVDDA());
  debugPrintf("%d"CLI_TAB, getVoltageVDDA());
  debugPrintf("%d", getVoltageVDDA_Av());
}

static void test_gps(void)
{
  debugPrintf("UTC time: %f"CLI_NEW_LINE, GPS.utc_time);
}

static void test_test(void)
{
  debugPrintf("Test OK");
  cli_resetTest();
}

static void monitor_out_test(void)
{
  switch (cli_getTest())
  {
    case ADC_T:
    {
      test_adc();
      break;
    }
    case GPS_C:
    {
      test_gps();
      break;
    }
    case TEST:
    {
      test_test();
      break;
    }

    default:
      break;

  }
}

void cli_init(void)
{
  uart_clear_buff();
  uart_receve_IT();
  cli_init_queue();
  cli_resetTest();
  cli_send_hello();
}
/*
 * Protothread CLI_Thread
 *
 */
PT_THREAD(CLI_Thread(struct pt *pt))
{
  static uint32_t timer1;

  PT_BEGIN(pt);
  
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