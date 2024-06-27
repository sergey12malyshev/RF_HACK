#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

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

#include "gps.h"



extern LCD_Handler *lcd;     //Указатель на первый дисплей в списке
extern GPS_t GPS;

static void bootingScreen(void)
{
  char str[100] = {0};
  uint16_t y = 0;
  const uint8_t shift = 25;

  LCD_Fill(lcd, COLOR_BLACK);

  LCD_WriteString(lcd, 0, y+=shift, "LCD TEST",
            &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
 
  LCD_WriteString(lcd, 0, y+=shift, "FreeRTOS: "tskKERNEL_VERSION_NUMBER,
            &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);
  
  sprintf(str, "HAL: %lu", HAL_GetHalVersion());
  LCD_WriteString(lcd, 0, y+=shift, str,
            &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  LCD_WriteString(lcd, 0, y+=shift, "Data build: "__DATE__,
            &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  LCD_WriteString(lcd, 0, y+=shift, "Time build: "__TIME__,
            &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

}

void StartApplicationTask(void *argument)
{
  /* USER CODE BEGIN StartApplicationTask */
  const TickType_t xPeriod_ms = 450 / portTICK_PERIOD_MS;

  TickType_t xLastWakeTime = xTaskGetTickCount();
  /* Infinite loop */
  bootingScreen();
  vTaskDelayUntil(&xLastWakeTime, 1000);
  LCD_Fill(lcd, COLOR_BLACK);
  vTaskDelayUntil(&xLastWakeTime, 100);
  GPS_Init();
  for(;;)
  {
    heartbeatLedToggle();

    char str[100] = "   N = ";
    static uint16_t i;
    utoa(i++, &str[7], 10);
    strcat(str, " count    ");

    __disable_irq();
    LCD_WriteString(lcd, 0, 0, str, &Font_8x13, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
    __enable_irq();

    HAL_Delay(450);
  }
}
  /*
 * Протопоток StartApplication_Thread
 *
 * 
 */
PT_THREAD(StartApplication_Thread(struct pt *pt))
{
  static uint32_t timeCount;

  PT_BEGIN(pt);
  
  bootingScreen();

  timeCount = HAL_GetTick();
  PT_WAIT_UNTIL(pt, (HAL_GetTick() - timeCount) > 500U);
  timeCount = HAL_GetTick();

  LCD_Fill(lcd, COLOR_BLACK);
  GPS_Init();

  while (1)
  {
    PT_WAIT_UNTIL(pt, (HAL_GetTick() - timeCount) > 450U); // Запускаем преобразования ~ раз в 50 мс
    timeCount = HAL_GetTick();	
    

    heartbeatLedToggle();

#if 0
    char str[100] = "   N = ";
    static uint16_t i;
    utoa(i++, &str[7], 10);
    strcat(str, " count    ");
#endif
    char str[100] = "";
      sprintf(str, "Utc time: %.2f", GPS.utc_time);
   // __disable_irq();
    LCD_WriteString(lcd, 0, 0, str, &Font_8x13, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
   // __enable_irq();
      sprintf(str, "longitude: %.4f", GPS.dec_longitude);
    LCD_WriteString(lcd, 0, 15, str, &Font_8x13, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);

      sprintf(str, "latitude: %.4f", GPS.dec_latitude);
    LCD_WriteString(lcd, 0, 30, str, &Font_8x13, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);

      sprintf(str, "altitude_ft: %.4f", GPS.msl_altitude);
    LCD_WriteString(lcd, 0, 45, str, &Font_8x13, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
    

    PT_YIELD(pt);
  }

  PT_END(pt);
}
