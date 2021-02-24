#pragma once

#ifndef cncEngineConfig_h
#define cncEngineConfig_h
#include "Arduino.h"
#include "nuts_and_bolts.h"
#include <RFX_Console.h> // Common serial port out interface
#include "ArduinoJson.h"
#include "RFX_FILE_SYSTEM.h"

//############ Stepper Motor Timing #############
#define engineDefaultPulseOnDuration 2     // 1.9 usec
#define engineDefaultPulseOffDuration 2    // 1.9 usec
#define engineDefaultSetupTimeBeforeStep 1 // 650 nsec
#define engineDefaultWakeupTime 2          // 1.7 msec

//############# raw Step Generator ##############
#define queLength 64
#define axisCountLimit 9

namespace RFX_CNC
{
    //Pre calculated powers of 2:   0 1 2 3 4  5  6  7   8   9   10   11   12   13   14    15
    const uint16_t powersOfTwo[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};
    struct spindle_struct
    {
        float min_value = 0;
        float max_value = 100;
        bool ramp = false;
        uint8_t output_pin = -1;
        bool output_pin_invert = false;
    };
    struct axis_struct
    {
        char id = 'X';     // Symbol used to control
        char follow = 'X'; // Make this axis follow
        bool is_rotary = false;
        bool is_discrete = true;

        float min = 0;
        float max = 0;
        float home = 0;
        bool home_negative  = false;
        bool require_home   = false;
        bool is_homed       = false;

        bool hard_limit = false;
        bool soft_limit = false;

        float home_feed_fine = 1;
        float home_feed_coarse = 5;
        float home_pulloff_units = 1;

        uint16_t steps_per_unit = 200;
        float max_feed_units_per_sec = 60; // units / sec
        float acceleration = 20;           // units / sec^2

        // Pins
        int8_t dir_pin = -1;
        bool dir_pin_invert = false;
        int8_t step_pin = -1;
        bool step_pin_invert = false;
        int8_t enable_pin = -1;
        bool enable_pin_invert = false;

        int8_t limit_pin_min = -1;
        bool limit_pin_min_invert = false;
        int8_t limit_pin_max = -1;
        bool limit_pin_max_invert = false;
        int8_t home_pin = -1;
        bool home_pin_invert = false;

        int8_t limit_min_map = -1;
        int8_t limit_max_map = -1;
        int8_t home_map = -1;

        uint8_t i_follow = 0;
        float max_distance = 1000;
    };
    struct input_struct
    {
        int8_t pin = -1;
        bool invert = false;
        int32_t debounce = 0;
        bool state = 0;
    };

    class config_class
    {
    private:
    public:
        const char home_order[9] = {'Z', 'Y', 'X', 'A', 'B', 'C', 'U', 'V', 'W'}; // TODO, convert to Vector... or other solution

        //<<<<<<<<<<<<<<< USER Config File Set >>>>>>>>>>>>>>>>>

        String machine_name = "Default Name";
        units_enum machine_units = units_mm;

        uint32_t step_max_usec_between = 50000; // Interval shall never be greater than this value for step timer, (200steps/mm => 0.1mm/sec min speed, 20 Hz)
        uint32_t step_min_usec_between = 20;    // Interval shall never be lower than this value for step timer,   (200steps/mm => 240mm/sec max speed, 50 kHz)
        uint8_t step_pulse_on_usec = 10;        // usec (1-255)
        uint8_t step_idle_delay_msec = 25;      // TODO implement
        uint8_t dir_pin_settle_usec = 2;        // usec (1-255)
        float junction_deviation = 0.010;
        float arc_tolerance = 0.002;

        int8_t estop_pin = 14;
        bool estop_pin_invert = false;
        int8_t estop_map = -1;

        int8_t probe_pin = 27;
        bool probe_pin_invert = false;
        int8_t probe_map = -1;

        int8_t feed_hold_pin = -1;
        bool feed_hold_pin_invert = false;
        int8_t input_debounce_msec = 5;
        //std::vector<std::vector<float>> coordinate;
        std::vector<spindle_struct> spindle;
        std::vector<axis_struct> axis;
        std::vector<input_struct> critical_inputs;
        std::vector<std::vector<float>> coordinate;
        //<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

