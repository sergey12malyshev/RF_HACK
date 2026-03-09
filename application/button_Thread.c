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

#define BUTTON_H        6

/* Button coordinates */
#define BUTTON_GPS_X    184
#define BUTTON_GPS_Y    25

#define BUTTON_SCAN_X   184
#define BUTTON_SCAN_Y   85

#define BUTTON_JAMM_X   184
#define BUTTON_JAMM_Y   145

#define BUTTON_TX_X     184
#define BUTTON_TX_Y     205

#define BUTTON_BOOT_X   184
#define BUTTON_BOOT_Y   265

/* Button identifiers */
typedef enum 
{
  BUTTON_TX = 0,
  BUTTON_SCAN,
  BUTTON_JAMM,
  BUTTON_BOOT,
  BUTTON_GPS,
  BUTTON_COUNT
} ButtonId;

/* Button descriptor */
typedef struct 
{
  uint16_t x;
  uint16_t y;
  const char *label;
  uint32_t active_color;
  bool *state;               /* pointer to external state variable */
} ButtonDesc;

/* Button states (external) */
static bool TxButton, scanButton, jammButton, bootButton, gpsButton;

/* Button descriptors array */
static const ButtonDesc buttons[BUTTON_COUNT] = 
{
  [BUTTON_TX]   = { BUTTON_TX_X,   BUTTON_TX_Y,   "TX",   COLOR_RED,    &TxButton },
  [BUTTON_SCAN] = { BUTTON_SCAN_X, BUTTON_SCAN_Y, "SCAN", COLOR_CYAN,   &scanButton },
  [BUTTON_JAMM] = { BUTTON_JAMM_X, BUTTON_JAMM_Y, "JAMM", COLOR_RED,    &jammButton },
  [BUTTON_BOOT] = { BUTTON_BOOT_X, BUTTON_BOOT_Y, "BOOT", COLOR_RED,    &bootButton },
  [BUTTON_GPS]  = { BUTTON_GPS_X,  BUTTON_GPS_Y,  "GPS",  COLOR_YELLOW, &gpsButton }
};

/* Get state functions (for external use) */
bool getTxButtonState(void)    { return TxButton;   }
bool getScanButtonState(void)  { return scanButton; }
bool getJammButtonState(void)  { return jammButton; }
bool getBootButtonState(void)  { return bootButton; }
bool getGpsButtonState(void)   { return gpsButton;  }

/* Reset all button states */
static void allButtonClearState(void)
{
  for (int16_t i = 0; i < BUTTON_COUNT; i++) 
  {
    *buttons[i].state = false;
  }
}

/* Draw a single button */
static void buttonDraw(ButtonId id, const uint32_t color)
{
#define BUTTON_BORDER      2

  const ButtonDesc *btn = &buttons[id];
  int16_t hw = LCD_GetHeight(lcd) / BUTTON_H;   // button side length

  int16_t x = btn->x;
  int16_t y = btn->y;

  LCD_DrawRectangle(lcd, x, y, x + hw - BUTTON_BORDER, y + hw - BUTTON_BORDER, COLOR_WHITE);
  LCD_DrawFilledRectangle(lcd, x + BUTTON_BORDER, y + BUTTON_BORDER, x + hw - 4, y + hw - 4, color);

  // Center text roughly (label length * font width / 2)
  int16_t text_x = x + hw/2 - (strlen(btn->label) * 8)/2;   // Font_8x13 width = 8
  int16_t text_y = y + hw/2 - 6;                            // half of 13
  LCD_WriteString(lcd, text_x, text_y, btn->label,
                  &Font_8x13, COLOR_BLACK, COLOR_BLACK,
                  LCD_SYMBOL_PRINT_PSETBYPSET);
}

/* Clear all buttons (draw white) */
static void button_logoClear(void)
{
  for (int16_t i = 0; i < BUTTON_COUNT; i++) 
  {
    buttonDraw((ButtonId)i, COLOR_WHITE);
  }
}

/* Touch screen handler */
static void buttonHandler(XPT2046_Handler *t)
{
  static uint8_t i = 0;
  static bool noClick = false;
  const uint8_t antibouncing_k = 1;

  // Poll touch screen (must not be called in interrupt)
  (void)XPT2046_GetTouch(t);

  if (t->click) 
  {
    tPoint point_d;
    XPT2046_ConvertPoint(&point_d, &t->point, &t->coef);
    int16_t x = point_d.x;
    int16_t y = point_d.y;

    // Clamp to display bounds
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= lcd->Width)  x = lcd->Width - 1;
    if (y >= lcd->Height) y = lcd->Height - 1;

    int16_t hw = LCD_GetHeight(lcd) / BUTTON_H;

    // Check each button
    for (int16_t id = 0; id < BUTTON_COUNT; id++) 
    {
        const ButtonDesc *btn = &buttons[id];
        if (x >= btn->x && x < btn->x + hw &&
            y >= btn->y && y < btn->y + hw) 
        {
            // Button touched
            if (!*btn->state) 
            {
              // Activate this button, deactivate others
              allButtonClearState();
              button_logoClear();
              *btn->state = true;
              buttonDraw((ButtonId)id, btn->active_color);
            } 
            else 
            {
                // Already active: deactivate if click is finished
                if (noClick) 
                {
                  buttonDraw((ButtonId)id, COLOR_WHITE);
                  *btn->state = false;
                }
            }
            break;  // only one button per touch
        }
    }

    noClick = false;
    i = 0;
  } 
  else 
  {
    if (i++ > antibouncing_k) 
    {
      noClick = true;
    }
  }
}

/*
 * Protothread Button_Thread
 * Creates and processes button presses on the display
 */
PT_THREAD(Button_Thread(struct pt *pt))
{
  static uint32_t timer1;

  PT_BEGIN(pt);

  allButtonClearState();

  PT_DELAY_MS(pt, &timer1, screen_booting_get_time() + 100);

  button_logoClear();

  while (true) 
  {
    PT_WAIT_UNTIL(pt, timer(&timer1, 150));
    buttonHandler(&touch1);
    PT_YIELD(pt);
  }

  PT_END(pt);
}