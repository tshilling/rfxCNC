//Spindle On (Clockwise)
#pragma once
#include "Arduino.h"
#include "operations.h"
namespace CNC_ENGINE{
    class M3:public operation_class{    
        public:               
        M3(){

        }
        ~M3(){

        }
        String get_type(){
            return "M3";
        }
        String get_log(){
            return "Spindle CW";
        }
    };
}