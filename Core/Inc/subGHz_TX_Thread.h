#ifndef __TX_TREAD_H__
#define __TX_TREAD_H__

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

PT_THREAD(subGHz_TX_Thread(struct pt *pt));

uint8_t transmittRF(const char *packet_loc, uint8_t len);

#endif /*__TX_TREAD_H__ */