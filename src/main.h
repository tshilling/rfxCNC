#pragma once
#include "freertos/ringbuf.h"

#ifndef main_h
#define main_h


#define LED_BUILTIN 2
extern QueueHandle_t   server_to_engine_queue;
extern RingbufHandle_t server_to_engine_buffer;
struct msg_to_engine_struct{
    String msg;
    bool read;
};
#endif