#pragma once

#ifndef GCodeParser_h
#define GCodeParser_h

#include "Arduino.h"
#include "CNCHelpers.h"

namespace CNC_ENGINE{
        class command_class{
            public:
                String comment = "";
                String command = "";
                std::vector<keyValuePair> parameter;
                command_class(){
                }
        };
        command_class parse_gcode(String input);
}

#endif