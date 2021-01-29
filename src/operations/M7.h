//Spindle On (Clockwise)
#pragma once
#include "Arduino.h"
#include "operations.h"
namespace RFX_CNC{
    class M7:public operation_class{    
        public:               
        M7(){

        }
        ~M7(){

        }
        String get_type(){
            return "M7";
        }
        String get_log(){
            return "Coolant Mist";
        }
    };
}