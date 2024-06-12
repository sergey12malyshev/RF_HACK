#include <stdbool.h>

#include "cli_queue.h"

/*
Portable array-based cyclic FIFO queue. https://stackoverflow.com/questions/52783068/how-to-implement-a-message-queue-in-standard-c
*/
void cli_init_queue(QUEUE *queue) 
{
  queue->begin = 0;
  queue->end = 0;
  queue->current_load = 0;
  memset(&queue->messages[0], 0, QUEUE_SIZE * sizeof(MESSAGE));
}

bool cli_enque(QUEUE *queue, MESSAGE *message) 
{
  if (queue->current_load < QUEUE_SIZE) 
  {
    if (queue->end == QUEUE_SIZE)
    {
      queue->end = 0;
    }
    queue->messages[queue->end] = *message;
    queue->end++;
    queue->current_load++;

    return true;
  } 
  return false;
}

bool cli_deque(QUEUE *queue, MESSAGE *message) 
{
  if (queue->current_load > 0) 
  {
    *message = queue->messages[queue->begin];
    memset(&queue->messages[queue->begin], 0, sizeof(MESSAGE));
    queue->begin = (queue->begin + 1) % QUEUE_SIZE;
    queue->current_load--;

    return true;
  } 
  return false;
}

