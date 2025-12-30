#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "cli_queue.h"
#include "queue.h"
#include "cli_driver.h"

#define MESSAGE_SIZE    1U     /* The size of the received message is 1 byte for the UART */
#define QUEUE_SIZE      64     /* Queue size (number of buffer cells) */

#define DEBUG_QUEUE    false

typedef struct 
{
  uint8_t data_uart[MESSAGE_SIZE];
} queue_type_t;

QUEUE(cli_uart, queue_type_t, QUEUE_SIZE)

static cli_uart cli_queue;


void cli_init_queue(void) 
{
  cli_uart_init_queue(&cli_queue);
}

bool cli_enque(uint8_t *message_in) 
{
  return cli_uart_enque(&cli_queue, (queue_type_t*)message_in);
}

bool cli_deque(uint8_t *message_out) 
{
#if DEBUG_QUEUE
  debugPrintf("q l:%d e:%d b:%d"CLI_NEW_LINE, cli_queue.current_load, cli_queue.begin, cli_queue.end);
#endif
  return (cli_uart_deque(&cli_queue, (queue_type_t*)message_out));
}

