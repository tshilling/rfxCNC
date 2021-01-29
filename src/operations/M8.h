//Spindle On (Clockwise)
#pragma once
#include "Arduino.h"
#include "operations.h"
namespace RFX_CNC{
    class M8:public operation_class{    
        public:               
        M8(){

        }
        ~M8(){

        }
        String get_type(){
            return "M8";
        }
        String get_log(){
            return "Coolant Flood";
        }
    };
}