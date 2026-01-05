#include "power.h"
#include "main.h"

#ifdef __cplusplus

void Power::system_reset() 
{
  NVIC_SystemReset();
}

void Power::wdt_reset() 
{
  while(1) {}
}

void Power::disableInterrupts() 
{
  __disable_irq();
}

void Power::enableInterrupts() 
{
  __enable_irq();
}

#endif // __cplusplus

// C-compatible functions
#ifdef __cplusplus
extern "C" {
#endif

void power_systemReset(void) 
{
  Power::system_reset();
}

void power_wdtReset(void) 
{
  Power::wdt_reset();
}

#ifdef __cplusplus
}
#endif