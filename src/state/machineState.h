#pragma once
#include "Arduino.h"
#include <RFX_Console.h> // Common serial port out interface
#include "CNCEngineConfig.h"
#include "nuts_and_bolts.h"
#include "../operations/command_block.h"

#define _A_ 0
#define _B_ 1
#define _C_ 2
#define _D_ 3
#define _E_ 4
#define _F_ 5
#define _G_ 6
#define _H_ 7
#define _I_ 8
#define _J_ 9
#define _K_ 10
#define _L_ 11
#define _M_ 12
#define _N_ 13
#define _O_ 14
#define _P_ 15
#define _Q_ 16
#define _R_ 17
#define _S_ 18
#define _T_ 19
#define _U_ 20
#define _V_ 21
#define _W_ 22
#define _X_ 23
#define _Y_ 24
#define _Z_ 25

namespace RFX_CNC
{

    extern String modal_description[modal_enum::MAX_VALUE];

    namespace MACHINE
    {
        extern String machine_mode_description[];
        enum machine_mode_enum
        {
            locked = 0,
            door,
            need_homing,
            idle,
            run,
            hold,
            homing,
            probing,
            jogging
        };
        extern machine_mode_enum machine_mode;
        extern bool hard_limit_enabled;
        extern uint16_t home_required;
        extern bool is_emergency_stop;
        extern bool is_feedhold;
        extern bool is_active;
        extern bool optional_stop;

        extern float feed_override_squared;
        extern float spindle_override;
        extern bool spindle_override_allowed;

        extern float velocity_squared;
        extern float spindle_speed;

        extern uint32_t critical_status_bits;
        extern uint32_t critical_min_mask;
        extern uint32_t critical_max_mask;
        extern uint32_t critical_home_mask;
        extern uint32_t critical_other_mask;

        class machine_state_class
        {
        public:
            command_block block;
            //machine_state_class *previous_state = nullptr;
            std::vector<int32_t> p_absolute_position_steps;
            std::vector<int32_t> absolute_position_steps;
            std::vector<int32_t> zero_offset_steps;
            std::vector<float> unit_vector_of_last_move;
            int8_t coordinate_system_index;
            void update_state(command_block *in)
            {
                update_state(in, false);
            }
            void update_state(command_block *in, bool for_planner)
            {
                for (uint8_t i = 0; i < 26; i++)
                {
                    if (!isnan(in->parameter[i]) && !isinf(in->parameter[i]))
                    {
                        block.parameter[i] = in->parameter[i];
                    }
                }
                for (uint8_t i = 0; i < mg_max_value; i++)
                {
                    if (in->modal[i] != not_set)
                    {
                        block.modal[i] = in->modal[i];
                        if (i == mg_coordinate)
                        {
                            if ((block.modal[mg_coordinate] >= G54) && (block.modal[mg_coordinate] <= G59))
                            {
                                coordinate_system_index = block.modal[mg_coordinate] - G54; // Convert to value offset of 0
                            }
                            if (block.modal[mg_coordinate] == G92)
                            {
                                for (uint8_t i = 0; i < config.axis.size(); i++)
                                {
                                    float D = in->parameter[config.axis[i].id - 'A'];
                                    if (!isnan(D) && !isinf(D))
                                    {
                                        if (block.get_modal(mg_units) == G20) // If machine state units are "in"
                                            D *= unit_convert[units_in][units_mm];
                                        config.get_coordinate_system(G92 - G54)[i] = D + config.get_coordinate_system(coordinate_system_index)[i];
                                    }
                                }
                            }
                        }
                    }
                }
                if (!for_planner)
                    return;

                // Update Coordinates
                for (uint8_t i = 0; i < config.axis.size(); i++)
                {
                    p_absolute_position_steps[i] = absolute_position_steps[i];
                    uint8_t id = config.axis[i].id - 'A';
                    float D = in->parameter[id];
                    if (isnan(D) || isinf(D))
                        continue;

                    // Handle absolute vs relative positioning
                    if (block.get_modal(mg_distance) == G91)
                    {
                        // Get the dimension in the right units
                        if (block.get_modal(mg_units) == G20) // If machine state units are "in"
                            D = D * unit_convert[units_in][units_mm];
                        //else // Else, they must be "mm"
                        //    D = D * unit_convert[units_mm][config.machine_units];
                        int32_t steps = D * config.axis[i].steps_per_unit;
                        absolute_position_steps[i] += steps;
                    }
                    else
                    {
                        absolute_position_steps[i] = coordinates_to_steps(D, i);
                    }
                }
            }

            void print_gcode_mode()
            {
                for (uint8_t i = 0; i < 26; i++)
                {
                    console.log(String((char)(i + 'A')), 7 * (i + 1));
                }
                console.logln();
                for (uint8_t i = 0; i < 26; i++)
                {
                    console.log(String(block.parameter[i]), 7 * (i + 1));
                }
                console.logln();
                for (uint8_t i = 0; i < mg_max_value; i++)
                {
                    console.log(modal_description[block.modal[i]], 7 * (i + 1));
                }
                console.logln();
            }

