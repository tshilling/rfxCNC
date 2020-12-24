//Spindle On (Clockwise)
#pragma once
#include "Arduino.h"
#include "operations.h"
namespace CNC_ENGINE{
    class M9:public operation_class{    
        public:               
        M9(){

        }
        ~M9(){

        }
        String get_type(){
            return "M9";
        }
        String get_log(){
            return "Coolant Off";
        }
    };
}