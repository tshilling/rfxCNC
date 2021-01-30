#pragma once
#include "Arduino.h"
#include "../state/machineState.h"
#include "../CNCEngineConfig.h"
#include "../CNCHelpers.h"
#include "bresenham.h"
#include <RFX_Console.h>

namespace RFX_CNC{
    extern String operation_result_description[];
    enum operation_result_enum{
        success = 0,
        zero_length,
        zero_velocity,
        max_distance_exceeded,
        max_velocity_exceeded,
        invalid_operation,
        queue_full,
        unrecognized_command,
        ESTOP
    };
    class operation_class{
        public:
        float parameters[26];
        bool execute_in_interrupt = false;
        bool is_plannable = false;
        bool is_active = false;
        bool is_complete = false;  
        bool first_pass = true;
        MACHINE::machine_mode_enum machine_mode = MACHINE::run;
        String comment = "";

        // Why are these here you ask?  Unit vector will return all zero for non movement, which will make the junction speed zero
        // which in turn will result in a zero velocity.
        int32_t absolute_steps[axisCountLimit];
        int32_t delta_steps[axisCountLimit];
        float unit_vector[axisCountLimit];
        void copy_parameters_out(){
            for(uint8_t i = 0; i < 26;i++){
                if(!isnan(parameters[i]) && !isinf(parameters[i]))
                    MACHINE::machine_state->parameter[i] = parameters[i]; 
            }
        }
        void copy_parameters_in(float _parameters[]){      
            for(uint8_t i = 0; i < 26;i++){
                parameters[i] = _parameters[i];
            }
        }
        virtual bool execute(){
            copy_parameters_out();
            first_pass = false;
            is_complete = true;
            return is_complete;
        }
        operation_class(){
            for(uint8_t i = 0; i < Config::axis_count;i++){
                delta_steps[i] = 0;
                absolute_steps[i] = 0;
                unit_vector[i] = 0;
            }
        }
        virtual ~operation_class(){

        }

        virtual operation_result_enum init(float _parameters[]){
            for(uint8_t i = 0; i < Config::axis_count;i++){
                absolute_steps[i] = MACHINE::planner_state->absolute_position_steps[i];// parameters[Config::axis[i].id-'A'] * Config::axis[i].stepsPerUnit;
            }      
            copy_parameters_in(_parameters);
            return success;
        }
        virtual String get_type(){
            return "null";
        }
        virtual String get_log(){
            return "Empty";
        }
        virtual operation_class* get_new(){
            return new operation_class;
        }
        operation_class get_copy(){
            return *this;
        }
    };
    class movement_class : public operation_class{   

        public:
        struct v_struct{
            float target = 0;
            float max = 0;
        };
        v_struct V02;
        v_struct Vt2;
        v_struct Vf2;

        float    target_velocity = 0;
        float    length_in_units = 1;  
        float    max_acceleration = 0;
        float    dot_product_with_previous_segment = 1;                  
        float    sec_to_usec_multiplied_by_unit_vector = 0;
        float    acceleration_factor_times_two = 0;

        bresenham_line_class bresenham;
        movement_class(){   
            is_plannable = true;
        }

        void compute_velocity_components(){
            // Find Junction angle
            dot_product_with_previous_segment = 0;
            for(uint8_t i = 0;i<Config::axis_count;i++){
                dot_product_with_previous_segment += MACHINE::planner_state->unit_vector_of_last_move[i]*unit_vector[i];
            }
            if(dot_product_with_previous_segment < 0)
                dot_product_with_previous_segment = 0;

            acceleration_factor_times_two = 2.0f*(max_acceleration) / ((float)(Config::axis[0].steps_per_unit<<bresenham.smoothing));
            sec_to_usec_multiplied_by_unit_vector = 1000000.0f/((Config::axis[bresenham.index_of_dominate_axis].steps_per_unit<<bresenham.smoothing) * abs(unit_vector[bresenham.index_of_dominate_axis]));
        }
        operation_result_enum compute_max_mechanics(){
            // Find limit feedrate based on configuration
            float Vmax = infinityf();
            max_acceleration = infinityf();
            for(uint8_t i = 0; i < Config::axis_count;i++){
                // Actual max velocity the axis allows
                Vmax =  
                    MIN(
                        Vmax,
                        Config::axis[i].max_feed_units_per_sec / abs(unit_vector[i])
                    );    
                // Actual max accel the axis allows              
                max_acceleration = 
                    MIN(
                        max_acceleration,
                        Config::axis[i].acceleration / abs(unit_vector[i])
                    );           
            }            
            Vt2.max = powf2(Vmax);
            return success;
        }
        int8_t compute_smoothing(){            
            int usec = 1000000.0f / (target_velocity * abs(unit_vector[bresenham.index_of_dominate_axis]) * Config::axis[bresenham.index_of_dominate_axis].steps_per_unit);  
            if(usec < Config::step_engine_config.min_usec_between_steps)
                return -1;
            uint8_t smoothing = 0;
            for(smoothing=0;smoothing<8;smoothing++){
                // Limit smoothing based on min_usec_between_steps, this limits it in a way that we can see a feed over ride of 200% (...steps<<1)
                if(usec>>smoothing < (Config::step_engine_config.min_usec_between_steps<<1))
                    break;
                // Limit based on largest value storeable by int32
                if(bresenham.delta_steps[bresenham.index_of_max_axis]<<smoothing >= (INT32_MAX>>1))
                    break;
            } 
            return smoothing;
        }
        operation_result_enum init_infinate_move(float _unit_vector[], float Vt){
            int32_t steps[Config::axis_count];
            for(uint8_t i = 0; i < Config::axis_count;i++){
                steps[i] = (_unit_vector[i] * 1000.0f);
            }
            operation_result_enum result = init_delta_step_move(steps, 0, Vt, Vt);
            bresenham.is_infinate = true;
            return result;
        }
        operation_result_enum init_delta_step_move(int32_t steps[], float V0, float Vt, float Vf){
            bresenham.is_infinate = false;
            if(Vt<=0){
                return zero_velocity;
            }
            target_velocity = Vt;
            Vt2.target = powf2(Vt);

            for(uint8_t i = 0; i < Config::axis_count;i++){
                delta_steps[i] = steps[i];
            }
            length_in_units = unit_vector_return_length(delta_steps,unit_vector,Config::axis_count,1);
            if(length_in_units == 0)
                return zero_length;
            bresenham.init(delta_steps);
            int8_t smoothing = compute_smoothing();
            if(smoothing < 0)
                return max_velocity_exceeded;
            bresenham.smooth(smoothing);
            compute_max_mechanics();        
            compute_velocity_components();
            if(acceleration_factor_times_two == 0)
                return invalid_operation;
            return success;
        }
    };
}