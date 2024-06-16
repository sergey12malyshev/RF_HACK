#include <stdbool.h>

#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "application_task.h"
#include "gpio.h"
#include "cli_driver.h"

#include "display.h"
#include "ili9341.h"
#include "xpt2046.h"
#include "calibrate_touch.h"
#include "demo.h"


extern LCD_Handler *lcd;     //Указатель на первый дисплей в списке

void StartApplicationTask(void *argument)
{
  /* USER CODE BEGIN StartApplicationTask */
  const TickType_t xPeriod_ms = 250 / portTICK_PERIOD_MS;

  TickType_t xLastWakeTime = xTaskGetTickCount();
  /* Infinite loop */
  for(;;)
  {
    heartbeatLedToggle();

    char str[100] = "   N = ";
    static uint16_t i;
    utoa(i++, &str[7], 10);
    strcat(str, " count    ");
    LCD_WriteString(lcd, 0, 0, str, &Font_8x13, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);

    vTaskDelayUntil(&xLastWakeTime, xPeriod_ms);
  }
  /* USER CODE END StartApplicationTask */
}