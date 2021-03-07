#pragma once
#include "Arduino.h"
#include "..\parsers\commandParser.h"
#include "..\nuts_and_bolts.h"
namespace RFX_CNC
{
    enum modal_enum
    {
        not_set = 0,
        //#### Move Group ####
        G0,     // Rapid Linear Move
        G1,     // Linear Move
        G2,     // TODO Clockwise Arc
        G3,     // TODO Counter-Clockwise Arc
        G4,     // DWELL
        G38_2,  // TODO Probe toward a target and stop.  Error if not target found
        G38_3,  // TODO Probe toward a target and stop.  Don't error
        G38_4,  // TODO Probe away from target and stop.  Error if not target found
        G38_5,  // TODO Probe away from target and stop.  Don't error
        G80,    // TODO Cancel Motion mode... forget current setting

        //#### Coordinate System ####
        G54,    // WARNING, Coordinate selection must remain sequential in this list
        G55,    // This includes G92, which is a custom offset defined by the user
        G56,
        G57,
        G58,
        G59,
        G28,
        G30,    // End Coordinate Select
        
        G60,
        G17,
        G18,
        G19,   // Plane Select
        G90,   // Absolute Distance Mode
        G90_1, // Absolute Arc Distance Mode
        G91,   // Incremental Distance Mode
        G91_1, // Incremental Arc Distance Mode
        G92,
        G93,   // Inverse Feed Rate
        G94,   // Normal Feed Rate (units/sec)
        G20,   // Units in
        G21,   // Units mm
        G40,   // Cutter Compensation
        G43_1,
        G49,    // Tool Length
        G61,    // Control Mode
        M0,     // Program Stop
        M1,     // Optional Program Stop
        M2,
        M30,    // Program Flow
        M3,     // Spindle CW
        M4,     // Spindle CCW
        M5,     // Spindle Off
        M7,     // Coolant Mist
        M8,     // Coolant Flood
        M9,     // Coolant Off
        M48,    // Override enabled
        M49,    // Override disabled
        MAX_VALUE
    };
    enum modal_group_enum   // Keep this in reverse order
    {
        mg_program_flow = 0,
        mg_motion       = 1,
        // binary groups
        mg_arc,
        mg_distance,
        mg_coordinate,
        mg_tool_length,
        mg_cutter_compensation,
        mg_units,
        mg_plane,
        mg_override,
        mg_coolant,
        mg_spindle,
        mg_feed_rate,
        mg_max_value,
        mg_not_set
    };
    struct command_group_struct{
        char  letter            = 0;
        float number            = 0;
        modal_group_enum group  = mg_not_set;
    };
    extern std::vector<command_group_struct> valid_commands();
    void add_command(char letter, float number, modal_group_enum group){
        command_group_struct cgs;
        cgs.letter = letter;
        cgs.number = number;
        cgs.group = group;
    }
    class command_block
    {
    public:
        String input = "";
        String comment = "";
        float parameter[26] = {INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY};
        uint8_t modal[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        uint16_t modal_flag = 0;
        bool coordinate_flag = false;

        status_enum result;

        void log_block()
        {
            console.bar(64);
            console.logln("input: " + input + "   " + String(result));
            console.logln("comment: " + comment);
            for (uint8_t i = 0; i < 26; i++)
            {
                console.logln(String((char)(i + 'A')) + ": " + String(parameter[i]));
            }
            console.bar(64);
        }
        uint8_t get_modal(uint8_t group){
            return modal[group];
        }
        void set_modal(uint8_t group, uint8_t value){
            modal[group] = value;
        }
        command_block()
        {
            for (uint8_t i = 0; i < 26; i++)
            {
                parameter[i] = 0;
            }
            for (uint8_t i = 0; i < 12; i++)
            {
                modal[i] = 0;
            }
            result = status_ok;
        }
        command_block(PARSER::command_struct *command)
        {
            // Parameters,
            //  infinity    => Not mentioned
            //  NAN         => mentioned, but not set
            //  value       => value set
            // modals,
            //  zero        => Not mentioned
            //  value       => modal_enum
            if (command == nullptr)
            {
                result = status_ok;
                return;
            }
            if (command->parameter.size() == 0)
            {
                result = status_ok;
                return;
            }
            result = status_ok;
            input = command->input;
            comment = command->comment;
            char letter;
            float value;
            uint8_t int_value;
            uint16_t mantissa;
            modal_flag = 0;

            for (uint8_t i = 0; i < command->parameter.size(); i++)
            {
                uint16_t new_flag = 0;
                letter = command->parameter[i].key[0];
                if((letter<'A')||(letter>'Z'))
                {
                    result = status_unsupported_gcode_found;
                    return;
                }
                value = command->parameter[i].value;
                int_value = trunc(value);
                mantissa = round(100 * (value - int_value));
                if (letter == 'M')
                {
                    switch (int_value)
                    {
                    case 0:
                        bitWrite(new_flag, mg_program_flow, 1);
                        modal[mg_program_flow] = modal_enum::M0;
                        break;
                    case 1:
                        bitWrite(new_flag, mg_program_flow, 1);
                        modal[mg_program_flow] = modal_enum::M1;
                        break;
                    case 2:
                        bitWrite(new_flag, mg_program_flow, 1);
                        modal[mg_program_flow] = modal_enum::M2;
                        break;
                    case 30:
                        bitWrite(new_flag, mg_program_flow, 1);
                        modal[mg_program_flow] = modal_enum::M30;
                        break;
                    case 3:
                        bitWrite(new_flag, mg_spindle, 1);
                        modal[mg_spindle] = modal_enum::M3;
                        break;
                    case 4:
                        bitWrite(new_flag, mg_spindle, 1);
                        modal[mg_spindle] = modal_enum::M4;
                        break;
                    case 5:
                        bitWrite(new_flag, mg_spindle, 1);
                        modal[mg_spindle] = modal_enum::M5;
                        break;
                    case 7:
                        bitWrite(new_flag, mg_coolant, 1);
                        modal[mg_coolant] = modal_enum::M7;
                        break;
                    case 8:
                        bitWrite(new_flag, mg_coolant, 1);
                        modal[mg_coolant] = modal_enum::M8;
                        break;
                    case 9:
                        bitWrite(new_flag, mg_coolant, 1);
                        modal[mg_coolant] = modal_enum::M9;
                        break;
                    case 48:
                        bitWrite(new_flag, mg_override, 1);
                        modal[mg_override] = modal_enum::M48;
                        break;
                    case 49:
                        bitWrite(new_flag, mg_override, 1);
                        modal[mg_override] = modal_enum::M49;
                        break;
                    default:
                        //parameter[letter - 'A'] = value;
                        result = status_unsupported_gcode_found;
                    }
                }
                else if (letter == 'G')
                {
                    switch (int_value)
                    {
                    case 0:
                        bitWrite(new_flag, mg_motion, 1);
                        modal[mg_motion] = modal_enum::G0;
                        break;
                    case 1:
                        bitWrite(new_flag, mg_motion, 1);
                        modal[mg_motion] = modal_enum::G1;
                        break;
                    case 2:
                        bitWrite(new_flag, mg_motion, 1);
                        modal[mg_motion] = modal_enum::G2;
                        break;
                    case 3:
                        bitWrite(new_flag, mg_motion, 1);
                        modal[mg_motion] = modal_enum::G3;
                        break;
                    case 4:
                        bitWrite(new_flag, mg_motion, 1);
                        modal[mg_motion] = modal_enum::G4;
                        break;
                    case 38:
                        bitWrite(new_flag, mg_motion, 1);
                        if (mantissa == 20)
                            modal[mg_motion] = modal_enum::G38_2;
                        if (mantissa == 30)
                            modal[mg_motion] = modal_enum::G38_3;
                        if (mantissa == 40)
                            modal[mg_motion] = modal_enum::G38_4;
                        if (mantissa == 50)
                            modal[mg_motion] = modal_enum::G38_5;
                        break;
                    case 80:
                        bitWrite(new_flag, mg_motion, 1);
                        modal[mg_motion] = modal_enum::G80;
                        break;
                    case 17:
                        bitWrite(new_flag, mg_plane, 1);
                        modal[mg_plane] = modal_enum::G17;
                        break;
                    case 18:
                        bitWrite(new_flag, mg_plane, 1);
                        modal[mg_plane] = modal_enum::G18;
                        break;
                    case 19:
                        bitWrite(new_flag, mg_plane, 1);
                        modal[mg_plane] = modal_enum::G19;
                        break;
                    case 90:
                        if (mantissa == 0)
                        {
                            bitWrite(new_flag, mg_distance, 1);
                            modal[mg_distance] = modal_enum::G90;
                        }
                        if (mantissa == 10)
                        {
                            bitWrite(new_flag, mg_arc, 1);
                            modal[mg_arc] = modal_enum::G90_1;
                        }
                        break;
                    case 91:
                        if (mantissa == 0)
                        {
                            bitWrite(new_flag, mg_distance, 1);
                            modal[mg_distance] = modal_enum::G91;
                        }
                        if (mantissa == 10)
                        {
                            bitWrite(new_flag, mg_arc, 1);
                            modal[mg_arc] = modal_enum::G91_1;
                        }
                        break;
                    case 92:
                        bitWrite(new_flag, mg_coordinate, 1);
                        modal[mg_coordinate] = modal_enum::G92;
                        break;
                    case 93:
                        bitWrite(new_flag, mg_feed_rate, 1);
                        modal[mg_feed_rate] = modal_enum::G93;
                        break;
                    case 94:
                        bitWrite(new_flag, mg_feed_rate, 1);
                        modal[mg_feed_rate] = modal_enum::G94;
                        break;
                    case 20:
                        bitWrite(new_flag, mg_units, 1);
                        modal[mg_units] = modal_enum::G20;
                        break;
                    case 21:
                        bitWrite(new_flag, mg_units, 1);
                        modal[mg_units] = modal_enum::G21;
                        break;
                    case 40:
                        bitWrite(new_flag, mg_cutter_compensation, 1);
                        modal[mg_cutter_compensation] = modal_enum::G40;
                        break;
                    case 43:
                        bitWrite(new_flag, mg_tool_length, 1);
                        if (mantissa == 10)
                            modal[mg_tool_length] = modal_enum::G43_1;
                    case 49:
                        bitWrite(new_flag, mg_tool_length, 1);
                        modal[mg_tool_length] = modal_enum::G49;
                        break;
                    case 54:
                        bitWrite(new_flag, mg_coordinate, 1);
                        modal[mg_coordinate] = modal_enum::G54;
                        break;
                    case 55:
                        bitWrite(new_flag, mg_coordinate, 1);
                        modal[mg_coordinate] = modal_enum::G55;
                        break;
                    case 56:
                        bitWrite(new_flag, mg_coordinate, 1);
                        modal[mg_coordinate] = modal_enum::G56;
                        break;
                    case 57:
                        bitWrite(new_flag, mg_coordinate, 1);
                        modal[mg_coordinate] = modal_enum::G57;
                        break;
                    case 58:
                        bitWrite(new_flag, mg_coordinate, 1);
                        modal[mg_coordinate] = modal_enum::G58;
                        break;
                    case 59:
                        bitWrite(new_flag, mg_coordinate, 1);
                        modal[mg_coordinate] = modal_enum::G59;
                        break;
                    case 92:
                        bitWrite(new_flag, mg_coordinate, 1);
                        modal[mg_coordinate] = modal_enum::G92;
                        break;
                    default:
                        //parameter[letter - 'A'] = value;
                        result = status_unsupported_gcode_found;
                    }
                }
                else
                {
                    parameter[letter - 'A'] = value;
                    for(uint8_t i=0;i<config.axis.size();i++){
                        if(config.axis[i].id == letter){
                            coordinate_flag = true;
                        }
                    }
                }
                if((new_flag & modal_flag) > 0)
                {
                    result = status_modal_group_duplication;
                    return;
                }
                modal_flag = modal_flag | new_flag;
            }
        }
    };
} // namespace RFX_CNC