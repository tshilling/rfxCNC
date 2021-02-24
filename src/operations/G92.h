#pragma once
#include "Arduino.h"
#include "operations.h"

/*
Dwell pauses the command queue and waits for a period of time.

If both S and P are included, S takes precedence.
*/

namespace RFX_CNC{
    class operation_G92:public operation_class{ 
            
        public:
           
        operation_G92(command_block* _block):operation_class(_block){

        }
        ~operation_G92(){

        } 
        unsigned long last_time = 0;
        bool sub_execute(MACHINE::machine_state_class* state){
            for(uint8_t i = 0; i < config.axis.size();i++){
                float v = block->parameter[config.axis[i].id - 'A'];
                if(!isnan(v) && !isinf(v))
                config.get_coordinate_system(G92-G54)[i] = v;
            }
            return true;
        }
        status_enum init(MACHINE::machine_state_class* state){
            operation_class::init(state);
            return status_ok;
        }
        String get_log(){
            return "G92";
        }
        String get_type(){
            return "G92";
        }
    };
}