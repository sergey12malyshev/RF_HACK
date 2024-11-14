#ifndef __APPL_THREAD_H__
#define __APPL_THREAD_H__

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

PT_THREAD(Application_Thread(struct pt *pt));

void screen_clear(void);

bool getBootingScreenMode(void);
void screen_bootload(void);
uint16_t screen_booting_get_time(void);

#endif /*__APPL_THREAD_H__ */