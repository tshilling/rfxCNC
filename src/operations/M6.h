// Compulsory Stop, non-optional
#pragma once
#include "Arduino.h"
#include "operations.h"
namespace RFX_CNC{
    class M6:public operation_class{   
        public:         
        M6(){

        }
        ~M6(){

        }
        String get_type(){
            return "M6";
        }
        String get_log(){
            return "Change Tool";
        }
    };
}