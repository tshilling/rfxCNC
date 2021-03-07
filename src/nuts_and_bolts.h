#pragma once
#include "Arduino.h"

#define SERIAL_SIZE_RX 1024

#define MIN_FLOAT_VALUE -3.4028235E+38
#define MAX_FLOAT_VALUE 3.4028235E+38
#define MIN_INT16_VALUE -32768
#define MAX_INT16_VALUE 32767
#define MIN_UINT16_VALUE 0
#define MAX_UINT16_VALUE 65535
#define MIN_INT32_VALUE -2147483648
#define MAX_INT32_VALUE 2147483647
#define MIN_UINT32_VALUE 0
#define MAX_UINT32_VALUE 4294967295
#define MIN_INT64_VALUE -9223372036854780000
#define MAX_INT64_VALUE 9223372036854780000
#define MIN_UINT64_VALUE 0
#define MAX_UINT64_VALUE 18446744073709600000
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

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
    enum status_enum
    {
        status_ok = 0,                                        // ok
        status_letter_not_found_with_number = 1,              // G-code words consist of a letter and a value. Letter was not found.
        status_numeric_format_invalid = 2,                    // Numeric value format is not valid or missing an expected value.
        status_system_command_not_supported = 3,              // Grbl '$' system command was not recognized or supported.
        status_negative_value_when_positive_expected = 4,     // Negative value received for an expected positive value.
        status_homing_cycle_not_enabled = 5,                  // Homing cycle is not enabled via settings.
        status_min_step_pulse_too_small = 6,                  // Minimum step pulse time must be greater than 3usec
        status_failed_to_read_settings = 7,                   // EEPROM read failed. Reset and restored to default values.
        status_command_cannnot_be_used_out_of_idle = 8,       // Grbl '$' command cannot be used unless Grbl is IDLE. Ensures smooth operation during a job.
        status_locked_out_due_to_alarm_or_jog = 9,            // G-code locked out during alarm or jog state
        status_soft_limits_enabled_without_home_enabled = 10, // Soft limits cannot be enabled without homing also enabled.
        status_max_line_length_exceeded = 11,                 // Max characters per line exceeded. Line was not processed and executed.
        status_setting_exceeds_step_rate = 12,                // (Compile Option) Grbl '$' setting value exceeds the maximum step rate supported.
        status_door_openned = 13,                             // Safety door detected as opened and door state initiated.
        status_startup_length_exceeded = 14,                  // (Grbl-Mega Only) Build info or startup line exceeded EEPROM line length limit.
        status_jog_exceeds_max_travel = 15,                   // Jog target exceeds machine travel. Command ignored.
        status_jog_contains_prohibited_code = 16,             // Jog command with no '=' or contains prohibited g-code.
        status_ramp_requires_PWM = 17,                        // Laser mode requires PWM output.
        status_unsupported_gcode_found = 20,                  // Unsupported or invalid g-code command found in block.
        status_modal_group_duplication = 21,                  // More than one g-code command from same modal group found in block.
        status_feed_rate_not_set = 22,                        // Feed rate has not yet been set or is undefined.
        status_gcode_value_must_be_integer = 23,              // G-code command in block requires an integer value.
        status_axis_word_required_by_mulitple_commands = 24,  // Two G-code commands that both require the use of the XYZ axis words were detected in the block.
        status_repeat_gcode_command = 25,                     // A G-code word was repeated in the block.
        status_axis_word_required = 26,                       // A G-code command implicitly or explicitly requires XYZ axis words in the block, but none were detected.
        status_line_number_invalid = 27,                      // N line number value is not within the valid range of 1 - 9,999,999.
        status_missing_parameter = 28,                        // A G-code command was sent, but is missing some required P or L value words in the line.
        status_coordinate_system_invalid = 29,                // Grbl supports six work coordinate systems G54-G59. G59.1, G59.2, and G59.3 are not supported.
        status_G53_requires_G0_or_G1 = 30,                    // The G53 G-code command requires either a G0 seek or G1 feed motion mode to be active. A different motion was active.
        status_unused_axis_words = 31,                        // There are unused axis words in the block and G80 motion mode cancel is active.
        status_arc_without_axis_words = 32,                   // A G2 or G3 arc was commanded but there are no XYZ axis words in the selected plane to trace the arc.
        status_motion_has_invalid_target = 33,                // The motion command has an invalid target. G2, G3, and G38.2 generates this error, if the arc is impossible to generate or if the probe target is the current position.
        status_arc_math_error = 34,                           // A G2 or G3 arc, traced with the radius definition, had a mathematical error when computing the arc geometry. Try either breaking up the arc into semi-circles or quadrants, or redefine them with the arc offset definition.
        status_arc_missing_ijk = 35,                          // A G2 or G3 arc, traced with the offset definition, is missing the IJK offset word in the selected plane to trace the arc.
        status_unused_gcode_words = 36,                       // There are unused, leftover G-code words that aren't used by any command in the block.
        status_tool_length_error = 37,                        // The G43.1 dynamic tool length offset command cannot apply an offset to an axis other than its configured axis. The Grbl default axis is the Z-axis.
        status_tool_number_invalid = 38,                      // Tool number greater than max supported value.
        status_parser_unmatched_comment_closure = 200,
        status_queue_full,
        status_empty,
        status_zero_acceleration
    };
    extern String operation_result_description[];
    float get_length(float vector[], uint8_t count);
    float Q_rsqrt(float number, uint8_t refinement);
    bool unit_vector(float vector_in[], float result_out[], uint8_t count, uint8_t refinement);
    float unit_vector_return_length(float vector_in[], std::vector<float>& result_out, uint8_t count, uint8_t refinement);
    float unit_vector_return_length(int32_t vector_in[], std::vector<float>& result_out, uint8_t count, uint8_t refinement);
    float IRAM_ATTR clipValue(float input, float min, float max);
    uint8_t index_of_max(float input[], uint8_t count);
    uint8_t index_of_max(int32_t input[], uint8_t count);
    uint8_t index_of_max(uint32_t input[], uint8_t count);
    float powf2(float input);
    // The smallest float value possible without being zero.
    // Particularlly useful for values where divide by zero could happen
    extern const float SMALLEST_FLOAT;
    //    struct keyValuePair{
    //        String key;
    //        float value;
    //    };
    enum direction_enum
    {
        backward = 0,
        forward = 1
    };
    extern const float unit_convert[5][5];

    const String unitIdentifiers[] = {"m", "cm", "mm", "ft", "in"};
    enum units_enum
    {
        units_m = 0,
        units_cm = 1,
        units_mm = 2,
        units_ft = 3,
        units_in = 4
    };
};