#ifndef BUTTON_THREAD_H__
#define BUTTON_THREAD_H__

bool getTxButtonState(void);
bool getScanButtonState(void);
bool getjammButtonState(void);
bool getBootButtonState(void);
bool getGpsButtonState(void);


#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

PT_THREAD(Button_Thread(struct pt *pt));

#endif /* BUTTON_THREAD_H__ */