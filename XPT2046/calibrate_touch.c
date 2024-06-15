/*
 *  Copyright (C) 2019, VadRov, all right reserved.
 *
 *  Калибровка тачскрина по 5 точкам
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

#include "calibrate_touch.h"

/*
 * Расчет коэффициентов для преобразования координат тачскрина в дисплейные координаты
 */
static void CoefCalc(tPoint *p_d, tPoint *p_t, tCoef *coef, uint8_t all_points)
{
	uint64_t a = 0, b = 0, c = 0, d = 0, e = 0, X1 = 0, X2 = 0, X3 = 0, Y1 = 0, Y2 = 0, Y3 = 0;
	for(uint8_t i = 0; i < all_points; i++)	{
		a += p_t[i].x * p_t[i].x;
		b += p_t[i].y * p_t[i].y;
		c += p_t[i].x * p_t[i].y;
		d += p_t[i].x;
		e += p_t[i].y;
		X1 += p_t[i].x * p_d[i].x;
		X2 += p_t[i].y * p_d[i].x;
		X3 += p_d[i].x;
		Y1 += p_t[i].x * p_d[i].y;
		Y2 += p_t[i].y * p_d[i].y;
		Y3 += p_d[i].y;
	}
	coef->D = all_points * (a * b - c * c) + 2 * c *  d * e - a * e * e - b * d * d;
	coef->Dx1 = all_points * (X1 * b - X2 * c) + e * (X2 * d - X1 * e) + X3 * (c * e - b * d);
	coef->Dx2 = all_points * (X2 * a - X1 * c) + d * (X1 * e - X2 * d) + X3 * (c * d - a * e);
	coef->Dx3 = X3 * (a * b - c * c) + X1 * (c * e - b * d) + X2 * (c * d - a * e);
	coef->Dy1 = all_points * (Y1 * b - Y2 * c) + e * (Y2 * d - Y1 * e) + Y3 * (c * e - b * d);
	coef->Dy2 = all_points * (Y2 * a - Y1 * c) + d * (Y1 * e - Y2 * d) + Y3 * (c * d - a * e);
	coef->Dy3 = Y3 * (a * b - c * c) + Y1 * (c * e - b * d) + Y2 * (c * d -a * e);
}

/*
 * Преобразование координат тачскрина в дисплейные/экранные координаты:
 * - в переменной p_t (тип tPoint) принимает координаты тачскрина;
 * - в переменной coef (тип tCoef) принимает коэффициенты преобразования;
 * - в переменной p_d (тип tPoint) возвращает дисплейные координаты.
 */
void XPT2046_ConvertPoint(tPoint *p_d, volatile tPoint *p_t, tCoef *coef)
{
	p_d->x = (int)((p_t->x * coef->Dx1 + p_t->y * coef->Dx2 + coef->Dx3) / coef->D);
	p_d->y = (int)((p_t->x * coef->Dy1 + p_t->y * coef->Dy2 + coef->Dy3) / coef->D);
}

/* Ожидает клика по тачскрину (опрос в прерывании разрешен!) */
static void read_touch(XPT2046_Handler *t)
{
	while (1) { //Цикл опроса тачскрина.
		while (!t->click) { //Ожидание касания тачскрина.
			__NOP();
		}
		while (t->click) { //Пока есть касание тачскрина.
			__NOP();
		}
		if (t->last_click_time > 3) { //Ограничиваем минимальную длительность касания, для фильтрации случайных касаний.
			break; //Прерываем цикл опроса тачскрина.
		}
		__NOP();
	}
}

static void draw_calibrate_point (LCD_Handler *lcd, uint16_t x, uint16_t y, uint16_t r)
{
	LCD_DrawCircle(lcd, x, y, r, COLOR_BLACK);
	LCD_DrawLine(lcd, x, y - 5, x, y + 5, COLOR_RED);
	LCD_DrawLine(lcd, x - 5, y, x + 5, y, COLOR_RED);
}

