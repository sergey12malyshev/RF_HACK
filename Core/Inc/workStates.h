#ifndef __WORK_STATES_H
#define __WORK_STATES_H

typedef enum 
{
  RX = 0, 
  TX,
  SCAN,
  JAMMER,
  GPS_MODE,
  NUMBER_STATE
}Work_state;


void setWorkSate(const Work_state s);
Work_state getWorkState(void);
bool isWorkStateSet(Work_state s);

#endif // __WORK_STATES_H