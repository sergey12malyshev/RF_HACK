#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "main.h"
#include "button_Thread.h"
#include "application_Thread.h"
#include "gpio.h"
#include "cli_driver.h"

#include "display.h"
#include "displayInit.h"
#include "ili9341.h"
#include "xpt2046.h"
#include "calibrate_touch.h"
#include "demo.h"

#include "time.h"

#define BUTTON_H 6

#define BUTTON_TX_X 184
#define BUTTON_TX_Y 265

#define BUTTON_SCAN_X 184
#define BUTTON_SCAN_Y 205

#define BUTTON_JAMM_X 184
#define BUTTON_JAMM_Y 145

#define BUTTON_BOOT_X 184
#define BUTTON_BOOT_Y 85

#define BUTTON_GPS_X 184
#define BUTTON_GPS_Y 25


static bool TxButton, scanButton, jammButton, bootButton, gpsButton;

bool getTxButtonState(void)
{
  return TxButton;
}

bool getScanButtonState(void)
{
  return scanButton;
}

bool getjammButtonState(void)
{
  return jammButton;
}

bool getBootButtonState(void)
{
  return bootButton;
}

bool getGpsButtonState(void)
{
  return gpsButton;
}

static void allButtonClearState(void)
{
  TxButton = scanButton = jammButton = bootButton = gpsButton = 0;
}

void buttonTx_logo(uint32_t color)
{
  int x = BUTTON_TX_X, y = BUTTON_TX_Y;
  int hw = LCD_GetHeight(lcd) / BUTTON_H; // Сторона квадрата с цветом пера

  LCD_DrawRectangle(lcd, x, y, x + hw - 2, y + hw - 2, COLOR_WHITE);     // Черный контур вокруг текущего цвета
  LCD_DrawFilledRectangle(lcd, x + 2, y + 2, x + hw - 4, y + hw - 4, color); // Квадрат, залитый текущим цветом
  // Кнопка "TX" в квадрате с белым цветом
  LCD_WriteString(lcd, x + hw / 2 - 10, y + hw / 2 - 5, "TX", &Font_8x13, COLOR_BLACK, COLOR_BLACK, LCD_SYMBOL_PRINT_PSETBYPSET);
}

void buttonScan_logo(uint32_t color)
{
  int x = BUTTON_SCAN_X, y = BUTTON_SCAN_Y;
  int hw = LCD_GetHeight(lcd) / BUTTON_H; // Сторона квадрата с цветом пера

  LCD_DrawRectangle(lcd, x, y, x + hw - 2, y + hw - 2, COLOR_WHITE);     // Черный контур вокруг текущего цвета
  LCD_DrawFilledRectangle(lcd, x + 2, y + 2, x + hw - 4, y + hw - 4, color); // Квадрат, залитый текущим цветом
  // Кнопка "SCAN" в квадрате с белым цветом
  LCD_WriteString(lcd, x + hw / 2 - 15, y + hw / 2 - 5, "SCAN", &Font_8x13, COLOR_BLACK, COLOR_BLACK, LCD_SYMBOL_PRINT_PSETBYPSET);
}

void buttonJamm_logo(uint32_t color)
{
  int x = BUTTON_JAMM_X, y = BUTTON_JAMM_Y;
  int hw = LCD_GetHeight(lcd) / BUTTON_H; // Сторона квадрата с цветом пера

  LCD_DrawRectangle(lcd, x, y, x + hw - 2, y + hw - 2, COLOR_WHITE);     // Черный контур вокруг текущего цвета
  LCD_DrawFilledRectangle(lcd, x + 2, y + 2, x + hw - 4, y + hw - 4, color); // Квадрат, залитый текущим цветом
  // Кнопка "JAMM" в квадрате с белым цветом
  LCD_WriteString(lcd, x + hw / 2 - 15, y + hw / 2 - 5, "JAMM", &Font_8x13, COLOR_BLACK, COLOR_BLACK, LCD_SYMBOL_PRINT_PSETBYPSET);
}

void buttonBoot_logo(uint32_t color)
{
  int x = BUTTON_BOOT_X, y = BUTTON_BOOT_Y;
  int hw = LCD_GetHeight(lcd) / BUTTON_H; // Сторона квадрата с цветом пера

  LCD_DrawRectangle(lcd, x, y, x + hw - 2, y + hw - 2, COLOR_WHITE);     // Черный контур вокруг текущего цвета
  LCD_DrawFilledRectangle(lcd, x + 2, y + 2, x + hw - 4, y + hw - 4, color); // Квадрат, залитый текущим цветом
  // Кнопка "BOOT" в квадрате с белым цветом
  LCD_WriteString(lcd, x + hw / 2 - 15, y + hw / 2 - 5, "BOOT", &Font_8x13, COLOR_BLACK, COLOR_BLACK, LCD_SYMBOL_PRINT_PSETBYPSET);
}

