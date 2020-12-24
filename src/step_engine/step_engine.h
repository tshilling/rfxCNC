#pragma once
#ifndef step_engine_h
#define step_engine_h

#include "Arduino.h"
#include "CNCEngineConfig.h"
#include "Console.h"
#include "../operations/operations.h"

#define step_engine_buffer_size 64        
#define clock_usec_divider 10  // clock prescale is initially set to 80, which leads to a pulse every 1usec.  This divider takes 80/divider => 1usec/divider
                                // This allows for greater velocity resolution.  At 200 steps / mm, a divider of 1 and a min_usec_between_steps of 20, the following limits apply
                                // 250 mm/sec @ 20 usec_between_steps
                                // 238 mm/sec @ 21 usec_between_steps
                                // 227 mm/sec @ 22 usec_between_steps...
                                // A divider of 10 gives:
                                // 250 mm/sec @ 20 usec_between_steps
                                // 249 mm/sec @ 20.1 usec_between_steps
                                // 248 mm/sec @ 20.2 usec_between_steps
                                // 246 mm/sec @ 20.3 usec_between_steps
                                // 245 mm/sec @ 20.4 usec_between_steps
                                // ... (We do lose, 247 by int math, but that is the only one out of 250).  This is a good balance.
                                // This is applied to clock initialization and then again to usec_per_step after every other calculation is performed.  
                                // Requires those calculations to be floating point 
namespace CNC_ENGINE{
    namespace step_engine{            
        extern bool is_active;
        void init();
        extern int usec_in_event;
        extern bresenham_line_class* current_line;
        extern movement_class* current_move;
        //bresenham_return_enum add_move(int32_t steps[],float _Vi, float _Vt, float _Vf);
    };
};
#endif