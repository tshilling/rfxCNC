#pragma once
#include "Arduino.h"
#include "bresenham.h"
#include "..\nuts_and_bolts.h"
namespace RFX_CNC
{
    class motion_class
    {
    public:
        bool is_complete = false; 
        std::vector<int32_t> absolute_steps;
        std::vector<float> unit_vector;
        struct v_struct
        {
            float target = 0;
            float max = 0;
        };
        v_struct V02;
        v_struct Vt2;
        v_struct Vf2;

        float target_velocity = 0;
        float length_in_units = 1;
        float max_acceleration = 0;
        float dot_product_with_previous_segment = 1;
        float sec_to_usec_multiplied_by_unit_vector = 0;
        float acceleration_factor_times_two = 0;

        bresenham_line_class bresenham;
        motion_class()
        {
            absolute_steps.resize(config.axis.size());
            unit_vector.resize(config.axis.size());
        }
        ~motion_class()
        {
        }
        void compute_velocity_components()
        {
            // Find Junction angle
            dot_product_with_previous_segment = 0;
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                dot_product_with_previous_segment += MACHINE::planner_state->unit_vector_of_last_move[i] * unit_vector[i];
            }
            if (dot_product_with_previous_segment < 0)
                dot_product_with_previous_segment = 0;

            acceleration_factor_times_two = 2.0f * (max_acceleration) / ((float)(config.axis[0].steps_per_unit << bresenham.smoothing));
            sec_to_usec_multiplied_by_unit_vector = 1000000.0f / ((config.axis[bresenham.index_of_dominate_axis].steps_per_unit << bresenham.smoothing) * abs(unit_vector[bresenham.index_of_dominate_axis]));
        }
        status_enum compute_max_mechanics()
        {
            // Find limit feedrate based on configuration
            float Vmax = infinityf();
            max_acceleration = infinityf();
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                // Actual max velocity the axis allows
                Vmax =
                    MIN(
                        Vmax,
                        config.axis[i].max_feed_units_per_sec / abs(unit_vector[i]));
                // Actual max accel the axis allows
                max_acceleration =
                    MIN(
                        max_acceleration,
                        config.axis[i].acceleration / abs(unit_vector[i]));
            }
            Vt2.max = powf2(Vmax);
            return status_ok;
        }
        int8_t compute_smoothing()
        {
            int usec = 1000000.0f / (target_velocity * abs(unit_vector[bresenham.index_of_dominate_axis]) * config.axis[bresenham.index_of_dominate_axis].steps_per_unit);
            if (usec < config.step_min_usec_between)
                return -1;
            uint8_t smoothing = 0;
            for (smoothing = 0; smoothing < 8; smoothing++)
            {
                // Limit smoothing based on min_usec_between_steps, this limits it in a way that we can see a feed over ride of 200% (...steps<<1)
                if (usec >> smoothing < (config.step_min_usec_between << 1))
                    break;
                // Limit based on largest value storeable by int32
                if (bresenham.delta_steps[bresenham.index_of_max_axis] << smoothing >= (INT32_MAX >> 1))
                    break;
            }
            return smoothing;
        }
        status_enum init_infinate_move(float _unit_vector[], float Vt)
        {
            int32_t steps[config.axis.size()];
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                steps[i] = (_unit_vector[i] * 1000.0f);
            }
            status_enum result = init_delta_step_move(steps, 0, Vt, Vt);
            bresenham.is_infinate = true;
            return result;
        }
        status_enum init_delta_step_move(int32_t steps[], float V0, float Vt, float Vf)
        {
            status_enum result = status_ok;
            int32_t delta_steps[config.axis.size()];
            bresenham.is_infinate = false;
            console.logln("Vt: "+String(Vt));
            if (Vt <= 0)
            {
                return status_feed_rate_not_set;
            }
            target_velocity = Vt;
            Vt2.target = powf2(Vt);

            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                delta_steps[i] = steps[i];
            }
            length_in_units = unit_vector_return_length(delta_steps, unit_vector, config.axis.size(), 1);
            if (length_in_units == 0)
                return status_empty;
            bresenham.init(delta_steps);
            int8_t smoothing = compute_smoothing();
            if (smoothing < 0){
                //result = max_velocity_exceeded;
                smoothing = 0;
            }
            bresenham.smooth(smoothing);
            compute_max_mechanics();
            compute_velocity_components();
            if (acceleration_factor_times_two == 0)
                result = status_zero_acceleration;
            return result;
        }
    };
} // namespace RFX_CNC