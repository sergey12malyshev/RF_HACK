#include <stdbool.h>

#include "workStates.h"

static Work_state mainState;

void setWorkSate(const Work_state s)
{
  mainState = s;
}

Work_state getWorkState(void)
{
  return mainState;
}

bool isWorkStateSet(Work_state s)
{
  return (bool) (s == mainState);
}