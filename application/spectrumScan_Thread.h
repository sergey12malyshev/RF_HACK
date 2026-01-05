#pragma once
#ifndef __SPC_THREAD_H__
#define __SPC_THREAD_H__

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

PT_THREAD(spectrumScan_Thread(struct pt *pt));

#endif /*__SPC_THREAD_H__ */