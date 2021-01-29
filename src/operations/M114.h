// Get Position
#pragma once
#include "Arduino.h"
#include "operations.h"
namespace RFX_CNC{
    class M114:public operation_class{     
        public:              
        M114(){

        }
        ~M114(){

        }
        bool execute(){
            console.log("Position: ");
            for(uint8_t i=0;i<Config::axis_count;i++){
                console.log(String(MACHINE::machine_state->get_position_in_coordinates(i))+", ");
            }
            console.logln();
            is_complete = true;
            return is_complete;
        }
        String get_type(){
            return "M114";
        }
        String get_log(){
            return "";
        }
    };
}