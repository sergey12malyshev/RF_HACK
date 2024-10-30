#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "cli_driver.h"
#include "usart.h"

extern UART_HandleTypeDef huart1;

static char printBufer[CLI_SHELL_MAX_LENGTH] = {0};

int debugPrintf(const char *serial_data, ...)
{
  va_list arg;
  va_start(arg, serial_data);
  uint16_t len = vsnprintf(printBufer, sizeof(printBufer), serial_data, arg);
  va_end(arg);

  const uint8_t block_timeout_ms = 40u;
  HAL_UART_Transmit(&huart1, (uint8_t *)printBufer, len, block_timeout_ms);

  return len;
}