        uint32_t get_step_port_invert_mask()
        {
            uint32_t result = 0;
            for (uint8_t i = 0; i < axis.size(); i++)
            {
                if (axis[i].step_pin_invert)
                    bitWrite(result, i, 1);
            }
            return result;
        }
        void set_step_port_invert_mask(uint32_t in)
        {
            for (uint8_t i = 0; i < axis.size(); i++)
            {
                if (bitRead(in, i))
                {
                    axis[i].step_pin_invert = true;
                }
                else
                {
                    axis[i].step_pin_invert = false;
                }
            }
            return;
        }
        uint32_t get_dir_port_invert_mask()
        {
            uint32_t result = 0;
            for (uint8_t i = 0; i < axis.size(); i++)
            {
                if (axis[i].dir_pin_invert)
                    bitWrite(result, i, 1);
            }
            return result;
        }
        uint32_t get_step_enable_port_invert_mask()
        {
            uint32_t result = 0;
            for (uint8_t i = 0; i < axis.size(); i++)
            {
                if (axis[i].step_pin_invert)
                    bitWrite(result, i, 1);
            }
            return result;
        }
        uint32_t get_limit_pin_min_invert_mask()
        {
            uint32_t result = 0;
            for (uint8_t i = 0; i < axis.size(); i++)
            {
                if (axis[i].limit_pin_min_invert)
                    bitWrite(result, i, 1);
            }
            return result;
        }
        uint32_t get_limit_pin_max_invert_mask()
        {
            uint32_t result = 0;
            for (uint8_t i = 0; i < axis.size(); i++)
            {
                if (axis[i].limit_pin_max_invert)
                    bitWrite(result, i, 1);
            }
            return result;
        }
        uint32_t get_soft_limit_mask()
        {
            uint32_t result = 0;
            for (uint8_t i = 0; i < axis.size(); i++)
            {
                if (axis[i].soft_limit)
                    bitWrite(result, i, 1);
            }
            return result;
        }
        uint32_t get_hard_limit_mask()
        {
            uint32_t result = 0;
            for (uint8_t i = 0; i < axis.size(); i++)
            {
                if (axis[i].hard_limit)
                    bitWrite(result, i, 1);
            }
            return result;
        }
        uint32_t get_home_cycle_mask()
        {
            uint32_t result = 0;
            for (uint8_t i = 0; i < axis.size(); i++)
            {
                if (axis[i].require_home)
                    bitWrite(result, i, 1);
            }
            return result;
        }
        uint32_t get_home_dir_invert_mask()
        {
            uint32_t result = 0;
            for (uint8_t i = 0; i < axis.size(); i++)
            {
                if (axis[i].home_negative)
                    bitWrite(result, i, 1);
            }
            return result;
        }
        uint32_t get_ramp_mask()
        {
            uint32_t result = 0;
            for (uint8_t i = 0; i < spindle.size(); i++)
            {
                if (spindle[i].ramp)
                    bitWrite(result, i, 1);
            }
            return result;
        }
        float* get_coordinate_system(uint8_t index){
            if(index > coordinate.size())
                return &coordinate[0][0];
            return &coordinate[index][0];
        }
        config_class()
        {
        }
        ~config_class()
        {
        }

        bool good_config = false;

        float G0_feed_rate = 20;

        uint32_t worldToMachine(float input, uint8_t axisIndex);

