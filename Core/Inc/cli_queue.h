#pragma once
#ifndef __CLI_QUEUE_H
#define __CLI_QUEUE_H

#include <stdio.h>


void cli_init_queue(void);
bool cli_enque(uint8_t *message_in);
bool cli_deque(uint8_t *message_out);

#endif /* __CLI_QUEUE_H  */