            float steps_to_coordinate(int32_t steps, uint8_t axis_index)
            {
                float position = 0;
                // 1) Convert to Units
                position = ((float)steps) / ((float)config.axis[axis_index].steps_per_unit);
                // 2) Remove Offset
                position -= config.get_coordinate_system(coordinate_system_index)[axis_index] - config.get_coordinate_system(G92 - G54)[axis_index];
                ;
                // 3) Convert Units
                if (block.get_modal(mg_units) == G20) // If machine state units are "in"
                    position *= unit_convert[units_mm][units_in];
                //else // Else, they must be "mm"
                //    position *= unit_convert[config.machine_units][units_mm];
                return position;
            }
            int32_t coordinates_to_steps(float position, uint8_t axis_index)
            {
                // 1) Convert Units
                if (block.get_modal(mg_units) == G20) // If machine state units are "in"
                    position *= unit_convert[units_in][units_mm];
                //else // Else, they must be "mm"
                //    position *= unit_convert[units_mm][config.machine_units];
                // 2) Apply Offset
                position += config.get_coordinate_system(coordinate_system_index)[axis_index] - config.get_coordinate_system(G92 - G54)[axis_index];
                // 3) Convert to steps
                int32_t steps = position * config.axis[axis_index].steps_per_unit;
                return steps;
            }
            float get_feed_rate()
            {
                float result = block.parameter[_F_];
                if (result == 0) // Easy to protect from division of zero error
                    return 0;
                if (block.get_modal(mg_feed_rate) == G93) // If in inverse feed mode? invert it.
                    result = 1.0f / result;
                if (block.get_modal(mg_units) == G20) // If machine state units are "in"
                    result = result * unit_convert[units_in][units_mm];
                //else // Else, they must be "mm"
                //    result = result * unit_convert[units_mm][config.machine_units];
                return result;
            }
            bool is_feed_overried_allowed()
            {
                if (block.get_modal(mg_override) == M48)
                    return true;
                return false;
            }
            float get_spindle_speed()
            {
                float result = block.parameter[_S_];
                if (block.get_modal(mg_spindle) == M3)
                    return result;
                if (block.get_modal(mg_spindle) == M4)
                    return -result;
                return 0;
            }
            enum coolant_enum
            {
                off,
                mist,
                flood
            };
            coolant_enum get_coolant()
            {
                if (block.get_modal(mg_coolant) == M7)
                    return mist;
                if (block.get_modal(mg_coolant) == M8)
                    return flood;
                if (block.get_modal(mg_coolant) == M9)
                    return off;
            }
            uint8_t get_active_coordinate_index()
            {
                switch (block.get_modal(mg_coordinate))
                {
                case G54:
                    return 0;
                case G55:
                    return 1;
                case G56:
                    return 2;
                case G57:
                    return 3;
                case G58:
                    return 4;
                case G59:
                    return 5;
                };
                return 0;
            }

            void init()
            {
                for (uint8_t i = 0; i < 26; i++)
                {
                    block.parameter[i] = 0;
                }
                for (uint8_t i = 0; i < mg_max_value; i++)
                {
                    block.modal[i] = 0;
                }
                for (int i = 0; i < config.axis.size(); i++)
                {
                    p_absolute_position_steps[i] = 0;
                    absolute_position_steps[i] = 0;
                    zero_offset_steps[i] = 0;
                    unit_vector_of_last_move[i] = 0;
                }
                critical_status_bits = 0;
                // Load default startup state
            }

            machine_state_class()
            {
                p_absolute_position_steps.resize(config.axis.size());
                absolute_position_steps.resize(config.axis.size());
                zero_offset_steps.resize(config.axis.size());
                unit_vector_of_last_move.resize(config.axis.size());
                init();
            }
        };
        extern machine_state_class *machine_state;
        extern machine_state_class *planner_state;

        void init_machine_state();
        void update_parameters(float _parameter[]);
        void update_modals(uint8_t _modal[]);

        void scan_inputs(unsigned long delta_time);
        void handle_inputs();
        void handle_outputs();

        bool perform_emergency_stop();
        bool perform_emergency_stop(String msg);
        bool perform_unlock();
        bool perform_cycle_start();
        bool perform_feed_hold();
        void perform_stop();
        void perform_end_of_program();

        void set_machine_mode();
        String get_state_log(String pre);
        String get_state_log();
        void set_feed_override(float value);
        float get_feed_override();
        float get_feed_override_squared();
        bool set_spindle_override(float value);
        float get_spindle_override();
        void disable_all_drives();
        void enable_all_drives();
        float getVelocity();
        float getSpindle();
        void print_gcode_mode();
        void set_spindle_state(uint8_t command);
        void set_coolant_state(uint8_t command);
    } // namespace MACHINE
} // namespace RFX_CNC