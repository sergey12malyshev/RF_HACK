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

void encoder_setStateSwitch(bool const s)
{
  encoderSwitch = s;
}


void encoder_init(void)
{
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
}


int32_t prevCounter = 0;

void encoder_process(void) 
{
  int32_t currCounter = __HAL_TIM_GET_COUNTER(&htim2);

  currCounter = 32767 - ((currCounter-1) & 0xFFFF) / 2;

  if(currCounter != prevCounter) 
  {
    debugPrintf("%d"CLI_NEW_LINE, currCounter);
    prevCounter = currCounter;
  }

  if(encoder_getStateSwitch())
  {
    debugPrintf("SW!"CLI_NEW_LINE);
  }
}
