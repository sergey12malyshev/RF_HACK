#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "main.h"
#include "encoderDriver.h"
#include "cli_driver.h"
#include "tim.h"


static bool encoderSwitch;
static bool encoderSwitchIRQ;
static uint32_t encoderTimeSwitch;

static int32_t currCounter;

bool encoder_getStateSwitch(void)
{
  bool rc = false;

  if(encoderSwitch)
  {
    rc = true;
    encoderSwitch = false;
  }

  return rc;
}

void encoder_setStateSwitch(bool const s, uint32_t time)
{
  encoderSwitchIRQ = s;
  encoderTimeSwitch = time;
}

int32_t encoder_getRotaryNum(void)
{
  return currCounter;
}

void encoder_setRotaryNum(int32_t n)
{
  __HAL_TIM_SET_COUNTER(&htim2, 32767 - n);
  currCounter = n;
}

void encoder_init(void)
{
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
  encoder_setRotaryNum(0);
}


int32_t prevCounter = 0;

void encoder_process(void) 
{
  currCounter = __HAL_TIM_GET_COUNTER(&htim2);

  currCounter = 32767 - ((currCounter-1) & 0xFFFF);

  if(currCounter != prevCounter) 
  {
    //debugPrintf("%d"CLI_NEW_LINE, currCounter);
    prevCounter = currCounter;
  }
  
 // https://istarik.ru/blog/stm32/148.html
  if(encoderSwitchIRQ && (HAL_GetTick() - encoderTimeSwitch) > 200)
  {
    encoderSwitch = true;
    encoderSwitchIRQ = false;
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_8);
    NVIC_EnableIRQ(EXTI9_5_IRQn);
  }
}
