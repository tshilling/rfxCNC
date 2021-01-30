//Spindle On (Clockwise)
#pragma once
#include "Arduino.h"
#include "operations.h"
namespace RFX_CNC{
    class M3:public operation_class{    
        public:               
        M3(){
            execute_in_interrupt = false;
            is_plannable = false;
        }
        ~M3(){

        }
        bool execute(){
            copy_parameters_out();
            MACHINE::machine_state->spindle_state = MACHINE::machine_state_class::spindle_CW;
            return true;
        }
        String get_type(){
            return "M3";
        }
        String get_log(){
            return "Spindle CW";
        }
    };
}