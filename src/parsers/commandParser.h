#pragma once
#include <Arduino.h>
#include "nuts_and_bolts.h"
namespace RFX_CNC
{
    namespace PARSER
    {
        extern String result_description[];

        struct key_value_pair_struct
        {
            String key = "";
            float value = 0;
        };
        struct command_struct
        {
            String input = "";
            String comment = "";
            std::vector<key_value_pair_struct> parameter;
        };
        struct result_struct
        {
            status_enum status;
            command_struct *command;
        };
        result_struct parse(String input);
    }
}