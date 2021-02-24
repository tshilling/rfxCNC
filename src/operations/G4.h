#pragma once
#include "Arduino.h"
#include "operations.h"

/*
Dwell pauses the command queue and waits for a period of time.

If both S and P are included, S takes precedence.
*/

namespace RFX_CNC{
    class operation_G4:public operation_class{ 
            
        public:
        uint32_t msec_to_wait = 0;    
        operation_G4(command_block* _block):operation_class(_block){

        }
        ~operation_G4(){

        } 
        unsigned long last_time = 0;
        void set_wait_time(){
            if(!isnan(block->parameter[_S_]) && !isinf(block->parameter[_S_])){
                msec_to_wait = block->parameter[_S_]*1000.0f;
            }
            else if(!isnan(block->parameter[_P_]) && !isinf(block->parameter[_P_])){
                msec_to_wait = block->parameter[_P_]*1000.0f;
            }
        }
        bool sub_execute(MACHINE::machine_state_class* state){
            if(pass_count == 1){
                set_wait_time();
                last_time = millis();
            }
            unsigned long delta = (unsigned long)(millis() - last_time);
            if(delta >= msec_to_wait){
                msec_to_wait = 0;
                return true;
            }
            else
            {
                msec_to_wait-=delta;
            }
            last_time += delta;
            return false;
        }
        
        status_enum init(MACHINE::machine_state_class* state){
            operation_class::init(state);
            set_wait_time();
            return status_ok;
        }
        String get_log(){
            //if(pass_count == 0)
            //    set_wait_time();
            return "G4- Sec Remaining: "+String((float)msec_to_wait/1000.0f);
        }
        String get_type(){
            return "G4";
        }
    };
}