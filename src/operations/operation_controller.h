#pragma once
#include <Arduino.h>
#include <RFX_Queue.h>
#include "..\state\machineState.h"
#include "operations.h"
#include "..\parsers\commandParser.h"
#include "..\CNCHelpers.h"
#include "G1.h"
#include "G4.h"
#include "G28.h"
#include "G92.h"
#include "M0.h"
#include "M1.h"
#include "M2.h"
#include "M3.h"
#include "M4.h"
#include "M5.h"
#include "M6.h"
#include "M7.h"
#include "M8.h"
#include "M9.h"
#include "M114.h"
/* 
        --- MODAL COMMANDS ---
        Motion Mode	G0, G1, G2, G3, G38.2, G38.3, G38.4, G38.5, G80
        Coordinate System Select	G54, G55, G56, G57, G58, G59
        Plane Select	G17, G18, G19
        Distance Mode	G90, G91
        Arc IJK Distance Mode	G91.1

        Feed Rate Mode	G93, G94
        Units Mode	G20, G21
        NOT - Cutter Radius Compensation	G40
        NOT - Tool Length Offset	G43.1, G49
        Program Mode	M0, M1, M2, M30
        Spindle State	M3, M4, M5
        Coolant State	M7, M8, M9

        --- NON MODAL ---
        G4, G10 L2, G10 L20, G28, G30, G28.1, G30.1, G53, G92, G92.1

        -- Order of operations ---
        Comment (including message)
        Set feed rate mode (G93, G94).
        Set feed rate (F).
        Set spindle speed (S).
        Select tool (T).
        Change tool (M6).
        Spindle on or off (M3, M4, M5).
        Coolant on or off (M7, M8, M9).
        Enable or disable overrides (M48, M49).
        Dwell (G4).
        Set active plane (G17, G18, G19).
        Set length units (G20, G21).
        Cutter radius compensation on or off (G40, G41, G42)
        Cutter length compensation on or off (G43, G49)
        Coordinate system selection (G54, G55, G56, G57, G58, G59, G59.1, G59.2, G59.3).
        Set path control mode (G61, G61.1, G64)
        Set distance mode (G90, G91).
        Set retract mode (G98, G99).
        Go to reference location (G28, G30) or change coordinate system data (G10) or set axis offsets (G92, G92.1, G92.2, G94).
        Perform motion (G0 to G3, G33, G73, G76, G80 to G89), as modified (possibly) by G53.
        Stop (M0, M1, M2, M30, M60).
        */
namespace RFX_CNC
{
    class operation_controller_class
    {
    private:
        float sticky_parameters[26] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    public:
        operation_class *current_operation = nullptr;
        rfx_queue<operation_class> operation_queue;
        operation_controller_class()
        {
        }
        ~operation_controller_class()
        {
        }
        void init(uint16_t queue_buffer_size)
        {
            operation_queue.resize_queue(queue_buffer_size);
            for (uint8_t i = 0; i < 26; i++)
            {
                sticky_parameters[i] = 0;
            }
        }
        void clip_speed_to_achievable(float *V0_squared, float *Vc_squared, float *Vf_squared, float *acceleration, float *distance)
        {
            *Vf_squared = MIN(abs((*V0_squared) + 2 * (*acceleration) * (*distance)), abs((*Vc_squared)));
        }
        rfx_queue<operation_class>::Node *get_motion_working_backward(rfx_queue<operation_class>::Node *start)
        {
            while (start != nullptr)
            {
                if (start->item->is_plannable)
                    return start;
                start = start->previous;
            }
            return nullptr;
        }
        rfx_queue<operation_class>::Node *get_motion_working_forward(rfx_queue<operation_class>::Node *start)
        {
            while (start != nullptr)
            {
                if (start->item->is_plannable)
                    return start;
                start = start->previous;
            }
            return nullptr;
        }
        // Execution of 'the plan' occurs in ste_engine.cpp.  There we use Vf, Vc and Vmax
        void plan()
        {
            rfx_queue<operation_class>::Node *node = operation_queue.getTailPtr();
            if (node == nullptr)
                return;
            if (node->item == nullptr)
                return;
            float Vxt = 0;
            float Vxm = 0;
            while (node->item->is_plannable)
            {
                movement_class* move = static_cast<movement_class *>(node->item);
                move->Vf2.target = Vxt;
                move->Vf2.max = Vxm;

                Vxt = move->dot_product_with_previous_segment  * MIN(abs(move->Vf2.target + (2 * move->max_acceleration * move->length_in_units)),abs(move->Vt2.target));
                Vxm = move->dot_product_with_previous_segment  * MIN(abs(move->Vf2.max + (2 * move->max_acceleration * move->length_in_units)),abs(move->Vt2.max));
            
                if (!node->previous)
                    break;
                node = node->previous;
            }
        }

