#pragma once
#ifndef __TIME_H__
#define __TIME_H__


/*
* The time management module
*/

#define PT_DELAY_MS(pt, var, t_ms)  {setTime(var); PT_WAIT_UNTIL(pt, timer(var, t_ms));}

static inline void setTime(uint32_t * time)
{
  *time = HAL_GetTick();
}

static inline __attribute__((always_inline)) bool timer(uint32_t * tickStart, const uint32_t timeout_ms)
{
  if ((uint32_t)(HAL_GetTick() - *tickStart) >= timeout_ms)
  {
    *tickStart = HAL_GetTick();
    return true;
  }
  return false;
}


#endif /* __TIME_H__ */