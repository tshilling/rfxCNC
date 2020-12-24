#include "Arduino.h"
#include <Console.h>
#include "GCodeParser.h"
#include "CNCHelpers.h"

namespace CNC_ENGINE{
        command_class parse_gcode(String input){
            command_class result;// = new command_class;
            if(input.length() == 0)
                return result;
            // Strip out end of line
            result.command = input;
            result.command.toUpperCase();
            keyValuePair* KVP = nullptr;
            String value = "";
            String command = "";
            uint8_t comment = 0;
            for(uint i = 0; i < result.command.length();i++){
                char current = result.command[i];
                if(current == ')'){
                    comment--; 
                    continue;
                }
                if(comment>=1)
                    result.comment+=current;
                if(current == '('){
                    comment++;  
                    continue;
                }
                if(comment > 0)
                    continue;

                if(current == ';' || current == '/')
                    break;

                if((current >= '0' && current <= '9') || current == '.' || current == '-'|| current == '+'){
                    if(command.length()!=0)
                        value += current;
                    continue;
                }
                if(current >= '!' && current <= '~'){ // Letter{
                    if(value.length() != 0){
                        result.parameter.push_back(keyValuePair());
                        KVP = &result.parameter[result.parameter.size()-1];
                        KVP->key = command;
                        KVP->value = value.toFloat();
                        command = "";
                        value = "";
                    }
                    command += current;
                    continue;
                }
            }                    
            if(value.length() != 0 && command.length() != 0){
                result.parameter.push_back(keyValuePair());
                KVP = &result.parameter[result.parameter.size()-1];
                KVP->key = command;
                KVP->value = value.toFloat();
                command = "";
                value = "";
            }

            return result;
        }
}