        operation_result_enum add_operation_to_queue(operation_class *operation)
        {
            if(MACHINE::is_emergency_stop)
                return ESTOP;
            if (operation_queue.isFull())
                return queue_full;
            if (!operation_queue.enqueue(operation))
                return queue_full;
            if(operation_queue.isEmpty()){
                MACHINE::planner_state = new MACHINE::machine_state_class(*MACHINE::machine_state); // Perform Deep Copy?
            }
            if (operation->is_plannable)
            {
                movement_class *move = static_cast<movement_class *>(operation);
                for (uint8_t i = 0; i < Config::axis_count; i++)
                {
                    MACHINE::planner_state->unit_vector_of_last_move[i] = move->unit_vector[i];
                    MACHINE::planner_state->absolute_position_steps[i] += move->delta_steps[i];
                }
                plan();
            }
            return success;
        }
        /*
        -- Order of operations ---
        Comment (including message)
    -    Set feed rate mode (G93, G94).
    -    Set feed rate (F).
 
    -> Change tool (M6).
    -> Spindle on or off (M3, M4, M5).
    -> Coolant on or off (M7, M8, M9).
    -    Enable or disable overrides (M48, M49).
    > Dwell (G4).
    -    Set active plane (G17, G18, G19).
    -    Set length units (G20, G21).
    -    Cutter radius compensation on or off (G40, G41, G42)
    -    Cutter length compensation on or off (G43, G49)
    -    Coordinate system selection (G54, G55, G56, G57, G58, G59, G59.1, G59.2, G59.3).
    -    Set path control mode (G61, G61.1, G64)
    -    Set distance mode (G90, G91).
        Set retract mode (G98, G99).
    > Go to reference location (G28, G30) or change coordinate system data (G10) or set axis offsets (G92, G92.1, G92.2, G94).
    > Perform motion (G0 to G3, G33, G73, G76, G80 to G89), as modified (possibly) by G53.
    >  Stop (M0, M1, M2, M30, M60).

    Sitcky Parameters:  When a new REAL value is encountered, it is placed in the sticky_parameter array.
                        NAN values are never put into this array, always Real Numbers
    new_parameters:     Tracks what parameters were mentioned or set in this execution of the function. 
                        Initizlized as INFINITY (As a placeholder for, not mentioned in command), if a real value is 
                        encountered, it is stored.  If a NAN is encountered, it to is set (to NAN).  In this way we can 
                        tell if a control word is mentioned with or without a value and/or if at all by this incoming command line. 
        */

