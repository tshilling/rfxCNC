#pragma once

#ifndef CNCEngine_h
#define CNCEngine_h

#include <Arduino.h>
#include "config.h"
#include "parsers/commandParser.h"

namespace RFX_CNC{
    void init(void);
    void _engineLoop(void * parameter);
    void rx_command(String command);
    void process_command(PARSER::command_struct* commands);
};

#endif