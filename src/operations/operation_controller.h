#pragma once
#include <Arduino.h>
#include "..\state\machineState.h"
#include "operations.h"
#include "..\parsers\commandParser.h"
#include "..\nuts_and_bolts.h"

#include "G0.h"
#include "G1.h"
#include "G4.h"
#include "G28.h"
#include "G92.h"
#include "M114.h"

#include "..\Queue.h"
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

    public:
        operation_class *current_operation = nullptr;
        Queue<operation_class*> operation_queue;
        operation_controller_class()
        {
        }
        ~operation_controller_class()
        {
        }
        void init(uint16_t queue_buffer_size)
        {
            //operation_queue.resize_queue(queue_buffer_size);
        }

        /*
    Sitcky Parameters:  When a new REAL value is encountered, it is placed in the sticky_parameter array.
                        NAN values are never put into this array, always Real Numbers
    new_parameters:     Tracks what parameters were mentioned or set in this execution of the function. 
                        Initizlized as INFINITY (As a placeholder for, not mentioned in command), if a real value is 
                        encountered, it is stored.  If a NAN is encountered, it to is set (to NAN).  In this way we can 
                        tell if a control word is mentioned with or without a value and/or if at all by this incoming command line. 
        */
        status_enum push(operation_class *operation)
        {
            //MACHINE::planner_state->previous_state = new MACHINE::machine_state_class(*MACHINE::planner_state);
            status_enum result = operation->init(MACHINE::planner_state);
            if (result == status_ok){
                if(!operation_queue.enqueue(operation)){
                    result = status_queue_full;
                }
            }
            return result;
        }
        status_enum add_operation_to_queue(PARSER::command_struct *command)
        {
            if(operation_queue.is_full())
                return status_queue_full;
            if (command->parameter.size() == 0)
                return status_ok;

            status_enum result = status_ok;
            command_block *block = new command_block(command);
            if(block->result != status_ok)
                return block->result;
 
            if (operation_queue.is_empty())
            {
                // First thing being added to operation queue.  Copy current machine state
                MACHINE::planner_state = new MACHINE::machine_state_class(*MACHINE::machine_state);
            }
            operation_class *operation = nullptr;

            //-- Order of operations ---
            //    Comment (including message)
            //    Set feed rate mode (G93, G94).
            //    Set feed rate (F).
            //    Set spindle speed (S).
            //    Select tool (T).
            //    Change tool (M6).
            //    Spindle on or off (M3, M4, M5).
            //    Coolant on or off (M7, M8, M9).
            //    Enable or disable overrides (M48, M49).
            //    Dwell (G4).
            //    Set active plane (G17, G18, G19).
            //    Set length units (G20, G21).
            //    Cutter radius compensation on or off (G40, G41, G42)
            //    Cutter length compensation on or off (G43, G49)
            //    Coordinate system selection (G54, G55, G56, G57, G58, G59, G59.1, G59.2, G59.3).
            //    Set path control mode (G61, G61.1, G64)
            //    Set distance mode (G90, G91).
            //    Set retract mode (G98, G99).
            //    Go to reference location (G28, G30) or change coordinate system data (G10) or set axis offsets (G92, G92.1, G92.2, G94).
            //    Perform motion (G0 to G3, G33, G73, G76, G80 to G89), as modified (possibly) by G53.

            if(block->modal[mg_coordinate]!=0){
                switch (block->modal[mg_coordinate])
                {
                case G92:
                    operation = new operation_G92(block);
                    break;
                default:
                    break;
                }
            }
            if (block->modal[mg_motion] != 0)
            {
                if(operation!=nullptr){
                    // Some illegal duplicate command is present
                    return status_modal_group_duplication;
                }
                switch (block->modal[mg_motion])
                {
                case G0:
                    operation = new operation_G0(block);
                    break;
                case G1:
                    operation = new operation_G1(block);
                    break;
                case G2: // TODO
                    break;
                case G3: // TODO
                    break;
                case G4: // TODO
                    operation = new operation_G4(block);
                    break;
                case G38_2: // TODO
                    break;
                case G38_3: // TODO
                    break;  // TODO
                case G38_4: // TODO
                    break;
                case G38_5: // TODO
                    break;
                case G80:
                    operation = new operation_class(block);
                    break;
                }
            }
            if (operation == nullptr)
                operation = new operation_class(block);
            result = push(operation);
            return result;
        }
    };
    extern operation_controller_class operation_controller;
} // namespace RFX_CNC
