// Compulsory Stop, non-optional
#pragma once
#include "Arduino.h"
#include "operations.h"
namespace RFX_CNC{
    class M0:public operation_class{   
        public:                
        M0(){

        }
        ~M0(){

        }
        bool execute(){
            MACHINE::is_active = false;
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