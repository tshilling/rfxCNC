// Compulsory Stop, non-optional
#pragma once
#include "Arduino.h"
#include "operations.h"
namespace CNC_ENGINE{
    class M0:public operation_class{   
        public:                
        M0(){

        }
        ~M0(){

        }
        bool execute(){
            machine_state.is_active = false;
            is_complete = true;
            return is_complete;
        }
        String get_type(){
            return "M0";
        }
        String get_log(){
            return "Compulsory Stop";
        }
    };
}