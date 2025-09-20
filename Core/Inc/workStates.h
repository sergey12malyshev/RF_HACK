#ifndef __WORK_STATES_H
#define __WORK_STATES_H

typedef enum 
{
  RX_MODE = 0, 
  TX_MODE,
  SCAN_MODE,
  JAMMER_MODE,
  GPS_MODE,
  NUMBER_STATE
}Work_state_t;


void setWorkSate(const Work_state_t s);
Work_state_t getWorkState(void);
bool isWorkStateSet(Work_state_t s);

#endif // __WORK_STATES_H