#pragma once
#include "Arduino.h"
#include "bresenham.h"
#include "..\nuts_and_bolts.h"
namespace RFX_CNC
{   
    /*
    Equations of Motion:
    1) X = X0 + V0*t
    2) V = V0 + a*t
    3) X = X0 + V0*t + 0.5*a* t^2
    4) V^2 = V0^2 + 2*a*dx

    Thoughts behind design:
    The ISR that controls steps has a variable delta t that changes with the velocity of the system.  
    By using dx, which is constant as every ISR firing coorisponds to one step, we can more quickly handle velocity changes

    */
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
        v_struct Vt2;
        v_struct Vf2;

        float target_velocity = 0;
        float length_in_units = 1;
        float max_acceleration = 0;
        float dot_product_with_previous_segment = 1;
        float sec_to_usec_multiplied_by_unit_vector = 0;
        float acceleration_per_step = 0;

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

                                        //  2.0f * a(mm/sec^2) * delta_mm     delta_mm = (1 step/ISR) / (steps/mm)
                                        // This doesn't use unit_vector because we are maximizing our acceleration based on each axis, not some global acceleration... 
            acceleration_per_step = 2.0f * (max_acceleration) / ((float)(config.axis[bresenham.index_of_dominate_axis].steps_per_unit << bresenham.smoothing));
                                        // 1,000,000 (usec/sec) / (steps/mm) = usec/step
                                        // multiply the above factor with the current 1/V (mm/sec) value to get usec/step 
            sec_to_usec_multiplied_by_unit_vector = 1000000.0f / ((config.axis[bresenham.index_of_dominate_axis].steps_per_unit << bresenham.smoothing) * abs(unit_vector[bresenham.index_of_dominate_axis]));
        }

        status_enum compute_max_mechanics()
        {
            // Find limit feedrate based on configuration
            float max_velocity = infinityf();
            max_acceleration = infinityf();
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                // Actual max velocity the axis allows
                max_velocity =
                    MIN(
                        max_velocity,
                        config.axis[i].max_feed_units_per_sec / abs(unit_vector[i]));
                // Actual max accel the axis allows
                max_acceleration =
                    MIN(
                        max_acceleration,
                        config.axis[i].acceleration / abs(unit_vector[i]));
            }
            Vt2.max = powf2(max_velocity);
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
            if (Vt <= 0)
            {
                return status_feed_rate_not_set;
            }
            target_velocity = Vt;
            Vt2.target = powf2(Vt);

            // Calculate unit vector
            length_in_units = 0;
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                delta_steps[i] = steps[i];
                if(config.axis[i].is_kinematic)
                    length_in_units+=delta_steps[i]*delta_steps[i];
            }
            if(length_in_units==0)
                return status_empty;
            length_in_units = Q_rsqrt(length_in_units,1);
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                if(config.axis[i].is_kinematic)
                    unit_vector[i] = delta_steps[i]*length_in_units;
                else
                    unit_vector[i] = 1;
            }
            length_in_units = 1.0f/length_in_units;

            bresenham.init(delta_steps);
            int8_t smoothing = compute_smoothing();
            if (smoothing < 0)
                smoothing = 0;
                
            bresenham.smooth(smoothing);
            compute_max_mechanics();
            compute_velocity_components();
            if (acceleration_per_step == 0)
                result = status_zero_acceleration;
            ;
            return result;
        }
    };
} // namespace RFX_CNC