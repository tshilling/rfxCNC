// Spindle Stop
#pragma once
#include "Arduino.h"
#include "operationS.h"
namespace CNC_ENGINE{
    class M5:public operation_class{   
        public:         
        M5(){

        }
        ~M5(){

        }
        String get_type(){
            return "M0";
        }
        String get_log(){
            return "Spindle Stop";
        }
    };
}