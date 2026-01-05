#include <stdbool.h>
#include <stdint.h>

#include "buzzer_driver.h"
#include "main.h"

/*
* Buzzer driver
* 
*/

static GPIO_TypeDef *buzzer_port = NULL;
static uint32_t buzzer_pin;


void buzzer_enable(void)
{
  LL_GPIO_SetOutputPin(buzzer_port, buzzer_pin);
}

void buzzer_disable(void)
{
  LL_GPIO_ResetOutputPin(buzzer_port, buzzer_pin); 
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