#ifndef __RF_TREAD_H__
#define __RF_TREAD_H__

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

PT_THREAD(subGHz_RX_Thread(struct pt *pt));

typedef struct
{
  uint16_t countMessage;
  uint16_t countError;
  int16_t RSSI;
  int32_t RSSI_main;
  uint8_t dataString[8];
} RF_t;

int RSSIconvert(char raw_rssi);

#endif /*__RF_TREAD_H__ */