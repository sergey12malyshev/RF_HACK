#include <stdbool.h>
#include <stdint.h>

#include "buzzer_driver.h"
#include "main.h"

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#include "time.h"

/*
* Buzzer driver
* 
*/

static GPIO_TypeDef *buzzer_port = NULL;
static uint32_t buzzer_pin;

static bool buzzer_sound_on;
static uint32_t buzzer_timeout = 5;

void buzzer_soundOn(uint32_t time)
{
  if (time == 0)
  {
    return;
  }
  buzzer_sound_on = true;
  buzzer_timeout = time;
}

static void buzzer_enable(void)
{
  LL_GPIO_SetOutputPin(buzzer_port, buzzer_pin);
}

static void buzzer_disable(void)
{
  LL_GPIO_ResetOutputPin(buzzer_port, buzzer_pin);
  buzzer_sound_on = false;
}

bool buzzer_init(GPIO_TypeDef *GPIOx, uint32_t PinMask)
{
  if (GPIOx == NULL)
  {
    return true;
  }

  buzzer_port = GPIOx;
  buzzer_pin = PinMask;

  return false;
}



/*
 * Protothread Buzzer_Thread
 *
 * Creating and processing sound
 */

PT_THREAD(Buzzer_Thread(struct pt *pt))
{
  static uint32_t timer_buzzer;

  PT_BEGIN(pt);

  PT_DELAY_MS(pt, &timer_buzzer, timer_buzzer);

  while (true)
  {
    PT_WAIT_UNTIL(pt, buzzer_sound_on);

    buzzer_enable();
    PT_DELAY_MS(pt, &timer_buzzer, buzzer_timeout);
    buzzer_disable();

    PT_YIELD(pt);
  }

  PT_END(pt);
}
