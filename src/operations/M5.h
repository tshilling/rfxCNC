// Spindle Stop
#pragma once
#include "Arduino.h"
#include "operationS.h"
namespace RFX_CNC{
    class M5:public operation_class{   
        public:         
        M5(){
            execute_in_interrupt = false;
            is_plannable = false;

        }
        ~M5(){

        }
        bool execute(){
            copy_parameters_out();
            MACHINE::machine_state->spindle_state = MACHINE::machine_state_class::spindle_off;
            return true;
        }
        String get_type(){
            return "M5";
        }
        String get_log(){
            return "Spindle Stop";
        }
    };
}