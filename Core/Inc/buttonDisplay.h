#ifndef DISPLAY_INIT_H__
#define DISPLAY_INIT_H__

bool getTxButtonState(void);
bool getScanButtonState(void);
bool getjammButtonState(void);
bool getBootButtonState(void);


#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

PT_THREAD(Display_Thread(struct pt *pt));

#endif /* DISPLAY_INIT_H__ */