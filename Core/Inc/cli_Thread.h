#pragma once
#ifndef __CLI_TASK_H__
#define __CLI_TASK_H__

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

PT_THREAD(CLI_Thread(struct pt *pt));
void cli_uart_callBack(void);

#endif /*__CLI_TASK_H__ */