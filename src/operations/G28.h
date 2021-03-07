#pragma once
#include "Arduino.h"
#include "operations.h"
#include <RFX_Console.h>

namespace RFX_CNC
{
    class operation_G28 : public operation_class
    {
    public:
        operation_G28(command_block* _block):operation_class(_block)
        {

            machine_mode = MACHINE::homing;
        }
        ~operation_G28()
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
        status_enum init(MACHINE::machine_state_class *state)
        {
            operation_class::init(state);
            uint8_t count = 0;
            home_order = new char[config.axis.size()];
            for (int i = 0; i < config.axis.size(); i++)
            {
                home_order[i] = -1;
                if (config.home_order[i] >= 'A')
                {
                    if (!isinf(state->block.parameter[config.home_order[i] - 'A'])) // Parameter mentioned
                    {
                        count++;
                        int8_t index_of_axis = config.get_axis_index_by_id(config.home_order[i]);
                        if (index_of_axis >= 0)
                        {
                            home_order[i] = index_of_axis;
                        }
                    }
                }
            }
            if (count == 0)
            {
                for (int i = 0; i < config.axis.size(); i++)
                {
                    if (config.home_order[i] >= 'A')
                    {
                        int8_t index_of_axis = config.get_axis_index_by_id(config.home_order[i]);
                        if (index_of_axis >= 0)
                            home_order[i] = index_of_axis;
                    }
                }
            }
            console.log("Home Order After: ");
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                String t = String((int)home_order[i]);
                t += ", ";
                console.log(t);
            }
            console.logln();
            home_order_index = 0;
            move_mode = start;
            return status_ok;
        }
        uint8_t axis_index = 0;
        bool execute(MACHINE::machine_state_class *state)
        {
            /*
            MACHINE::machine_mode = MACHINE::home;
            if (move_mode == start)
            {
                if (pass_count == 1)
                {
                    movement_class::execute(state);
                }
                for (; home_order_index < config.axis.size(); home_order_index++)
                {
                    if ((home_order[home_order_index] >= 0) && (home_order[home_order_index] < 255))
                        break;
                }
                if (home_order_index == config.axis.size())
                {
                    bresenham.is_complete = true;
                    is_complete = true;
                    STEP_ENGINE::current_line = nullptr;
                    STEP_ENGINE::current_move = nullptr;
                    MACHINE::machine_mode = MACHINE::run;
                    MACHINE::set_machine_mode();
                    return true; // ie. operation is done
                }
                axis_index = home_order[home_order_index];
                float uV[config.axis.size()];
                for (uint8_t i = 0; i < config.axis.size(); i++)
                {
                    uV[i] = 0;
                }
                // Home Z axis, 3rd axis
                uV[axis_index] = 1;
                init_infinate_move(uV, config.axis[axis_index].home_feed_coarse);
                move_mode = seek;
            }
            else if (move_mode == seek)
            {
                if (MACHINE::critical_status_bits != 0)
                {
                    int uV[config.axis.size()];
                    for (uint8_t i = 0; i < config.axis.size(); i++)
                    {
                        uV[i] = 0;
                    }
                    uV[axis_index] = -config.axis[axis_index].steps_per_unit * config.axis[axis_index].home_pulloff_units;
                    init_delta_step_move(uV, 0, config.axis[axis_index].home_feed_coarse, 0);
                    move_mode = backoff;
                    // machine_state->hard_limit_enabled = false;
                }
            }
            else if (move_mode == backoff)
            {
                if (bresenham.is_complete)
                {
                    float uV[config.axis.size()];
                    for (uint8_t i = 0; i < config.axis.size(); i++)
                    {
                        uV[i] = 0;
                    }
                    uV[axis_index] = 1;
                    init_infinate_move(uV, config.axis[axis_index].home_feed_fine);
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
                MACHINE::machine_state->absolute_position_steps[axis_index] = config.axis[axis_index].home * config.axis[axis_index].steps_per_unit;
                MACHINE::machine_state->zero_offset_steps[axis_index] = config.axis[axis_index].home * config.axis[axis_index].steps_per_unit;

                bitWrite(MACHINE::home_required, axis_index, 0);
                home_order_index++;
                move_mode = start;
            }
            else
            {
                STEP_ENGINE::current_line = &bresenham;
                STEP_ENGINE::current_move = this;
            }
*/
            return false;
        }
    };
} // namespace RFX_CNC