#pragma once
#include "Arduino.h"
#include "../state/machineState.h"
#include "../CNCEngineConfig.h"
//#include "operation_controller.h"
#include "../CNCHelpers.h"
#include "bresenham.h"
#include <RFX_Console.h>

namespace CNC_ENGINE{
    enum operation_result_enum{
        success = 0,
        zero_length,
        zero_velocity,
        max_distance_exceeded,
        max_velocity_exceeded,
        invalid_operation,
        queue_full,
        unrecognized_command
    };
    extern const char* operation_result_string[];

    class operation_class{
        public:
        bool execute_in_interrupt = false;
        bool is_movement = false;
        bool is_active = false;
        bool is_complete = false;  
        bool first_pass = true;
        String comment = "";

        // Why are these here you ask?  Unit vector will return all zero for non movement, which will make the junction speed zero
        // which in turn will result in a zero velocity.
        int32_t absolute_steps[axisCountLimit];
        int32_t delta_steps[axisCountLimit];
        float unit_vector[axisCountLimit];

        virtual bool execute(){
            first_pass = false;
            is_complete = true;
            return is_complete;
        }
        virtual float my_test(){
            return 1.0f;
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
        virtual float get_V0_squared(){
            return 0;
        }
        virtual float get_Vf_squared(){
            return 0;
        }
        virtual float get_Vc_squared(){
            return 0;
        }
        virtual void set_V0_squared(float value){
            
        }
        virtual void set_Vf_squared(float value){
            
        }
        virtual void set_Vc_squared(){

        }
        virtual void set_Vc_squared(float value){
            
        }
        virtual void clip_velocity_kinematically(direction_enum dir){

        }
        virtual float get_dot_product_with_previous_segment(){
            return 0;
        }
        virtual operation_result_enum init(float parameters[]){
            for(uint8_t i = 0; i < Config::axis_count;i++){
                absolute_steps[i] = planner_state.absolute_position_steps[i];// parameters[Config::axis[i].id-'A'] * Config::axis[i].stepsPerUnit;
            }
            return success;
        }
        virtual operation_result_enum init(float parameters[], uint32_t present_flag){
            return init(parameters);
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
        float    V0_squared = 0;
        float    Vc_squared = 0;
        float    Vf_squared = 0;
        float    Vi_squared = 0;
        float    Vmax_squared = 0;        
        float    target_velocity = 0;
        float    length_in_units = 1;  
        float    max_acceleration = 0;
        float    dot_product_with_previous_segment = 1;                  
        float    sec_to_usec_multiplied_by_unit_vector = 0;
        float    acceleration_factor_times_two = 0;

        bresenham_line_class bresenham;
        movement_class(){   
            is_movement = true;
        }
        float get_V0_squared(){
            return V0_squared;
        }
        float get_Vf_squared(){
            return Vf_squared;
        }
        float get_Vc_squared(){
            return Vc_squared;
        }
        void set_V0_squared(float value){
            V0_squared = value;
        }
        void set_Vf_squared(float value){
            Vf_squared = value;            
        }
        void set_Vc_squared(float value){
            Vc_squared = value;            
        }
        void set_Vc_squared(){
            Vc_squared = MIN(powf2(target_velocity*machine_state.feed_override),Vmax_squared);
        }
        float get_dot_product_with_previous_segment(){
            return dot_product_with_previous_segment;
        }
        void clip_velocity_kinematically(direction_enum dir){
            if(dir == backward)
                V0_squared = MIN(abs(Vf_squared + (2 * max_acceleration * length_in_units)),abs(Vc_squared));
            if(dir == forward)
                Vf_squared = MIN(abs(V0_squared + (2 * max_acceleration * length_in_units)),abs(Vc_squared));
        }
        void compute_velocity_components(){
            // Find Junction angle
            dot_product_with_previous_segment = 0;
            //if(previous_move!=nullptr){
                for(uint8_t i = 0;i<Config::axis_count;i++){
                    dot_product_with_previous_segment += planner_state.unit_vector_of_last_move[i]*unit_vector[i];
                }
                if(dot_product_with_previous_segment < 0)
                    dot_product_with_previous_segment = 0;
            //}
            acceleration_factor_times_two = 2.0f*(max_acceleration) / ((float)(Config::axis[0].steps_per_unit<<bresenham.smoothing));
            sec_to_usec_multiplied_by_unit_vector = 1000000.0f/((Config::axis[bresenham.index_of_dominate_axis].steps_per_unit<<bresenham.smoothing) * abs(unit_vector[bresenham.index_of_dominate_axis]));
        }
        operation_result_enum compute_max_mechanics(){
            // Find limit feedrate based on configuration
            Vmax_squared = MAX_FLOAT_VALUE;
            max_acceleration = MAX_FLOAT_VALUE;
            for(uint8_t i = 0; i < Config::axis_count;i++){
                float v = Config::axis[i].max_feed_units_per_sec / abs(unit_vector[i]);
                float a = Config::axis[i].acceleration / abs(unit_vector[i]);
                if(v < Vmax_squared)
                    Vmax_squared = v;
                if(a < max_acceleration)
                    max_acceleration = a;
            }
            Vmax_squared = Vmax_squared*Vmax_squared;
            
            return success;
        }
        int8_t compute_smoothing(){            
            int usec = 1000000.0f / (target_velocity * abs(unit_vector[bresenham.index_of_dominate_axis]) * Config::axis[bresenham.index_of_dominate_axis].steps_per_unit);  
            if(usec < Config::step_engine_config.min_usec_between_steps)
                return -1;
            uint8_t smoothing = 0;
            for(smoothing=0;smoothing<8;smoothing++){
                // Limit smoothing based on min_usec_between_steps
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
            Vc_squared = powf2(Vt);
            V0_squared = powf2(V0);
            Vf_squared = powf2(Vf);
            Vi_squared = V0_squared;

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
            operation_result_enum result;
            if((result = compute_max_mechanics())!=success)
                return result;         
            compute_velocity_components();
            if(acceleration_factor_times_two == 0)
                return invalid_operation;
            return success;
        }
    };
}