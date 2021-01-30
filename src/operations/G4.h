#pragma once
#include "Arduino.h"
#include "operations.h"
namespace RFX_CNC{
    class G4:public operation_class{ 
            
        public:
        uint32_t msec_to_wait = 0;    
        G4(){
            execute_in_interrupt = false;
        }
        ~G4(){

        } 
        unsigned long last_time = 0;
        bool execute(){   
            if(is_complete)
                return is_complete;  
            if(first_pass){
                copy_parameters_out();
                last_time = millis();
                first_pass = false;
            }
            unsigned long delta = (unsigned long)(millis() - last_time);
            if(delta >= msec_to_wait){
                msec_to_wait = 0;
                is_complete = true;
            }
            else
            {
                msec_to_wait-=delta;
            }
            last_time += delta;
            return is_complete;
        }
        operation_result_enum init(float _parameters[]){
            operation_class::init(_parameters);
            msec_to_wait = parameters[_P_];
            return success;
        }
        String get_log(){
            return "G4- Sec Remaining: "+String((float)msec_to_wait/1000.0f);
        }
        String get_type(){
            return "G4";
        }
    };
}