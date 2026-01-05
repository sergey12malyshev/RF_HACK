#include <stdbool.h>
#include <stdio.h>

/*
* Battery driver
* NCR18650B Liitokala
*/

typedef struct _Battery
{
  uint16_t voltage;
  uint8_t charge_prc;
  uint32_t timeDischarge;
}Battery_t;


uint8_t battery_getChargePrecent(uint16_t vbat)
{ 
  float charge = 0.1169f*vbat - 385.54f; // y = 0,1169x - 385,54

  if (charge < 0.0)
  {
    charge = 0.0;
  }
  if (charge > 100.0)
  {
    charge = 100.0;
  } 

  return (uint8_t) charge;
}
