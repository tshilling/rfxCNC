// Get Position
#pragma once
#include "Arduino.h"
#include "operations.h"
namespace RFX_CNC{
    class operation_M114:public operation_class{     
        public:              
        operation_M114(command_block* _block):operation_class(_block)
        {

        }
        ~operation_M114(){

        }
        bool sub_execute(MACHINE::machine_state_class* state){
            operation_class::execute(state);
            console.log("Position: ");
            for(uint8_t i=0;i<config.axis.size();i++){
                console.log(String(state->steps_to_coordinate(state->absolute_position_steps[i], i))+", ");
            }
            console.logln();
            return true;
        }
        String get_type(){
            return "M114";
        }
        String get_log(){
            return "";
        }
    };
}