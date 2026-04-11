#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "cli_driver.h"
#include "usart.h"
#include "configFile.h"

extern UART_HandleTypeDef huart1;

static char printBufer[CLI_SHELL_MAX_LENGTH] = {0};

int debugPrintf(const char *serial_data, ...)
{
  va_list arg;
  va_start(arg, serial_data);
  int len = vsnprintf(printBufer, sizeof(printBufer), serial_data, arg);
  va_end(arg);

  if (len < 0)
  {
    return -1; /* Formatting error */
  }

  if (len >= (int)sizeof(printBufer))
  {
    len = sizeof(printBufer) - 1; /* Truncate to buffer size */
  }

  const uint8_t block_timeout_ms = 40u;

#if (CLI_ENABLE)
  if (HAL_UART_Transmit(&huart1, (uint8_t *)printBufer, (uint16_t)len, block_timeout_ms))
  {
    return -1;
  }
#endif

  return len;
}