void buttonGPS_logo(uint32_t color)
{
  int x = BUTTON_GPS_X, y = BUTTON_GPS_Y;
  int hw = LCD_GetHeight(lcd) / BUTTON_H; // Сторона квадрата с цветом пера

  LCD_DrawRectangle(lcd, x, y, x + hw - 2, y + hw - 2, COLOR_WHITE);     // Черный контур вокруг текущего цвета
  LCD_DrawFilledRectangle(lcd, x + 2, y + 2, x + hw - 4, y + hw - 4, color); // Квадрат, залитый текущим цветом
  // Кнопка "GPS" в квадрате с белым цветом
  LCD_WriteString(lcd, x + hw / 2 - 15, y + hw / 2 - 5, "GPS", &Font_8x13, COLOR_BLACK, COLOR_BLACK, LCD_SYMBOL_PRINT_PSETBYPSET);
}

void button_logoClear(void)
{
  buttonTx_logo(COLOR_WHITE);
  buttonScan_logo(COLOR_WHITE);
  buttonJamm_logo(COLOR_WHITE);
  buttonBoot_logo(COLOR_WHITE);
  buttonGPS_logo(COLOR_WHITE);
}


static bool buttonHandler(XPT2046_Handler *t)
{
  int x = 0, y = 0;
  tPoint point_d;

  static bool noClick;

  (void)XPT2046_GetTouch(t); // Опрос тачскрина (на всякий случай, вдруг запрещено опрашивать тачскрин в прерывании)

  if (t->click) // Есть касание тачскрина
  {
    XPT2046_ConvertPoint(&point_d, &t->point, &t->coef); // Преобразуем координаты тачскрина в дисплейные
    x = point_d.x;                     // Получаем значения дисплейных
    y = point_d.y;                     // координат
    if (x < 0)
      x = 0; // Проверяем координаты
    if (y < 0)
      y = 0; // на
    if (x >= lcd->Width)
      x = lcd->Width - 1; // выход за допустимые
    if (y >= lcd->Height)
      y = lcd->Height - 1; // границы

    // debugPrintf("%d, %d"CLI_NEW_LINE, x, y);
    int hw = LCD_GetHeight(lcd) / BUTTON_H; // Сторона квадрата с цветом пера
    


    if (x >= BUTTON_TX_X && x < (hw + BUTTON_TX_X) && y >= BUTTON_TX_Y && y < (hw + BUTTON_TX_Y))
    {
      if(!TxButton)
      {
        allButtonClearState();
        button_logoClear();
        buttonTx_logo(COLOR_RED);
        TxButton = true;
      }
      else
      {
        if(noClick)
        {
        buttonTx_logo(COLOR_WHITE);
        TxButton = false;
        }
      }
    }

    if (x >= BUTTON_SCAN_X && x < (hw + BUTTON_SCAN_X) && y >= BUTTON_SCAN_Y && y < (hw + BUTTON_SCAN_Y))
    {
      if(!scanButton)
      {
        allButtonClearState();
        button_logoClear();
        buttonScan_logo(COLOR_CYAN);
        scanButton = true;
      }
      else
      {
        if(noClick)
        {
        buttonScan_logo(COLOR_WHITE);
        scanButton = false;
        }
      }
    }

    if (x >= BUTTON_JAMM_X && x < (hw + BUTTON_JAMM_X) && y >= BUTTON_JAMM_Y && y < (hw + BUTTON_JAMM_Y))
    {
      if(!jammButton)
      {
        allButtonClearState();
        button_logoClear();
        buttonJamm_logo(COLOR_RED);
        jammButton = true;
      }
      else
      {
        if(noClick)
        {
        buttonJamm_logo(COLOR_WHITE);
        jammButton = false;
        }
      }
    }

    if (x >= BUTTON_BOOT_X && x < (hw + BUTTON_BOOT_X) && y >= BUTTON_BOOT_Y && y < (hw + BUTTON_BOOT_Y))
    {
      if(!bootButton)
      {

        allButtonClearState();
        button_logoClear();
        buttonBoot_logo(COLOR_RED);
        bootButton = true;
      }
      else
      {
        if(noClick)
        {
        buttonBoot_logo(COLOR_WHITE);
        bootButton = false;
        }
      }
    }

    if (x >= BUTTON_GPS_X && x < (hw + BUTTON_GPS_X) && y >= BUTTON_GPS_Y && y < (hw + BUTTON_GPS_Y))
    {
      if(!gpsButton)
      {

        allButtonClearState();
        button_logoClear();
        buttonGPS_logo(COLOR_YELLOW);
        gpsButton = true;
      }
      else
      {
        if(noClick)
        {
        buttonGPS_logo(COLOR_WHITE);
        gpsButton = false;
        }
      }
    }

    noClick = false;
  }
  else
  {
    noClick = true;
  }

  return false;
}

/*
 * Протопоток Button_Thread
 *
 *
 */

PT_THREAD(Button_Thread(struct pt *pt))
{
  static uint32_t timer1;

  PT_BEGIN(pt);

  allButtonClearState();

  PT_DELAY_MS(pt, &timer1, screen_booting_get_time());

  button_logoClear();

  while (1)
  {
    PT_WAIT_UNTIL(pt, timer(&timer1, 150));
    buttonHandler(&touch1);

    PT_YIELD(pt);
  }

  PT_END(pt);
}
