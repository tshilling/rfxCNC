#pragma once

#ifndef RFX_Server_h
#define RFX_Server_h

#include <Arduino.h>
#include "SPIFFS.h"

namespace RFX_Server{
    static String apSsid;

    void init();
    void _serverLoop(void * parameter);
};

#endif