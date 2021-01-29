#pragma once
#include <Arduino.h>
namespace PARSER{
    extern String result_description[];
    enum result_enum{
        ok,
        zero_length,
        too_long,
        invalid_token
    };
    struct key_value_pair_struct{
        String  key     = "";
        float   value   = 0;
    };
    struct command_struct{
        String input = "";
        String comment = "";
        std::vector<key_value_pair_struct> parameter;
    };
    struct result_struct{
        result_enum status;
        command_struct* command;
    };
   result_struct parse(String input);
}