#pragma once
#include "Arduino.h"
#include "operations.h"
//#include "operation_controller.h"
#include <RFX_Console.h>
#include "bresenham.h"
//#include "step_engine/step_engine.h"

namespace RFX_CNC
{
    class operation_G0 : public operation_class
    {
    public:
        operation_G0(command_block *_block) : operation_class(_block)
        {
            machine_mode = MACHINE::run;
        }
        ~operation_G0()
        {
        }
        String get_log()
        {
            String result = MACHINE::get_state_log("G0");
            return result;
        }
        String get_type()
        {
            return "G0";
        }

    public:
        status_enum init(MACHINE::machine_state_class *state)
        {
            status_enum result = operation_class::init(state);
            if (result != status_ok)
                return result;
            int32_t delta_steps[config.axis.size()];

            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                delta_steps[i] = state->absolute_position_steps[i] - state->p_absolute_position_steps[i]; // state->get_absolute_steps_from_coordinates(i);
            }
            float target_velocity = config.G0_feed_rate;
            motion = new motion_class();
            result = motion->init_delta_step_move(delta_steps, 0, target_velocity, 0);

            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                state->unit_vector_of_last_move[i] = motion->unit_vector[i];
            }

            return result;
        }
    };
} // namespace RFX_CNC