        int8_t get_axis_index_by_id(char id)
        {
            for (uint8_t i = 0; i < axis.size(); i++)
            {
                if (id == axis[i].id)
                    return i;
            }
            return -1;
        }
        uint8_t add_critical_input_to_list(int8_t pin, bool invert)
        {
            if (pin < 0)
                return -1;
            // Ensure it is unique
            for (uint8_t i = 0; i < critical_inputs.size(); i++)
            {
                if (critical_inputs[i].pin == pin)
                    return i;
            }
            input_struct input;
            input.pin = pin;
            input.invert = invert;
            critical_inputs.push_back(input);
            pinMode(pin, INPUT_PULLUP);
            return critical_inputs.size() - 1;
        }
        bool init()
        {

            if (!RFX_FILE_SYSTEM::ready)
                RFX_FILE_SYSTEM::init();
            if (readConfigFile())
            {
                console.logln("Config file, \"enginejson\" opened", console.routine);
            }
            else
            {
                console.logln("Could not open config file, \"enginejson\"", console.error);
            }
            for (uint8_t i = 0; i < axis.size(); i++)
            {
                axis[i].max_distance = INT32_MAX / axis[i].steps_per_unit;
            }
            // Configure Pins

            for (uint8_t i = 0; i < axis.size(); i++)
            {
                if (axis[i].step_pin >= 0)
                    pinMode(axis[i].step_pin, OUTPUT);

                if (axis[i].dir_pin >= 0)
                    pinMode(axis[i].dir_pin, OUTPUT);

                if (axis[i].enable_pin >= 0)
                    pinMode(axis[i].enable_pin, OUTPUT);

                axis[i].limit_min_map = add_critical_input_to_list(axis[i].limit_pin_min, axis[i].limit_pin_min_invert);
                axis[i].limit_max_map = add_critical_input_to_list(axis[i].limit_pin_max, axis[i].limit_pin_max_invert);
                axis[i].home_map = add_critical_input_to_list(axis[i].home_pin, axis[i].home_pin_invert);
            }
            estop_map = add_critical_input_to_list(estop_pin, estop_pin_invert);
            probe_map = add_critical_input_to_list(probe_pin, probe_pin_invert);

            return RFX_FILE_SYSTEM::ready;
        }
        int getUnitIndex(String units)
        {
            int unitIndex = 0;
            for (int i = 0; i < 5; i++)
            {
                if (unitIdentifiers[i].equals(units))
                {
                    unitIndex = i;
                    break;
                }
            }
            return unitIndex;
        }

