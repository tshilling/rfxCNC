// Spindle CCW
#pragma once
#include "Arduino.h"
#include "operations.h"
namespace CNC_ENGINE{
    class M4:public operation_class{      
        public:      
        M4(){

        }
        ~M4(){

        }
        String get_type(){
            return "M4";
        }
        String get_log(){
            return "Spindle CCW";
        }
    };
}