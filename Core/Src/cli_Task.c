#include <stdbool.h>

#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "cli_task.h"
#include "cli_driver.h"


void StartCLI_Task(void *argument)
{
  const TickType_t xPeriod_ms = 350u / portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for(;;)
  {
    debugPrintf("Test string"CLI_NEW_LINE);

    vTaskDelayUntil(&xLastWakeTime, xPeriod_ms);
  }
}