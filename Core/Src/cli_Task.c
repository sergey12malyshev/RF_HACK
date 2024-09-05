#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "cli_task.h"
#include "cli_driver.h"
#include "cli_queue.h"

#include "gps.h"

/*
  UART CLI 115200 Baud
  PA10 - RX
  PA9 - TX
*/

#define LOCAL_ECHO_EN  true

#define mon_strcmp(ptr, cmd) (!strcmp(ptr, cmd))

extern UART_HandleTypeDef huart1, huart6;
extern GPS_t GPS;

typedef enum
{
  NONE = 0,
  RST,
  R,
  TEST,
  ADC_T,
  GPS_C,
  INFO
}Command;

static char mon_comand[] = "Enter monitor command:\r\n\
HELP - see existing commands\r\n\
RST - restart\r\n\
R - restart using WDT\r\n\
TEST - switch test\r\n\
ADC - show ADC chanel\r\n\
GPS - show data gps\r\n\
INFO - read about project\r\n\
>";

#define SIZE_BUFF  12U
static char input_mon_buff[SIZE_BUFF] = {0};

/* queue UART */
QUEUE queue1 = {0};
uint8_t queueOutMsg[1] = {0};

/* Test API */
static Command monitorTest = NONE;

static void resetTest(void)
{
  monitorTest = NONE;
}

static void setTest(Command c)
{
  monitorTest = c;
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
  debugPrintf(">");
}

static void sendSNversion(void)
{
 //debugPrintf("Version: %d.%d.%d"CLI_NEW_LINE, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}

void debugPrintf_hello(void)
{
  debugPrintf("RF_HACK"CLI_NEW_LINE);
  sendSNversion();
  debugPrintf("Enter HELP"CLI_NEW_LINE);
  debugPrintf_symbolTerm();
}

void debugPrintf_help(void)
{
  debugPrintf(mon_comand);
}

static void debugPrintf_OK(void)
{
  debugPrintf("OK"CLI_NEW_LINE);
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
    HAL_UART_Transmit(&huart1, queueOutMsg, 1, 25); // Local echo
#endif
    if (queueOutMsg[0] == enter)
    {
      convertToUppercase();
      debugPrintf_r_n();
      if (mon_strcmp(input_mon_buff, "HELP"))
      {
        debugPrintf_help();
      }
      else if (mon_strcmp(input_mon_buff, "TEST"))
      { // enter TEST
        setTest(TEST);
        debugPrintf_OK();
      }
       else if (mon_strcmp(input_mon_buff, "ADC"))
      { // enter ADC
        debugPrintf_OK();
        setTest(ADC_T);
      }
      else if ((input_mon_buff[0] == 'R')&&(input_mon_buff[1] == 0))
      { // enter RST
        debugPrintf_OK();
        vTaskSuspendAll();
        while (1);
      }
      else if (mon_strcmp(input_mon_buff, "RST"))
      {
        debugPrintf_OK();
        HAL_NVIC_SystemReset();
      }
      else if (mon_strcmp(input_mon_buff, "GPS"))
      {
        debugPrintf_OK();
        setTest(GPS_C);
      }
      else if (mon_strcmp(input_mon_buff, "INFO"))
      {
        debugPrintf_OK();
        debugPrintf("https://github.com/sergey12malyshev/RF_HACK.git"CLI_NEW_LINE);
        debugPrintf("FreeRTOS: ");
        debugPrintf(tskKERNEL_VERSION_NUMBER);
        debugPrintf_r_n();
        debugPrintf("HAL: ");
        debugPrintf("%d", HAL_GetHalVersion());
        debugPrintf_r_n();
        debugPrintf("Data build: "__DATE__ CLI_NEW_LINE);
        debugPrintf("Time build: "__TIME__ CLI_NEW_LINE ">");
      }
      else
      {
        if (input_mon_buff[0] == 0)
        {
          debugPrintf_symbolTerm();
          uart_clear_buff();
          rec_len = 0;
          resetTest();
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
      if ((queueOutMsg[0] == backspace)||(queueOutMsg[0] == backspacePuTTY))
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
        if (rec_len < SIZE_BUFF)
        {
          if((queueOutMsg[0] > 0) &&  (queueOutMsg[0] <= 127))// ASCIi check
          {
            input_mon_buff[rec_len++] = queueOutMsg[0]; // load char do string
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
  switch (monitorTest)
  {
    case ADC_T:
      resetTest();
      break;
    case GPS_C:
      GPSTest();
      break;
    case TEST:
      debugPrintf("Test OK");
      resetTest();
      break;
    default:
      break;
  }
}

void StartCLI_Task(void *argument)
{
  const TickType_t xPeriod_ms = 125u / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  uart_clear_buff();
  uart_receve_IT();
  cli_init_queue(&queue1);
  resetTest();
  debugPrintf_hello();


  for(;;)
  {
    if(cli_deque(&queue1, (MESSAGE*)&queueOutMsg)) // чтение из очереди
    {
      monitorParser();
    }
    monitor_out_test();

    vTaskDelayUntil(&xLastWakeTime, xPeriod_ms);
  }
}




#include "time.h"

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
  resetTest();
  debugPrintf_hello();

  while (1)
  {
    PT_WAIT_UNTIL(pt, timer(&timer1, 50)); // Запускаем преобразования ~ раз в 50 мс

    if(cli_deque(&queue1, (MESSAGE*)&queueOutMsg)) // чтение из очереди
    {
      monitorParser();
    }
    monitor_out_test();
    PT_YIELD(pt);
  }

  PT_END(pt);
}