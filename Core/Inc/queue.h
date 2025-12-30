#pragma once

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdint.h>
#include <stdbool.h>


#ifdef __GNUC__ 
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE inline
#endif

/*
Simple FIFO Queue - Ring Buffer V0.1.0
Autor: Malyshev Sergey 2024-2025
https://github.com/sergey12malyshev/Simple-queue
*/
#define QUEUE(name, type, size)                                                    \
                                                                                   \
_Static_assert(size > 0, "Queue size must be positive");                           \
_Static_assert(size < UINT16_MAX, "The maximum queue size has been exceeded");     \
_Static_assert((size & (size - 1)) == 0, "Queue size must be a power of two");     \
                                                                                   \
typedef struct {                                                                   \
    type messages[size];                                                           \
    /*index of the head*/                                                          \
    uint16_t begin;                                                                \
    /*index of the tail*/                                                          \
    uint16_t end;                                                                  \
    /*load level the queue*/                                                       \
    uint16_t current_load;                                                         \
} name;                                                                            \
                                                                                   \
static FORCE_INLINE bool name##_init_queue(name *queue)                            \
{                                                                                  \
  if (queue == NULL)                                                               \
  {                                                                                \
    return false;                                                                  \
  }	                                                                               \
                                                                                   \
  queue->begin = 0;                                                                \
  queue->end = 0;                                                                  \
  queue->current_load = 0;                                                         \
  memset(&queue->messages[0], 0, size * sizeof(type));                             \
  return true;                                                                     \
}                                                                                  \
                                                                                   \
static FORCE_INLINE bool name##_enque(name *queue, type const *message)            \
{                                                                                  \
  if (queue->current_load < size)                                                  \
  {                                                                                \
    if (queue->end >= size)                                                        \
    {                                                                              \
      queue->end = 0;                                                              \
    }                                                                              \
    queue->messages[queue->end] = *message;                                        \
    queue->end = (queue->end + 1) % size;                                          \
    queue->current_load++;                                                         \
    return true;                                                                   \
  }                                                                                \
  else                                                                             \
  {                                                                                \
    return false;                                                                  \
  }                                                                                \
}                                                                                  \
                                                                                   \
static FORCE_INLINE bool name##_deque(name *queue, type *message)                  \
{                                                                                  \
  if (queue->current_load > 0)                                                     \
  {                                                                                \
    *message = queue->messages[queue->begin];                                      \
    memset(&queue->messages[queue->begin], 0, sizeof(type));                       \
    queue->begin = (queue->begin + 1) % size;                                      \
    queue->current_load--;                                                         \
    return true;                                                                   \
  }                                                                                \
  else                                                                             \
  {                                                                                \
    return false;                                                                  \
  }                                                                                \
}                                                                                  \
                                                                                   \
static FORCE_INLINE uint16_t name##_getQueueLoad(name *queue)                      \
{                                                                                  \
  return queue->current_load;                                                      \
}                                                                                  \
                                                                                   \
static FORCE_INLINE bool name##_is_queue_full(name *queue)                         \
{                                                                                  \
  return (queue->current_load >= size);                                            \
}                                                                                  \
                                                                                   \
static FORCE_INLINE bool name##_is_queue_empty(name *queue)                        \
{                                                                                  \
  return (queue->current_load == 0);                                               \
}                                                                                  \
                                                                                   \
static FORCE_INLINE int name##_get_queue_free(name *queue)                         \
{                                                                                  \
  return (size - queue->current_load);                                             \
}                                                                                   

#endif /* __QUEUE_H__ */