        bool readConfigFile()
        {
            good_config = false;
            console.tabIndex++;

            File configFile = RFX_FILE_SYSTEM::fileSystem.open("/public/engineConfig.json", "r");
            console.logln("Reading engineCongif.json");
            if (configFile)
            {
                StaticJsonDocument<16384> doc;
                DeserializationError error = deserializeJson(doc, configFile);
                if (error)
                {
                    console.logln("An Error Occurrred: " + String(error.c_str()));
                }

                if (!error)
                {
                    doc["machine_name"].isNull() ? machine_name = "none" : machine_name = String((const char *)doc["machine_name"]);
                    doc["machine_units"].isNull() ? machine_units = units_mm : machine_units = (units_enum)getUnitIndex(doc["machine_units"]);
                    doc["step_max_usec_between"].isNull() ? step_max_usec_between = 50000 : step_max_usec_between = doc["step_max_usec_between"];
                    doc["step_min_usec_between"].isNull() ? step_min_usec_between = 20 : step_min_usec_between = doc["step_min_usec_between"];
                    doc["step_pulse_on_usec"].isNull() ? step_pulse_on_usec = 10 : step_pulse_on_usec = doc["step_pulse_on_usec"];
                    doc["step_idle_delay_msec"].isNull() ? step_idle_delay_msec = 25 : step_idle_delay_msec = doc["step_idle_delay_msec"];
                    doc["dir_pin_settle_usec"].isNull() ? dir_pin_settle_usec = 2 : dir_pin_settle_usec = doc["dir_pin_settle_usec"];
                    doc["junction_deviation"].isNull() ? junction_deviation = 0.01f : junction_deviation = doc["junction_deviation"];
                    doc["arc_tolerance"].isNull() ? arc_tolerance = 0.002f : arc_tolerance = doc["arc_tolerance"];
                    doc["estop_pin"].isNull() ? estop_pin = -1 : estop_pin = doc["estop_pin"];
                    doc["estop_pin_invert"].isNull() ? estop_pin_invert = false : estop_pin_invert = doc["estop_pin_invert"];
                    doc["probe_pin"].isNull() ? probe_pin = -1 : probe_pin = doc["probe_pin"];
                    doc["probe_pin_invert"].isNull() ? probe_pin_invert = false : probe_pin_invert = doc["probe_pin_invert"];
                    doc["feed_hold_pin"].isNull() ? feed_hold_pin = -1 : feed_hold_pin = doc["feed_hold_pin"];
                    doc["feed_hold_pin_invert"].isNull() ? feed_hold_pin_invert = false : feed_hold_pin_invert = doc["feed_hold_pin_invert"];
                    doc["input_debounce_msec"].isNull() ? input_debounce_msec = 5 : input_debounce_msec= doc["input_debounce_msec"];

                    if (!doc["spindle"].isNull())
                    {
                        JsonArray spindle_json = doc["spindle"];
                        spindle.resize(spindle_json.size());
                        for (uint8_t i = 0; i < spindle.size(); i++)
                        {
                            spindle_json[i]["max_value"].isNull() ? spindle[i].max_value = 0 : spindle[i].max_value = spindle_json[i]["max_value"];
                            spindle_json[i]["min_value"].isNull() ? spindle[i].min_value = 0 : spindle[i].min_value = spindle_json[i]["min_value"];
                            spindle_json[i]["ramp"].isNull() ? spindle[i].ramp = false : spindle[i].ramp = spindle_json[i]["ramp"];
                            spindle_json[i]["output_pin"].isNull() ? spindle[i].output_pin = -1 : spindle[i].output_pin = spindle_json[i]["output_pin"];
                            spindle_json[i]["output_pin_invert"].isNull() ? spindle[i].output_pin_invert = false : spindle[i].output_pin_invert = spindle_json[i]["output_pin_invert"];
                        }
                    }
                    if (!doc["axis"].isNull())
                    {
                        JsonArray axis_json = doc["axis"];
                        axis.resize(axis_json.size());
                        for (uint8_t i = 0; i < axis.size(); i++)
                        {
                            axis_json[i]["axis_id"].isNull() ? axis[i].id = 0 : axis[i].id = ((const char *)axis_json[i]["axis_id"])[0];
                            axis_json[i]["follow"].isNull() ? axis[i].follow = 0 : axis[i].follow = ((const char *)axis_json[i]["follow"])[0];
                            axis_json[i]["is_rotary"].isNull() ? axis[i].is_rotary = false : axis[i].is_rotary = axis_json[i]["is_rotary"];
                            axis_json[i]["is_discrete"].isNull() ? axis[i].is_discrete = false : axis[i].is_discrete = axis_json[i]["is_discrete"];
                            axis_json[i]["require_home"].isNull() ? axis[i].require_home = false : axis[i].require_home = axis_json[i]["require_home"];

                            axis_json[i]["home_negative"].isNull() ? axis[i].home_negative = false : axis[i].home_negative = axis_json[i]["home_negative"];
                            axis_json[i]["hard_limit"].isNull() ? axis[i].hard_limit = false : axis[i].hard_limit = axis_json[i]["hard_limit"];
                            axis_json[i]["soft_limit"].isNull() ? axis[i].soft_limit = false : axis[i].soft_limit = axis_json[i]["soft_limit"];

                            axis_json[i]["min"].isNull() ? axis[i].min = 0 : axis[i].min = axis_json[i]["min"];
                            axis_json[i]["max"].isNull() ? axis[i].max = 0 : axis[i].max = axis_json[i]["max"];
                            axis_json[i]["home"].isNull() ? axis[i].home = 0 : axis[i].home = axis_json[i]["home"];
                            axis_json[i]["home_feed_fine"].isNull() ? axis[i].home_feed_fine = 0 : axis[i].home_feed_fine = axis_json[i]["home_feed_fine"];
                            axis_json[i]["home_feed_coarse"].isNull() ? axis[i].home_feed_coarse = 0 : axis[i].home_feed_coarse = axis_json[i]["home_feed_coarse"];
                            axis_json[i]["home_pulloff_units"].isNull() ? axis[i].home_pulloff_units = 0 : axis[i].home_pulloff_units = axis_json[i]["home_pulloff_units"];

                            axis_json[i]["steps_per_unit"].isNull() ? axis[i].steps_per_unit = 0 : axis[i].steps_per_unit = axis_json[i]["steps_per_unit"];
                            axis_json[i]["max_feed_units_per_sec"].isNull() ? axis[i].max_feed_units_per_sec = 0 : axis[i].max_feed_units_per_sec = axis_json[i]["max_feed_units_per_sec"];
                            axis_json[i]["acceleration"].isNull() ? axis[i].acceleration = 0 : axis[i].acceleration = axis_json[i]["acceleration"];

                            axis_json[i]["dir_pin"].isNull() ? axis[i].dir_pin = -1 : axis[i].dir_pin = axis_json[i]["dir_pin"];
                            axis_json[i]["dir_pin_invert"].isNull() ? axis[i].dir_pin_invert = false : axis[i].dir_pin_invert = axis_json[i]["dir_pin_invert"];
                            axis_json[i]["step_pin"].isNull() ? axis[i].step_pin = -1 : axis[i].step_pin = axis_json[i]["step_pin"];
                            axis_json[i]["step_pin_invert"].isNull() ? axis[i].step_pin_invert = false : axis[i].step_pin_invert = axis_json[i]["step_pin_invert"];
                            axis_json[i]["enable_pin"].isNull() ? axis[i].enable_pin = -1 : axis[i].enable_pin = axis_json[i]["enable_pin"];
                            axis_json[i]["enable_pin_invert"].isNull() ? axis[i].enable_pin_invert = false : axis[i].enable_pin_invert = axis_json[i]["enable_pin_invert"];

                            axis_json[i]["limit_pin_min"].isNull() ? axis[i].limit_pin_min = -1 : axis[i].limit_pin_min = axis_json[i]["limit_pin_min"];
                            axis_json[i]["limit_pin_min_invert"].isNull() ? axis[i].limit_pin_min_invert = false : axis[i].limit_pin_min_invert = axis_json[i]["limit_pin_min_invert"];
                            axis_json[i]["limit_pin_max"].isNull() ? axis[i].limit_pin_max = -1 : axis[i].limit_pin_max = axis_json[i]["limit_pin_max"];
                            axis_json[i]["limit_pin_max_invert"].isNull() ? axis[i].limit_pin_max_invert = false : axis[i].limit_pin_max_invert = axis_json[i]["limit_pin_max_invert"];
                            axis_json[i]["home_pin"].isNull() ? axis[i].home_pin = -1 : axis[i].home_pin = axis_json[i]["home_pin"];
                            axis_json[i]["home_pin_invert"].isNull() ? axis[i].home_pin_invert = false : axis[i].home_pin_invert = axis_json[i]["home_pin_invert"];
                        }
                    }
                    // BEWARE, this must be done after reading in axis, we need the number of axis to ensure we do not have array access violations later
                    if (!doc["coordinate"].isNull())
                    {
                        JsonArray coordinate_json = doc["coordinate"];
                        coordinate.resize(coordinate_json.size());
                        for(uint8_t i = 0; i < coordinate.size();i++){
                            // Initialize the size of the coordinate array to the number of axis available.
                            // Below, if there are fewer coordinates specified than their are axis, fill the rest with zero. 
                            coordinate[i].resize(axis.size());
                            for(uint8_t j = 0; j < coordinate[i].size();j++){
                                if(j>=coordinate_json[i].size())
                                {
                                    coordinate[i][j] = 0;
                                    continue;
                                }
                                coordinate[i][j] = coordinate_json[i][j];
                            }
                        }
                    }
                }
                // Perfomr unit conversion.  Make sure everything on the machine is stored in mm....
                for (uint8_t i = 0; i < axis.size(); i++)
                {
                    axis[i].min *= unit_convert[machine_units][units_mm];
                    axis[i].max *= unit_convert[machine_units][units_mm];
                    axis[i].home *= unit_convert[machine_units][units_mm];
                    axis[i].home_pulloff_units *= unit_convert[machine_units][units_mm];
                    axis[i].max_feed_units_per_sec *= unit_convert[machine_units][units_mm];
                    axis[i].steps_per_unit *= unit_convert[machine_units][units_mm];
                    axis[i].acceleration *= unit_convert[machine_units][units_mm];
                 }
                arc_tolerance *= unit_convert[machine_units][units_mm];
                junction_deviation *= unit_convert[machine_units][units_mm];
            }
            else
            {
                console.logln("Config File not Found", console.error);
            }
            configFile.close();

            console.tabIndex--;
            good_config = true;

            return true;
        }
        bool writeConfigFile()
        {
            /*
            File configFile = RFX_FILE_SYSTEM::fileSystem.open("/public/enginejson", "w");
            if (configFile)
            {
                DynamicJsonDocument doc(2048);
                doc["machine_name"] = machine_name;
                doc["machine_units"] = unitIdentifiers[(uint8_t)machine_units];
                JsonArray axis_array = doc.createNestedArray("axis");
                //JsonArray axis_array = doc.to<JsonArray>();
                for (uint8_t i = 0; i < axis.size(); i++)
                {
                    JsonObject axis = axis_array.createNestedObject();
                    axis["axis_id"] = String(axis[i].id);
                    axis["follow"] = String(axis[i].follow);
                    axis["is_rotary"] = axis[i].is_rotary;
                    axis["is_discrete"] = axis[i].is_discrete;
                    axis["min"] = axis[i].min * unit_convert[units_mm][machine_units];
                    axis["max"] = axis[i].max * unit_convert[units_mm][machine_units];
                    ;
                    axis["home"] = axis[i].home * unit_convert[units_mm][machine_units];
                    ;
                    axis["home_negative"] = axis[i].home_negative;
                    axis["hard_limit"] = axis[i].hard_limit;
                    axis["soft_limit"] = axis[i].soft_limit;
                    axis["home_feed_fine"] = axis[i].home_feed_fine * unit_convert[units_mm][machine_units];
                    ;
                    axis["home_feed_coarse"] = axis[i].home_feed_coarse * unit_convert[units_mm][machine_units];
                    ;
                    axis["home_switch_debounce_msec"] = axis[i].home_switch_debounce_msec;
                    axis["home_pulloff_units"] = axis[i].home_pulloff_units;
                    axis["steps_per_unit"] = axis[i].steps_per_unit * unit_convert[units_mm][machine_units];
                    ;
                    axis["max_feed_units_per_sec"] = axis[i].max_feed_units_per_sec * unit_convert[units_mm][machine_units];
                    ;
                    axis["acceleration"] = axis[i].acceleration * unit_convert[units_mm][machine_units];
                    ;
                    axis["dir_pin"] = axis[i].dir_pin;
                    axis["dir_pin_invert"] = axis[i].dir_pin_invert;
                    axis["step_pin"] = axis[i].step_pin;
                    axis["step_pin_invert"] = axis[i].step_pin_invert;
                    axis["enable_pin"] = axis[i].enable_pin;
                    axis["enable_pin_invert"] = axis[i].enable_pin_invert;
                    axis["home_pin"] = axis[i].home_pin;
                    axis["home_pin_invert"] = axis[i].home_pin_invert;
                    axis["limit_pin_min"] = axis[i].limit_pin_min;
                    axis["limit_pin_min_invert"] = axis[i].limit_pin_min_invert;
                    axis["limit_pin_max"] = axis[i].limit_pin_max;
                    axis["limit_pin_max_invert"] = axis[i].limit_pin_max_invert;
                }
                if (serializeJsonPretty(doc, configFile) == 0)
                {
                    console.logln("Failed to write engine config file", console.error);

                    configFile.close();
                    return false;
                }
                configFile.close();
                return true;
            }
            configFile.close();
            */
            return false;
        }
    };
    extern config_class config;
};

#endif