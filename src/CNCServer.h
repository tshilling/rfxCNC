#pragma once

#ifndef CNCServer_h
#define CNCServer_h

#include <Arduino.h>
#include "SPIFFS.h"

namespace CNCServer{
    extern Stream* stream;
    static String apSsid;

    void init(Stream *_stream);
    void _serverLoop(void * parameter);
};

#endif