        operation_result_enum add_operation_to_queue(PARSER::command_struct *command)
        {
            if (operation_queue.isFull(16))
                return queue_full;
            if (command->parameter.size() == 0)
                return invalid_operation;

            operation_result_enum result = success;
            for (uint8_t i = 0; i < command->parameter.size(); i++)
            {
                PARSER::key_value_pair_struct pair = command->parameter[i];
                if (pair.key == "G")
                {
                    if (pair.value == 93)
                        MACHINE::planner_state->modal.feed_rate_mode = MACHINE::machine_state_class::G93;
                    if (pair.value == 94)
                        MACHINE::planner_state->modal.feed_rate_mode = MACHINE::machine_state_class::G93;
                    if (pair.value == 17)
                        MACHINE::planner_state->modal.plane = MACHINE::machine_state_class::plane_XY;
                    if (pair.value == 18)
                        MACHINE::planner_state->modal.plane = MACHINE::machine_state_class::plane_ZX;
                    if (pair.value == 19)
                        MACHINE::planner_state->modal.plane = MACHINE::machine_state_class::plane_YZ;
                    if (pair.value == 20)
                        MACHINE::planner_state->modal.units = units_in;
                    if (pair.value == 21)
                        MACHINE::planner_state->modal.units = units_mm;
                    if (pair.value == 40)
                        MACHINE::planner_state->modal.cutter_radius_compentation = MACHINE::machine_state_class::off;
                    if (pair.value == 41)
                        MACHINE::planner_state->modal.cutter_radius_compentation = MACHINE::machine_state_class::left;
                    if (pair.value == 42)
                        MACHINE::planner_state->modal.cutter_radius_compentation = MACHINE::machine_state_class::right;
                    if (pair.value == 43)
                        MACHINE::planner_state->modal.cutter_length_compentation = MACHINE::machine_state_class::negative;
                    if (pair.value == 44)
                        MACHINE::planner_state->modal.cutter_radius_compentation = MACHINE::machine_state_class::positive;
                    if (pair.value == 49)
                        MACHINE::planner_state->modal.cutter_radius_compentation = MACHINE::machine_state_class::off;
                    if (pair.value >= 54 && pair.value <= 59)
                        MACHINE::planner_state->modal.coordinate_sytem = pair.value - 54;
                    if (pair.value > 59 && pair.value < 60)
                        MACHINE::planner_state->modal.coordinate_sytem = (uint8_t)((pair.value - 54.0f) * 10.0f);
                    if (pair.value == 61)
                        MACHINE::planner_state->modal.path_control = MACHINE::machine_state_class::exact;
                    if (pair.value == 64)
                        MACHINE::planner_state->modal.path_control = MACHINE::machine_state_class::off;
                    if (pair.value == 90)
                        MACHINE::planner_state->modal.distance_mode = MACHINE::machine_state_class::absolute;
                    if (pair.value == 91)
                        MACHINE::planner_state->modal.distance_mode = MACHINE::machine_state_class::incremental;
                    if (pair.value == 0)
                        MACHINE::planner_state->modal.motion = MACHINE::machine_state_class::G0;
                    if (pair.value == 1)
                        MACHINE::planner_state->modal.motion = MACHINE::machine_state_class::G1;
                    if (pair.value == 2)
                        MACHINE::planner_state->modal.motion = MACHINE::machine_state_class::G2;
                    if (pair.value == 3)
                        MACHINE::planner_state->modal.motion = MACHINE::machine_state_class::G3;
                }
                if (pair.key == "M")
                {
                    if (pair.value == 48)
                        MACHINE::feed_override_allowed = true;
                    if (pair.value == 49)
                        MACHINE::feed_override_allowed = false;
                }
            }
            float new_parameters[26] = {INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY};

            bool coordinate_specified = false; // if there is a coordinate, that means a move was requested (maybe), we will use what ever is the current motion mode.
            // Handle parameters and get them in our standard units
            //uint32_t parameter_in_command = 0;
            for (uint8_t i = 0; i < command->parameter.size(); i++)
            {
                PARSER::key_value_pair_struct pair = command->parameter[i];
                char p = pair.key[0];
                //bitWrite(parameter_in_command,p-'A',1);
                if (p >= 'A' && p <= 'Z')
                {
                    uint8_t index = p-'A';
                    if (!isnan(pair.value))
                    {
                        switch (p)
                        {
                        // Feed is always units per minute
                        case 'A':
                            coordinate_specified = true;
                            sticky_parameters[index] = pair.value * unit_convert[MACHINE::planner_state->modal.units][Config::machine_units];
                            break;
                        case 'B':
                            coordinate_specified = true;
                            sticky_parameters[index] = pair.value * unit_convert[MACHINE::planner_state->modal.units][Config::machine_units];
                            break;
                        case 'C':
                            coordinate_specified = true;
                            sticky_parameters[index] = pair.value * unit_convert[MACHINE::planner_state->modal.units][Config::machine_units];
                            break;
                        case 'D':
                            sticky_parameters[index] = pair.value * unit_convert[MACHINE::planner_state->modal.units][Config::machine_units];
                            break;
                        case 'E':
                            if (MACHINE::planner_state->modal.feed_rate_mode == MACHINE::machine_state_class::inverse_time)
                                sticky_parameters[index] = 1.0f / pair.value; // (3)
                            else
                                sticky_parameters[index] = pair.value;
                            sticky_parameters[index] *= unit_convert[MACHINE::planner_state->modal.units][Config::machine_units];
                            break;
                        case 'F':
                            if (MACHINE::planner_state->modal.feed_rate_mode == MACHINE::machine_state_class::inverse_time)
                                sticky_parameters[index] = 1.0f / pair.value; // (3)
                            else
                                sticky_parameters[index] = pair.value;
                            sticky_parameters[index] *= unit_convert[MACHINE::planner_state->modal.units][Config::machine_units];
                            break;
                        case 'G':
                            sticky_parameters[index] = pair.value;
                            break;
                        case 'H':
                            sticky_parameters[index] = pair.value * unit_convert[MACHINE::planner_state->modal.units][Config::machine_units];
                            break;
                        case 'I':
                            sticky_parameters[index] = pair.value * unit_convert[MACHINE::planner_state->modal.units][Config::machine_units];
                            break;
                        case 'J':
                            sticky_parameters[index] = pair.value * unit_convert[MACHINE::planner_state->modal.units][Config::machine_units];
                            break;
                        case 'K':
                            sticky_parameters[index] = pair.value * unit_convert[MACHINE::planner_state->modal.units][Config::machine_units];
                            break;
                        case 'L':
                            sticky_parameters[index] = pair.value * unit_convert[MACHINE::planner_state->modal.units][Config::machine_units];
                            break;
                        case 'M':
                            sticky_parameters[index] = pair.value;
                            break;
                        case 'N':
                            sticky_parameters[index] = pair.value;
                            break;
                        case 'O':
                            sticky_parameters[index] = pair.value;
                            break;
                        case 'P':
                            sticky_parameters[index] = pair.value;
                            break;
                        case 'Q':
                            sticky_parameters[index] = pair.value;
                            break;
                        case 'R':
                            sticky_parameters[index] = pair.value * unit_convert[MACHINE::planner_state->modal.units][Config::machine_units];
                            break;
                        case 'S':
                            sticky_parameters[index] = pair.value;
                            break;
                        case 'T':
                            sticky_parameters[index] = pair.value;
                            break;
                        case 'U':
                            coordinate_specified = true;
                            sticky_parameters[index] = pair.value * unit_convert[MACHINE::planner_state->modal.units][Config::machine_units];
                            break;
                        case 'V':
                            coordinate_specified = true;
                            sticky_parameters[index] = pair.value * unit_convert[MACHINE::planner_state->modal.units][Config::machine_units];
                            break;
                        case 'W':
                            coordinate_specified = true;
                            sticky_parameters[index] = pair.value * unit_convert[MACHINE::planner_state->modal.units][Config::machine_units];
                            break;
                        case 'X':
                            coordinate_specified = true;
                            sticky_parameters[index] = pair.value * unit_convert[MACHINE::planner_state->modal.units][Config::machine_units];
                            break;
                        case 'Y':
                            coordinate_specified = true;
                            sticky_parameters[index] = pair.value * unit_convert[MACHINE::planner_state->modal.units][Config::machine_units];
                            break;
                        case 'Z':
                            coordinate_specified = true;
                            sticky_parameters[index] = pair.value * unit_convert[MACHINE::planner_state->modal.units][Config::machine_units];
                            break;
                        default:
                            break;
                        }
                        new_parameters[index] = sticky_parameters[index];
                    }
                    else
                    {
                        new_parameters[index] = NAN;
                    }
                }
            }
            
            // Change Tool
            for (uint8_t i = 0; i < command->parameter.size(); i++)
            {
                PARSER::key_value_pair_struct pair = command->parameter[i];
                if (pair.key == "M")
                {
                    if (pair.value == 6)
                    {
                        operation_class *operation = new M6;
                        operation_queue.enqueue(operation);
                    }
                }
            }
            
            // Spindle
            for (uint8_t i = 0; i < command->parameter.size(); i++)
            {
                PARSER::key_value_pair_struct pair = command->parameter[i];
                if (pair.key == "M")
                {
                    if (pair.value == 3)
                    {
                        operation_class *operation = new M3;
                        operation->init(sticky_parameters);
                        operation_queue.enqueue(operation);
                    }
                    if (pair.value == 4)
                    {
                        operation_class *operation = new M4;
                        operation->init(sticky_parameters);
                        operation_queue.enqueue(operation);
                    }
                    if (pair.value == 5)
                    {
                        operation_class *operation = new M5;
                        operation->init(sticky_parameters);
                        operation_queue.enqueue(operation);
                    }
                }
            }
            
            // Coolant
            for (uint8_t i = 0; i < command->parameter.size(); i++)
            {

                PARSER::key_value_pair_struct pair = command->parameter[i];
                if (pair.key == "M")
                {
                    if (pair.value == 7)
                    {
                        operation_class *operation = new M7;
                        operation_queue.enqueue(operation);
                    }
                    if (pair.value == 8)
                    {
                        operation_class *operation = new M8;
                        operation_queue.enqueue(operation);
                    }
                    if (pair.value == 9)
                    {
                        operation_class *operation = new M9;
                        operation_queue.enqueue(operation);
                    }
                }
            }
            // Dwell

            for (uint8_t i = 0; i < command->parameter.size(); i++)
            {

                PARSER::key_value_pair_struct pair = command->parameter[i];
                if (pair.key == "G")
                {
                    if (pair.value == 4)
                    {
                        operation_class *operation = new G4;
                        result = operation->init(sticky_parameters); // operation_queue.getTailItemPtr());
                        if (result == success)
                            result = add_operation_to_queue(operation);
                    }
                }
            }
            //> Go to reference location (G28, G30) or change coordinate system data (G10) or set axis offsets (G92, G92.1, G92.2, G94).

            for (uint8_t i = 0; i < command->parameter.size(); i++)
            {
                PARSER::key_value_pair_struct pair = command->parameter[i];
                if (pair.key == "G")
                {
                    if (pair.value == 28)
                    { // Home
                        operation_class *operation = new G28;
                        result = operation->init(new_parameters);
                        if (result == success)
                        {
                            result = add_operation_to_queue(operation);
                        }
                        coordinate_specified = false;
                    }
                    if (pair.value == 30)
                    { // Probe z axis
                    }
                    if (pair.value == 92)
                    { // Set position, will give cooridnates, but not require move
                        operation_class *operation = new G92;
                        result = operation->init(new_parameters);
                        if (result == success)
                        {
                            result = add_operation_to_queue(operation);
                        }
                        coordinate_specified = false;
                    }
                }
            }
            //> Perform motion (G0 to G3, G33, G73, G76, G80 to G89), as modified (possibly) by G53.

            if (coordinate_specified)
            {
                if (MACHINE::planner_state->modal.motion == MACHINE::machine_state_class::G0)
                {
                }
                else if (MACHINE::planner_state->modal.motion == MACHINE::machine_state_class::G1)
                {

                    operation_class *operation = new G1;
                    result = operation->init(sticky_parameters);
                    if (result == success)
                    {
                        result = add_operation_to_queue(operation);
                    }
                }
                else if (MACHINE::planner_state->modal.motion == MACHINE::machine_state_class::G2)
                {
                }
                else if (MACHINE::planner_state->modal.motion == MACHINE::machine_state_class::G3)
                {
                }
            }

            //>  Stop (M0, M1, M2, M30, M60).
            for (uint8_t i = 0; i < command->parameter.size(); i++)
            {

                PARSER::key_value_pair_struct pair = command->parameter[i];
                if (pair.key == "M")
                {
                    if (pair.value == 0)
                    {
                        M0 *operation = new M0;
                        operation_queue.enqueue(operation);
                    }
                    if (pair.value == 1)
                    {
                        M1 *operation = new M1;
                        operation_queue.enqueue(operation);
                    }
                    if (pair.value == 2 || pair.value == 30)
                    {
                        M2 *operation = new M2;
                        operation_queue.enqueue(operation);
                    }
                    if (pair.value == 114)
                    {
                        M114 *operation = new M114;
                        operation_queue.enqueue(operation);
                    }
                }
            }
            return result;
        }
    };
    extern operation_controller_class operation_controller;
} // namespace CNC_ENGINE
