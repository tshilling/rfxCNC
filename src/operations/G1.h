#pragma once
#include "Arduino.h"
#include "operations.h"
#include "operation_controller.h"
#include <RFX_Console.h>
#include "bresenham.h"
#include "step_engine/step_engine.h"

namespace CNC_ENGINE{
    class G1:public movement_class{      
        public:     

        G1(){
            execute_in_interrupt = true;
            is_movement = true;
        }
        ~G1(){

        }
        String get_log(){
            String result = "G1- "+String(step_engine::usec_in_event)+" (usec)\t"+String(machine_state.getVelocity())+"units/sec"+"\tcoord(units):";
            for(int i = 0; i < Config::axis_count;i++){
                result += "\t"+String(((float)machine_state.absolute_position_steps[i])/((float)Config::axis[i].steps_per_unit));
            }
            return result;
        }
        String get_type(){
            return "G1";
        }

        public:
        long usec_in_event = 0;
        
        operation_result_enum init(float parameters[], uint32_t present_flag){
            return init(parameters);
        }
        operation_result_enum init(float parameters[]){
            for(uint8_t i = 0; i < Config::axis_count;i++){
                absolute_steps[i] = parameters[Config::axis[i].id-'A'] * Config::axis[i].steps_per_unit;
            }
            target_velocity = parameters[_F_];

            int32_t* previous_move_absolute_steps = &planner_state.absolute_position_steps[0];
            for(uint8_t i = 0; i < Config::axis_count;i++){
                delta_steps[i] = absolute_steps[i] - previous_move_absolute_steps[i];
                //delta_units[i] = delta_steps[i] / Config::axis[i].steps_per_unit;
            }
            return init_delta_step_move(delta_steps,0,target_velocity,0);
        }
    };
}