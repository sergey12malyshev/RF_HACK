#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "main.h"
#include "buttonDisplay.h"
#include "application_task.h"
#include "gpio.h"
#include "cli_driver.h"

#include "display.h"
#include "ili9341.h"
#include "xpt2046.h"
#include "calibrate_touch.h"
#include "demo.h"

#define BUTTON_TX_X 5
#define BUTTON_TX_Y 260
#define BUTTON_H 6

extern LCD_Handler *lcd;
extern XPT2046_Handler touch1;

static bool TxButton = false;

bool getTxButtonState(void)
{
  return TxButton;
}

void buttonTx_logo(uint32_t color)
{
  int x = BUTTON_TX_X, y = BUTTON_TX_Y;
	int hw = LCD_GetHeight(lcd) / BUTTON_H; //Сторона квадрата с цветом пера

	LCD_DrawRectangle(lcd, x, y, x + hw - 2, y + hw - 2, COLOR_WHITE); //Черный контур вокруг текущего цвета
	LCD_DrawFilledRectangle(lcd, x + 2, y + 2, x + hw - 4, y + hw - 4, color); //Квадрат, залитый текущим цветом
	//Кнопка "Exit" в квадрате с белым цветом
	LCD_WriteString(lcd, x + hw/2 - 10, y + hw/2 - 5, "TX", &Font_8x13, COLOR_BLACK, COLOR_BLACK, LCD_SYMBOL_PRINT_PSETBYPSET);
}


static bool TX_buttonHandler(XPT2046_Handler *t)
{
  int x = 0, y = 0;
  	tPoint point_d;
  
		(void)XPT2046_GetTouch(t); //Опрос тачскрина (на всякий случай, вдруг запрещено опрашивать тачскрин в прерывании)
		
    if (t->click)       						 //Есть касание тачскрина
    {			
			XPT2046_ConvertPoint(&point_d, &t->point, &t->coef); //Преобразуем координаты тачскрина в дисплейные
			x = point_d.x; 			//Получаем значения дисплейных
			y = point_d.y; 			//координат
			if (x < 0) x = 0; 							//Проверяем координаты
			if (y < 0) y = 0;							//на
			if (x >= lcd->Width) x = lcd->Width - 1;	//выход за допустимые
			if (y >= lcd->Height) y = lcd->Height - 1;	//границы

      //debugPrintf("%d, %d"CLI_NEW_LINE, x, y);	
      int hw = LCD_GetHeight(lcd) / BUTTON_H; //Сторона квадрата с цветом пера

			if (x >= BUTTON_TX_X && x < (hw + BUTTON_TX_X) && y >= BUTTON_TX_Y && y < (hw + BUTTON_TX_Y)) 
      {
         //debugPrintf("@@@@@");
				return true;
			}
	  }

  return false;
}

/*
 * Протопоток Display_Thread
 *
 * 
 */

PT_THREAD(Display_Thread(struct pt *pt))
{
  static uint32_t timer1;

  static bool TxButton_tmp;


  PT_BEGIN(pt);
  
  while (1)
  {

  TxButton = TX_buttonHandler(&touch1);
  
  if (TxButton != TxButton_tmp)
  {
    TxButton_tmp = TxButton;
    debugPrintf("%d"CLI_NEW_LINE, TxButton_tmp);

    if(TxButton)
    {
 		  buttonTx_logo(COLOR_RED);
    }
    else
    {
      buttonTx_logo(COLOR_WHITE);
    }
		LL_mDelay(10);
  }

    PT_YIELD(pt);
  }

  PT_END(pt);
}