/* Калибровка тачскрина */
void XPT2046_CalibrateTouch(XPT2046_Handler *t, LCD_Handler *lcd)
{
	//Таблица с дисплейными координатами 5 точек калибровки (4 - края + 1 - центр).
	uint16_t pos_xy[] = {5, 5, 5, lcd->Height - 5, lcd->Width - 5, 5, lcd->Width - 5, lcd->Height - 5, lcd->Width/2, lcd->Height/2};
	//Массивы для хранения данных о точках калибровки.
	tPoint p_display[5], p_touch[5];
	//Таблица с дисплейными координатами для проверки после калибровки.
	uint16_t ver_xy[] = {lcd->Width / 2, 20, lcd->Width / 2, lcd->Height - 20};
	uint8_t f_error, i;  //Флаг ошибки при проверки калибровки и счетчик точек калибровки.
	t->fl_interrupt = 1; //Разрешаем обновление координат тачскрина в прерывании
	while (1) { //Пока калибровка не будет успешной.
		i = 0;
		LCD_Fill(lcd, COLOR_WHITE); //Очищаем дисплей.
		//Вывод сообщения о том, что необходимо кликнуть на пять точек (по краям и в центре дисплея).
		LCD_WriteString(lcd, 0, 0, "Calibration. You will be prompted to click on 5 dots "\
				        "(on the edges and in the center of the display). Get ready.",
						&Font_8x13, COLOR_BLUE, COLOR_WHITE, LCD_SYMBOL_PRINT_FAST);
		LL_mDelay(4000); //Ждем 4 секунды.
		LCD_Fill(lcd, COLOR_WHITE); //Очищаем дисплей.
		while (i < 5) { //Цикл по точкам калибровки.
			//Рисуем окружность c перекрестием - точка калибровки.
			draw_calibrate_point(lcd, pos_xy[2 * i], pos_xy[2 * i + 1], 5);
			//Ждем касания тачскрина и снятия этого касания.
			read_touch(t);
			//Запоминаем дисплейные координаты - центр калибровочного круга.
			p_display[i].x = pos_xy[2 * i];
			p_display[i].y = pos_xy[2 * i + 1];
			//Запоминаем координаты тачскрина, соответствующие дисплейным.
			p_touch[i] = t->point;
			LCD_Fill(lcd, COLOR_WHITE); //Очищаем дисплей.
			LL_mDelay(1300); //Ждем 1.3 секунды.
			i++; //Переходим к следующей точке калибровки.
		}
		//Раcсчитываем коэффициенты для перехода от координат тачскрина в дисплейные координаты.
		CoefCalc(p_display, p_touch, &t->coef, 5);
		//Проверка калибровочных параметров.
		//Вывод сообщения о том, что необходимо кликнуть на две точки (вверху и внизу дисплея).
		LCD_WriteString(lcd, 0, 0, "Checking calibration parameters. You will be prompted to click on two dots "\
				        "(top and bottom of the display). Get ready.",
						&Font_8x13, COLOR_BLUE, COLOR_WHITE, LCD_SYMBOL_PRINT_FAST);
		LL_mDelay(4000); //Ждем 4 секунды.
		LCD_Fill(lcd, COLOR_WHITE); //Очищаем дисплей.
		f_error = i = 0; //Инициализируем флаг ошибки и счетчик точек проверки калибровки
		while (i < 2) {
			//Рисуем окружность c перекрестием - точка калибровки.
			draw_calibrate_point(lcd, ver_xy[2 * i], ver_xy[2 * i + 1], 5);
			//Ждем касания тачскрина и снятия этого касания.
			read_touch(t);
			//Переходим от координат тачскрина к дисплейным координатам.
			XPT2046_ConvertPoint(&p_display[0], &t->point, &t->coef);
			//Проверяем, принадлежит ли точка касания тачскрина заданному диапазону координат.
			//Если выходит за границы диапазона, то устанавливаем флаг ошибки калибровки.
			if (p_display[0].x < ver_xy[2 * i] - 5 || p_display[0].x > ver_xy[2 * i] + 5 ||
				p_display[0].y < ver_xy[2 * i + 1] - 5 || p_display[0].y > ver_xy[2 * i + 1] + 5) f_error = 1;
			LCD_Fill(lcd, COLOR_WHITE); //Очищаем дисплей.
			LL_mDelay(1300); //Ждем 1.3 секунды.
			i++; //К следующей точке.
		}
		if (!f_error)  { //Если нет ошибок в калибровочных параметрах, то выводим сообщение и выходим.
			LCD_WriteString(lcd, 0, 0, "Calibration OK!", &Font_8x13, COLOR_BLUE, COLOR_WHITE, LCD_SYMBOL_PRINT_FAST);
			LL_mDelay(2000); //Ждем 2 секунды
			break;
		}
		//Если есть ошибки в калибровочных параметрах, то выводим сообщение и повторяем процесс калибровки.
		LCD_WriteString(lcd, 0, 0, "Calibration Error!", &Font_8x13, COLOR_RED, COLOR_WHITE, LCD_SYMBOL_PRINT_FAST);
		LL_mDelay(2000); //Ждем 2 секунды
	}
	LCD_Fill(lcd, COLOR_WHITE); //Очищаем дисплей
}
