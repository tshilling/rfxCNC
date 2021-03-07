#pragma once
#include "Arduino.h"
#include "operations.h"
#include <RFX_Console.h>

namespace RFX_CNC{
    class operation_G1 : public operation_class{      
        public:     

        operation_G1(command_block* _block):operation_class(_block){
            machine_mode = MACHINE::run;
        }
        ~operation_G1(){

        }
        String get_log(){
            String result = MACHINE::get_state_log("G1");
            return result;
        }
        String get_type(){
            return "G1";
        }

        public:
        //long usec_in_event = 0;

        status_enum init(MACHINE::machine_state_class* state){    
            status_enum result = operation_class::init(state);
            if(result!=status_ok)
                return result;
            int32_t delta_steps[config.axis.size()];
            
             for(uint8_t i = 0; i < config.axis.size();i++){
                delta_steps[i] = state->absolute_position_steps[i] - state->p_absolute_position_steps[i];// state->get_absolute_steps_from_coordinates(i);
                //if(state->previous_state!=nullptr)
                //    delta_steps[i] -= state->previous_state->get_absolute_steps_from_coordinates(i);             
            }
            float target_velocity = state->get_feed_rate()/60.0f;
            motion = new motion_class();
            result = motion->init_delta_step_move(delta_steps,0,target_velocity,0);
            for(uint8_t i = 0; i<config.axis.size();i++){
                state->unit_vector_of_last_move[i] = motion->unit_vector[i];
            }
            return result;
        }
    };
}