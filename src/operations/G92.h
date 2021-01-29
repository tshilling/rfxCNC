#pragma once
#include "Arduino.h"
#include "operations.h"
namespace RFX_CNC{
    class G92:public operation_class{ 
            
        public:
        uint32_t msec_to_wait = 0;    
        G92(){
            execute_in_interrupt = false;
        }
        ~G92(){

        } 
        unsigned long last_time = 0;
        float *coordinates;
        bool execute(){   
            for(uint8_t i=0;i<Config::axis_count;i++){
                float v = coordinates[i];
                if(!isnan(v) && !isinf(v)){
                    MACHINE::machine_state->zero_offset_steps[i] = (MACHINE::machine_state->absolute_position_steps[i]) - v * Config::axis[i].steps_per_unit;
                }
            }
            is_complete = true;
            return is_complete;
        }
        operation_result_enum init(float parameters[]){
            operation_class::init(parameters);
            coordinates = new float[Config::axis_count];
            for(uint8_t i=0;i<Config::axis_count;i++){
                coordinates[i] = parameters[Config::axis[i].id-'A'];
            }
            return success;
        }
        String get_log(){
            return "";
        }
        String get_type(){
            return "G92";
        }
    };
}