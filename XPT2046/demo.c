/*
 *  Copyright (C) 2019, VadRov, all right reserved.
 *
 *  Демки работы с контроллером XPT2046
 *
 *  Допускается свободное распространение без целей коммерческого использования.
 *  При коммерческом использовании необходимо согласование с автором.
 *  Распространятся по типу "как есть", то есть использование осуществляете на свой страх и риск.
 *  Автор не предоставляет никаких гарантий.
 *
 *  https://www.youtube.com/@VadRov
 *  https://dzen.ru/vadrov
 *  https://vk.com/vadrov
 *  https://t.me/vadrov_channel
 *
 */

#include "demo.h"
#include <stdlib.h>
#include <string.h>

extern uint32_t millis;

/*
 * Демка рисования на дисплее с помощью тачскрина (возможность смены цвета пера)
 */
void Draw_TouchPenDemo(XPT2046_Handler *t, LCD_Handler *lcd)
{
	t->fl_interrupt = 1;//Разрешаем опрос тачскрина в прерывании
	int x = 0, y = 0, x_old = -1, y_old = -1;
	tPoint point_d;
	//Массив с цветами "пера"
	uint32_t color_dat[] = {COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, COLOR_CYAN, COLOR_MAGENTA, COLOR_WHITE};
	int hw = LCD_GetHeight(lcd) / 8; //Сторона квадрата с цветом пера
	uint32_t color = COLOR_BLACK; //Начальный цвет "пера"
	LCD_Fill(lcd, COLOR_WHITE); //Закрашиваем экран белым цветом
	for (int i = 0; i < 8; i++)	{ //Выводим 8 квадратов с вариантами цветов "пера"
		LCD_DrawRectangle(lcd, x, y, x + hw - 2, y + hw - 2, COLOR_BLACK); //Черный контур вокруг текущего цвета
		LCD_DrawFilledRectangle(lcd, x + 2, y + 2, x + hw - 4, y + hw - 4, color_dat[i]); //Квадрат, залитый текущим цветом
		y += hw; //Смещение по оси y в точку вывода следующего цвета
	}
	//Кнопка "Exit" в квадрате с белым цветом
	LCD_WriteString(lcd, x + 2, y - hw + 2, "Exit", &Font_8x13, COLOR_BLACK, COLOR_WHITE, LCD_SYMBOL_PRINT_PSETBYPSET);
	char str[100] = "   t = "; //Буфер для формирования строки с информацией
	while (1) { //В цикле рисуем до тех пор, пока не будет клика по квадрату с надписью "Exit"
		(void)XPT2046_GetTouch(t); //Опрос тачскрина (на всякий случай, вдруг запрещено опрашивать тачскрин в прерывании)
		if (t->click) {											 //Есть касание тачскрина
			XPT2046_ConvertPoint(&point_d, &t->point, &t->coef); //Преобразуем координаты тачскрина в дисплейные
			x = point_d.x; 			//Получаем значения дисплейных
			y = point_d.y; 			//координат
			if (x < 0) x = 0; 							//Проверяем координаты
			if (y < 0) y = 0;							//на
			if (x >= lcd->Width) x = lcd->Width - 1;	//выход за допустимые
			if (y >= lcd->Height) y = lcd->Height - 1;	//границы
			//Если касание тачскрина произошло в зоне выбора цветов "пера", то назначаем цвет "пера"
			//в зависимости от точки касания
			if (x >= 0 && x < hw && y >= 0 && y < hw * 8) {
				color = color_dat[y / hw]; //Новый цвет пера
				if (y / hw == 7) { //Сообщение "Ты настоящий художник!" и выход из демки, если кликнули на "Exit"
					LCD_WriteString(lcd, 50, 150, "You are a real artist!", &Font_8x13, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
					LL_mDelay(1000);
					return;
				}
			}
			//Для того, чтобы наше "художество" не выглядело "рваным" будем соединять линией
			//предыдущую точку с текущей
			if (x_old == -1 || y_old == -1) { 	//Если текущие координаты первые,
				LCD_DrawPixel(lcd, x, y, color);//то рисуем точку
			}
			else { //Если текущие координаты не первые, то рисуем линию
				LCD_DrawLine(lcd, x_old, y_old, x, y, color); //через предыдущие/"старые" и текущие координаты
			}
			x_old = x; //Переопределяем предыдущие/"старые"
			y_old = y; //координаты
		}
		else {
			//Формируем и выводим на дисплей строку с информацией о длительности
			//последнего касания тачскрина (в миллисекундах)
			utoa(t->last_click_time * t->timer_period, &str[7], 10);
			strcat(str, "ms    ");
			LCD_WriteString(lcd, hw, 0, str, &Font_8x13, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
			x_old = y_old = -1; //Инициализируем значения "старых" координат x и y
		}
		LL_mDelay(1); //Задержка 1 мс (для нормальной работы тачскрина в прерывании)
	}
}

static int random_value(int max)
{
	return (max * (rand() & 65535)) / 65536;
}

#define SMA_per	150 //Период скользящей средней температуры

/*
 * Демка рисует примитивы случайного размера в случайной области дисплея, отображает
 * температуру и позволяет перемещать окружность белого цвета по дисплею.
 * При удержании касания дисплей окрашивается случайным цветом
 */
void RoadCircleDemo(XPT2046_Handler *t, LCD_Handler *lcd)
{
	//Демка с геометрическими фигурами, появляющимися в случайных координатах, и закрашенным кругом, управляемым с тачскрина
	int w = lcd->Width;
	int h = lcd->Height;
	uint32_t fig = 0;
	char str[40] = " Primitives/c = ";
	char st[100] = " Temp = ";
	int temp_buf[SMA_per];
	int t_count = 1;
	int x0, y0;
	uint32_t tick = millis, tick_t;
	tPoint p;
	t->fl_interrupt = 0; //Запрещаем опрашивать тачскрин в прерывании
	while(1) {
		x0 = random_value(w);
		y0 = random_value(h);
		LCD_DrawFilledTriangle(lcd, x0 - random_value(70), y0 - random_value(70), x0 + random_value(70), y0 + random_value(70), x0 - random_value(70), y0 - random_value(70), (uint32_t)rand());
		LCD_DrawFilledCircle(lcd, random_value(w), random_value(h), random_value(30), (uint32_t)rand());
		fig += 2;
		(void)XPT2046_GetTouch(t); //Опрос тачскрина
		if (t->status == XPT2046_STATUS_CLICK) { //Есть касание
			XPT2046_ConvertPoint(&p, &t->point, &t->coef);
			LCD_DrawFilledCircle(lcd, p.x, p.y, 20, COLOR_WHITE);
		}
		else if (t->status == XPT2046_STATUS_HOLD) { //Есть удержание
			LCD_Fill(lcd, (uint32_t)rand());
		}
		if (millis - tick >= 1000) {
			utoa(fig, &str[16], 10);
			strcat(str, "   ");
			fig = 0;
			tick = millis;
		}
		tick_t = millis;
		LCD_WriteString(lcd, 0, 0, str, &Font_8x13, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
		//-------------------------- Скользящее среднее температуры --------------------------------
		int temp = XPT2046_GetTemp(t);
		if (temp != -100000) {
			if (t_count < SMA_per) {
				temp_buf[t_count++] = temp;
			}
			else {
				memmove(&temp_buf[0], &temp_buf[1], sizeof(int) * (SMA_per - 1));
				temp_buf[SMA_per - 1] = temp;
				temp = 0;
				for (int i = 0; i < SMA_per; i++) {
					temp += temp_buf[i];
				}
				temp /= SMA_per;
			}
			utoa(temp, &st[8], 10);
			strcat(st, "   ");
			LCD_WriteString(lcd, 0, 13, st, &Font_8x13, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
		}
		//-----------------------------------------------------------------------------------------------
		millis = tick_t;
	}
}
