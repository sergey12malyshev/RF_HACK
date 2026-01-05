#include <stdbool.h>

#include "workStates.h"

static Work_state_t mainState;

void setWorkSate(const Work_state_t s)
{
  mainState = s;
}

Work_state_t getWorkState(void)
{
  return mainState;
}

bool isWorkStateSet(Work_state_t s)
{
  return (bool) (s == mainState);
}