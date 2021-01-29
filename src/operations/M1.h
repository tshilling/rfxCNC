// Optional Stop
#pragma once
#include "Arduino.h"
#include "operations.h"
namespace RFX_CNC{
    class M1:public operation_class{     
        public:              
        M1(){

        }
        ~M1(){

        }
        bool execute(){
            if(MACHINE::optional_stop)
                MACHINE::is_active = false;
            is_complete = true;
            return is_complete;
        }
        String get_type(){
            return "M1";
        }
        String get_log(){
            return "Optional Stop";
        }
    };
}