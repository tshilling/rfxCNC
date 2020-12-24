#pragma once

#ifndef cncEngineConfig_h
#define cncEngineConfig_h
#include "Arduino.h"
#include "CNCHelpers.h"

//############ Stepper Motor Timing #############
#define engineDefaultPulseOnDuration        2   // 1.9 usec
#define engineDefaultPulseOffDuration       2   // 1.9 usec
#define engineDefaultSetupTimeBeforeStep    1   // 650 nsec
#define engineDefaultWakeupTime             2   // 1.7 msec

//############# raw Step Generator ##############
#define queLength 64
#define axisCountLimit 9

//###################################################
//############ Constants (Do not Change) ############
//###################################################

//Pre calculated powers of 2:   0 1 2 3 4  5  6  7   8   9   10   11   12   13   14    15
const uint16_t powersOfTwo[] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};

namespace CNC_ENGINE{
    namespace Config
    {
        //########### Constants ############
        const char axis_default_id[]    = {'X','Y','Z','A','B','C','U','V','W'};

        //########### Step Engine Settings ############
        struct step_engine_struct{
            uint64_t max_usec_between_steps     = 50000;    // Interval shall never be greater than this value for step timer, (200steps/mm => 0.1mm/sec min speed, 20 Hz)
            uint64_t min_usec_between_steps     = 20;       // Interval shall never be lower than this value for step timer,   (200steps/mm => 240mm/sec max speed, 50 kHz) 
            uint8_t  usec_pulse_on              = 10;       // usec (1-255)
            uint8_t  usec_direction_pin_settle  = 2;        // usec (1-255)
        };
        extern step_engine_struct step_engine_config;
        extern bool good_config;
     
        extern String machine_name;
        extern units_enum machine_units;   
        extern u_char axis_count;    

        struct axisStruct{
            char        id                      = 'X';      // Symbol used to control
            char        follow                  = 'X';      // Make this axis follow
            bool        is_rotary               = false;
            bool        is_discrete             = true;

            float       min                     = 0;
            float       max                     = 0;
            float       home                    = 0;
            bool        home_negative            = false;

            bool        hard_limit              = false;
            bool        soft_limit              = false;

            float       home_feed_fine          = 1;
            float       home_feed_coarse        = 5;
            uint8_t     home_switch_debounce_msec = 5;
            float       home_pulloff_units      = 1;

            uint16_t    steps_per_unit          = 200; 
            float       max_feed_units_per_sec  = 60;   // units / sec
            float       acceleration            = 20;   // units / sec^2
            
            // Pins
            int8_t     dir_pin                 = -1;
            bool        dir_pin_invert          = false;
            int8_t     step_pin                = -1;
            bool        step_pin_invert         = false;
            int8_t     enable_pin              = -1;
            bool        enable_pin_invert       = false;

            int8_t     limit_pin_min           = -1;
            bool        limit_pin_min_invert    = false;
            int8_t     limit_pin_max           = -1;
            bool        limit_pin_max_invert    = false;
            int8_t     home_pin                = -1;
            bool        home_pin_invert         = false;


            // s_ prefix indicates these should be in steps
            uint16_t    s_maxFeed       = 12000;// steps / sec
            uint16_t    s_acceleration  = 4000; // steps / sec^2
            uint16_t    s_jerk          = 2000; // steps / sec^3
            int32_t     s_min           = 0;  // Min is at 0 steps
            int32_t     s_max           = 0;
            int32_t     s_home          = 0;
            uint8_t     i_follow        = 0;
            uint32_t    max_distance    = 0;
        };
        extern axisStruct axis[axisCountLimit];
        uint32_t worldToMachine(float input, uint8_t axisIndex);
        bool readConfigFile();
        bool writeConfigFile();
        bool init();
    }
};

#endif