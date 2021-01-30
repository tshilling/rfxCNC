// Spindle CCW
#pragma once
#include "Arduino.h"
#include "operations.h"
namespace RFX_CNC{
    class M4:public operation_class{      
        public:      
        M4(){
            execute_in_interrupt = false;
            is_plannable = false;
        }
        ~M4(){

        }
        bool execute(){
            copy_parameters_out();
            MACHINE::machine_state->spindle_state = MACHINE::machine_state_class::spindle_CCW;
            return true;
        }
        String get_type(){
            return "M4";
        }
        String get_log(){
            return "Spindle CCW";
        }
    };
}