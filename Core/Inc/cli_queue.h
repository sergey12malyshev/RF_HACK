#pragma once
#ifndef __CLI_QUEUE_H
#define __CLI_QUEUE_H

#include <stdio.h>

#define MESSAGE_SIZE    1U     /* Размер принимаемого собщения - 1 байт для UART */
#define QUEUE_SIZE      64     /* Queue size (number of buffer cells) */

typedef struct 
{
  uint8_t msg[MESSAGE_SIZE];
} MESSAGE;

typedef struct 
{
  MESSAGE messages[QUEUE_SIZE];
  int begin;
  int end;
  int current_load;
} QUEUE;

void cli_init_queue(QUEUE *queue);
bool cli_enque(QUEUE *queue, MESSAGE *message);
bool cli_deque(QUEUE *queue, MESSAGE *message);

#endif /* __CLI_QUEUE_H  */