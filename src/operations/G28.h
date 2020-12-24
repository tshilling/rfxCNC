#pragma once
#include "Arduino.h"
#include "operations.h"
#include "Console.h"
#include "bresenham.h"
#include "step_engine/step_engine.h"

namespace CNC_ENGINE{
    class G28:public movement_class{      
        public:     

        G28(){
            execute_in_interrupt = false;
            is_movement = true;
        }
        ~G28(){

        }
        String get_log(){
            String result = "G28- "+String(step_engine::usec_in_event)+" (usec)\t"+String(machine_state.getVelocity())+"units/sec"+"\tcoord(units):";
            for(int i = 0; i < Config::axis_count;i++){
                result += "\t"+String(((float)machine_state.absolute_position_steps[i])/((float)Config::axis[i].steps_per_unit));
            }
            return result;
        }
        String get_type(){
            return "G28";
        }
        private:

        public:
        long usec_in_event = 0;
        enum mode_enum{
            start,
            seek,
            backoff,
            fine,
            done
        }mode;
        operation_result_enum init(float parameters[], uint32_t present_flag){
            mode = start;
            float uV[Config::axis_count];
            for(uint8_t i = 0; i < Config::axis_count;i++){
                uV[i] = 0;
            }
            uV[2] = 1;
            init_infinate_move(uV,Config::axis[2].home_feed_coarse);
            
            return success;
        }
        bool execute(){   
            if(mode==start){
                float uV[Config::axis_count];
                for(uint8_t i = 0; i < Config::axis_count;i++){
                    uV[i] = 0;
                }
                uV[2] = 1;
                init_infinate_move(uV,Config::axis[2].home_feed_coarse);
                mode = seek;
            }
            else if(mode == seek){
                if(machine_state.status_bits!=0){
                    int uV[Config::axis_count];
                    for(uint8_t i = 0; i < Config::axis_count;i++){
                        uV[i] = 0;
                    }
                    uV[2] = -500;
                    init_delta_step_move(uV,0,Config::axis[2].home_feed_coarse,0);
                    mode = backoff;
                   // machine_state.hard_limit_enabled = false;
                }
            }
            else if(mode== backoff){
                if(bresenham.is_complete){
                    float uV[Config::axis_count];
                    for(uint8_t i = 0; i < Config::axis_count;i++){
                        uV[i] = 0;
                    }
                    uV[2] = 1;
                    init_infinate_move(uV,Config::axis[2].home_feed_fine);
                    mode = fine;  
                  //  machine_state.hard_limit_enabled = true;
                }
            }
            else if(mode == fine){
                if(machine_state.status_bits!=0){
                    mode = done;
                }
            }
            if(mode==done){
                bresenham.is_complete = true;
                is_complete = true;
                step_engine::current_line = nullptr;
                step_engine::current_move = nullptr;
                return true;
            }
            step_engine::current_line = &bresenham;
            step_engine::current_move = this;

            return false;
        }
    };
}