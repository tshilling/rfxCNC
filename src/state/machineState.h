#pragma once
#include "Arduino.h"
#include "CNCEngineConfig.h"
#include "CNCHelpers.h"
       

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

namespace CNC_ENGINE{
    class machine_state_class{
        public:
        enum machine_mode_enum{
            locked,
            idle,
            homing,
            probing,
            running
        };
        machine_mode_enum machine_mode = idle;

        float parameter[26] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        int32_t absolute_position_steps[axisCountLimit];
        int32_t zero_offset_steps[axisCountLimit];
        float   unit_vector_of_last_move[axisCountLimit];

        uint8_t tool_index = 0;
        float   velocity_squared = 0;

        bool    is_emergency_stop = false;
        bool    is_feedhold = false;
        bool    is_active = false;
        bool    optional_stop = false;
        
        float   feed_override = 1.0f;
        bool    feed_override_allowed = true;
        float   spindle_override = 1.0f;

        enum motion_mode_enum{
            G0 = 0,         // Rapid
            G1 = 1,         // Linear
            G2 = 2,         // CW Arc
            G3 = 3,         // CCW Arc
            G38_2 = 382,    // Probe toward workpiece, stop on contact, signal error if failure
            G38_3 = 383,    // Probe toward workpiece, stop on contact
            G38_4 = 384,    // Probe away from workpiece, stop on loss of contact, signal error if failure
            G38_5 = 385,     // Probe away from workpiece, stop on loss of contact
            G80 = 80        // Cancel canned cycle
        };
        enum plane_select_enum{
            G17 = 17,
            G18 = 18,
            G19 = 19,
            plane_XY = 17,
            plane_ZX = 18,
            plane_YZ = 19
        };
        enum feed_rate_mode_enum{
            G93 = 93,   // Inverse Time Mode
            G94 = 94,    // Units per minute
            inverse_time = 93,
            units_per_minute = 94
        };
        enum state_words_enum{
            off=0,
            on=1,
            left,
            right,
            top,
            bottom,
            CW,
            CCW,
            positive,
            negative,
            exact,
            absolute,
            incremental
        };
        enum spindle_state_enum{
            M3 = 3, // CW
            M4 = 4, // CCW
            M5 = 5, // Stop
            spindle_CW = 3, // CW
            spindle_CCW = 4, // CCW
            spindle_off = 5 // Stop
        };
        enum coolant_state_enum{
            M7 = 7, // Coolant Mist
            M8 = 8, // Coolant Flood
            M9 = 9,  // Coolant Off
            cooland_mist = 7,
            coolant_flood = 9,
            coolant_off = 9
        };
        spindle_state_enum spindle_state;
        coolant_state_enum coolant_state;
        struct modal_struct{
            motion_mode_enum        motion;
            plane_select_enum       plane;
            uint8_t                 coordinate_sytem = 2; //mm
            state_words_enum        distance_mode;
            state_words_enum        arc_distance_mode;
            feed_rate_mode_enum     feed_rate_mode;
            units_enum              units;
            spindle_state_enum      spindle_state;
            coolant_state_enum      coolant_state;
            state_words_enum        cutter_radius_compentation;
            state_words_enum        cutter_length_compentation;
            state_words_enum        path_control;
        }modal;
        void init(){
            for(int i = 0; i < Config::axis_count;i++){
                absolute_position_steps[i] = 0;
                zero_offset_steps[i] = 0;
                unit_vector_of_last_move[i] = 0;
            }
            velocity_squared = 0;
            modal.motion = G0;
            modal.plane = plane_XY;
            modal.coordinate_sytem = 0;
            modal.distance_mode = absolute;
            modal.arc_distance_mode = absolute;
            modal.feed_rate_mode = units_per_minute;
            modal.units = units_mm;
            modal.spindle_state = spindle_off;
            modal.coolant_state = coolant_off;
        }
        float getVelocity(){
            float v = Q_rsqrt(velocity_squared,0);
            if(v==0)
                return 0;
            return (1.0f/v);
        }
        machine_state_class(){
            init();
        }
        uint32_t status_bits;
        bool hard_limit_enabled = true;
    };
    extern machine_state_class machine_state;
    extern machine_state_class planner_state;
    void init_machine_state();
}