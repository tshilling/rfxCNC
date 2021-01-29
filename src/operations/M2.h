// Optional Stop
#pragma once
#include "Arduino.h"
#include "operations.h"
namespace RFX_CNC{
    class M2:public operation_class{     
        public:              
        M2(){

        }
        ~M2(){

        }
        String get_type(){
            return "M2";
        }
        String get_log(){
            return "End of Program";
        }
    };
}