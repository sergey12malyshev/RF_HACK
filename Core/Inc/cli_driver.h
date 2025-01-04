#ifndef __CLI_DRIVER_H
#define __CLI_DRIVER_H

#define DEBUG_MASSAGE   true
#define DEBUG_PRINT(...) do { if (DEBUG_MASSAGE) debugPrintf(__VA_ARGS__); } while (0)  


/* Config CLI: */
#define CLI_SHELL_MAX_LENGTH     300U      // Shell out max command line size

#define CLI_INPUT_BUFF_LENGTH    12U      // CLI input max line size
////

#define CLI_PROMPT_STR  "rf> "

#define CLI_NEW_LINE    "\r\n"
#define CLI_TAB         "\t"
#define CLI_TAB2        "\t\t"
#define CLI_CLEAR_LINE  "\33[2K\r"

#define CLI_OK    "[\033[32mOK\033[0m] "
#define CLI_ERROR "[\033[31mERROR\033[0m] "
#define CLI_RX    "[\033[32mRX\033[0m] "
#define CLI_TX    "[\033[36mTX\033[0m] "
#define CLI_SYS   "[\033[32mSYS\033[0m] "
// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797

#define CYAN_CLR  "\033[36m"
#define RED_CLR   "\033[31m"
#define YEL_CLR   "\033[33m"
#define GREEN_CLR "\033[32m"
#define RST_CLR   "\033[0m"

#define CLI_DISPLAY_CLEAR()    debugPrintf("\033[2J")
#define CLI_RESET_CURSOR()     debugPrintf("\033[H")
#define CLI_HIDE_CURSOR()      debugPrintf("\033[?25l")
#define CLI_SHOW_CURSOR()      debugPrintf("\033[?25h")


int debugPrintf(const char *serial_data, ...);

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

PT_THREAD(Application_Thread(struct pt *pt));

#endif /* __CLI_DRIVER_H  */