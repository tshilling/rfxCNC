#pragma once
#include "Arduino.h"
#include "operations.h"
#include <RFX_Console.h>
#include "bresenham.h"
#include "step_engine/step_engine.h"

namespace RFX_CNC
{
    class G28 : public movement_class
    {
    public:
        G28()
        {
            execute_in_interrupt = false;
            is_plannable = false;
            machine_mode = MACHINE::home;
        }
        ~G28()
        {
        }
        String get_log()
        {
            String result = MACHINE::get_state_log("G28");
            return result;
        }
        String get_type()
        {
            return "G28";
        }

    private:
    public:
        long usec_in_event = 0;
        enum mode_enum
        {
            start,
            seek,
            backoff,
            fine,
            done
        } move_mode;
        uint8_t home_order_index = 0;
        char *home_order = nullptr;
        operation_result_enum init(float _parameters[])
        {
            copy_parameters_in(_parameters);
            uint8_t count = 0;
            home_order = new char[Config::axis_count];
            for (int i = 0; i < Config::axis_count; i++)
            {
                home_order[i] = -1;
                if (Config::home_order[i] >= 'A')
                {
                    if (!isinf(parameters[Config::home_order[i] - 'A']))         // Parameter mentioned
                    {
                        count++;
                        int8_t index_of_axis = Config::get_axis_index_by_id(Config::home_order[i]);
                        if (index_of_axis>=0){
                            home_order[i] = index_of_axis;
                        }
                    }
                }
            }
            if (count == 0){
                for (int i = 0; i < Config::axis_count; i++){
                    if (Config::home_order[i] >= 'A'){
                        int8_t index_of_axis = Config::get_axis_index_by_id(Config::home_order[i]);
                        if (index_of_axis>=0)
                            home_order[i] = index_of_axis;
                    }
                }
            }
            console.log("Home Order After: ");
            for (uint8_t i = 0; i < Config::axis_count; i++)
            {
                String t = String((int)home_order[i]);
                t += ", ";
                console.log(t);
            }
            console.logln();
            home_order_index = 0;
            move_mode = start;
            return success;
        }
        uint8_t axis_index = 0;
        bool execute()
        {
            MACHINE::machine_mode = MACHINE::home;
            if (move_mode == start)
            {
                copy_parameters_out();
                for(; home_order_index < Config::axis_count;home_order_index++){
                    if((home_order[home_order_index] >= 0) && (home_order[home_order_index]<255))
                        break;
                }
                if(home_order_index==Config::axis_count){
                    bresenham.is_complete = true;
                    is_complete = true;
                    STEP_ENGINE::current_line = nullptr;
                    STEP_ENGINE::current_move = nullptr;
                    MACHINE::machine_mode = MACHINE::run;
                    MACHINE::set_machine_mode();
                    return true; // ie. operation is done
                }
                axis_index = home_order[home_order_index];
                float uV[Config::axis_count];
                for (uint8_t i = 0; i < Config::axis_count; i++)
                {
                    uV[i] = 0;
                }
                // Home Z axis, 3rd axis
                uV[axis_index] = 1;
                init_infinate_move(uV, Config::axis[axis_index].home_feed_coarse);
                move_mode = seek;
            }
            else if (move_mode == seek)
            {
                if (MACHINE::critical_status_bits != 0)
                {
                    int uV[Config::axis_count];
                    for (uint8_t i = 0; i < Config::axis_count; i++)
                    {
                        uV[i] = 0;
                    }
                    uV[axis_index] = -Config::axis[axis_index].steps_per_unit * Config::axis[axis_index].home_pulloff_units;
                    init_delta_step_move(uV, 0, Config::axis[axis_index].home_feed_coarse, 0);
                    move_mode = backoff;
                    // machine_state->hard_limit_enabled = false;
                }
            }
            else if (move_mode == backoff)
            {
                if (bresenham.is_complete)
                {
                    float uV[Config::axis_count];
                    for (uint8_t i = 0; i < Config::axis_count; i++)
                    {
                        uV[i] = 0;
                    }
                    uV[axis_index] = 1;
                    init_infinate_move(uV, Config::axis[axis_index].home_feed_fine);
                    move_mode = fine;
                    //  machine_state->hard_limit_enabled = true;
                }
            }
            else if (move_mode == fine)
            {
                if (MACHINE::critical_status_bits != 0)
                {
                    move_mode = done;
                }
            }
            if (move_mode == done)
            {                                        
                bresenham.is_complete = true;
                is_complete = true;
                STEP_ENGINE::current_line = nullptr;
                STEP_ENGINE::current_move = nullptr;
                MACHINE::machine_state->absolute_position_steps[axis_index] = Config::axis[axis_index].home * Config::axis[axis_index].steps_per_unit;
                MACHINE::machine_state->zero_offset_steps[axis_index]  = Config::axis[axis_index].home * Config::axis[axis_index].steps_per_unit;
                
                bitWrite(MACHINE::home_required,axis_index,0);
                home_order_index++;
                move_mode = start;
            }
            else
            {
                STEP_ENGINE::current_line = &bresenham;
                STEP_ENGINE::current_move = this;
            }
            

            return false;
        }
    };
} // namespace CNC_ENGINE