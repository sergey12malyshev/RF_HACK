#include <stdbool.h>

#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "application_task.h"
#include "gpio.h"
#include "cli_driver.h"



void StartApplicationTask(void *argument)
{
  /* USER CODE BEGIN StartApplicationTask */
  const TickType_t xPeriod_ms = 500 / portTICK_PERIOD_MS;

  TickType_t xLastWakeTime = xTaskGetTickCount();
  /* Infinite loop */
  for(;;)
  {
    heartbeatLedToggle();

    vTaskDelayUntil(&xLastWakeTime, xPeriod_ms);
  }
  /* USER CODE END StartApplicationTask */
}