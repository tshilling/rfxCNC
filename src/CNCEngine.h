#pragma once

#ifndef CNCEngine_h
#define CNCEngine_h

#include <Arduino.h>
#include "config.h"

namespace CNC_ENGINE{
    void init(void);
    void _engineLoop(void * parameter);
    void rx_command(String command);
};

#endif