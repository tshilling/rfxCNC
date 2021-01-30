#include "step_engine.h"
#include "state\machineState.h"
#include "operations\operation_controller.h"

namespace RFX_CNC
{
    namespace STEP_ENGINE
    {
        /* Controls the state of the interrupt.  Interrupt, once started always fires at max_usec_between_steps. 
    * If is_active = false, no step or buffer manipulation is performed.  If true the buffer will be checked and
    * if there is a bresenham in the queue, that one will be executed.  If not, is_active will be returned to false.
    */
        bool is_active = false;

        volatile uint32_t usec_between_steps = 1000;
        hw_timer_t *step_pins_on_timer = NULL;
        hw_timer_t *step_pins_off_timer = NULL;
        portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
        enum timer_mode_enum
        {
            edge_rise,
            edge_fall,
        };
        volatile timer_mode_enum timer_mode = edge_rise;
        volatile float fire_interrupt_in_usec = Config::step_engine_config.max_usec_between_steps;
        bool set_direction_pins = true;
        bool block_timer_execution = false;
        movement_class *current_move;
        bresenham_line_class *current_line;

        /* takes in a bresenham struct pointer and increments it.  Again, using an IRAM based function
    * To allow for safe interrupt based operations.
    * Bresenham's line algorithm is a line drawing algorithm that determines the points of an n-dimensional 
    * raster that should be selected in order to form a close approximation to a straight line between two points. 
    * It is commonly used to draw line primitives in a bitmap image (e.g. on a computer screen), as it uses only 
    * integer addition, subtraction and bit shifting, all of which are very cheap operations in standard computer 
    * architectures. It is an incremental error algorithm.
    */
        bool IRAM_ATTR step_bresenham(bresenham_line_class *bresenham)
        {
            for (uint_fast8_t i = 0; i < 9; i++)
            {
                if (bresenham->D[i] >= 0)
                {
                    if (bresenham->double_delta[bresenham->index_of_dominate_axis] == 0)
                        continue;
                    uint32_t count;
                    if (bresenham->double_delta[bresenham->index_of_dominate_axis] != 0)
                    {
                        i == bresenham->index_of_dominate_axis ? count = 1 : count = 1 + bresenham->D[i] / bresenham->double_delta[bresenham->index_of_dominate_axis];

                        bresenham->D[i] -= count * bresenham->double_delta[bresenham->index_of_dominate_axis];
                        bresenham->smoothing_count[i] += count;

                        int16_t subCount = bresenham->smoothing_count[i] >> bresenham->smoothing; // Divide the smoothing count and get just the integer divisions,
                        bresenham->smoothing_count[i] -= subCount << bresenham->smoothing;        // ie. (int)(current_motion->delta_step_parameter[i].smoothing_count / 2^smoothing)
                        if (!bresenham->is_infinate)
                            bresenham->delta_steps[i] -= count;
                        MACHINE::machine_state->absolute_position_steps[i] += subCount * bresenham->direction[i];
                        if (subCount > 0)
                        {
                            if (Config::axis[i].step_pin >= 0)
                                digitalWrite(Config::axis[i].step_pin, !Config::axis[i].step_pin_invert);
                        }
                    }
                }
                bresenham->D[i] += bresenham->double_delta[i];
            }
            bresenham->is_complete = (bresenham->delta_steps[bresenham->index_of_dominate_axis] <= 0);
            return bresenham->is_complete;
        }

        /* (https://makezine.com/2008/12/04/quakes-fast-inverse-square-roo/)
    Quick way to approximate a inverse sqrt:
    1) Divide number by 2, its floating point so bitshift wont work = xhalf
    2) Take a reasonable guess at a 1/sqrt value = g
    3) refine it (As many times as desired) using: g = g*(1.5 – xhalf * g *g)

    The trick is getting that initial guess. The game engine coders use trick with how floating point numbers are represented in binary, 
    with the exponent and mantissa broken up similar to scientific notation. In a 32-bit float, the left-most bit is a sign bit and is 0 
    for positive numbers. That’s followed by 8 bits of exponent (biased by 127 to represent negative and positive exponents), and the final 
    23 bits represent the mantissa. Well, to do an inverse square root, you basically need to multiply the exponent by -1/2. When you shift 
    those bits to the right (a very fast operation), the effect on the exponent is to divide it by 2. You still need to subtract the exponent 
    from 0 to change it’s sign, though, and what do you do about the mantissa which was also affected in the bit shift operation?

    This is where the magic 0x5f3759df number comes in. It’s absolutely bonkers, but by subtracting the bit shift result from 0x5f3759df, 
    the mantissa is reset to near to it’s original state and the exponent is subtracted from 0 (taking into account it’s bias of 127). The result 
    is very close to the inverse square root. Close enough for a single pass through Newton’s equation to end up with a value precise enough for 
    practical purposes.
    */
        float IRAM_ATTR Q_rsqrt(float number, uint8_t refinement)
        {
            // A union is very special, it allows the foat and long to exist in the same bytes.  It is the same 1s and 0s and we can access them as either. This is the bit copy needed.
            union
            {
                float f;
                unsigned long i;
            } conv = {.f = number};
            conv.i = 0x5f3759df - (conv.i >> 1); // Make initial guess for sqrt.  Magic numbers get you close
            for (uint8_t i = 0; i <= refinement; i++)
            {
                float half_of_number = number * 0.5F;
                conv.f *= (1.5f - (half_of_number * conv.f * conv.f)); // Perform one Newton step to refine
            }
            return conv.f;
        }

        void IRAM_ATTR handle_step_pin_on_timer()
        {
            // This check is required because of how bresenham's work.  Each time the function is called
            // we will step even with 0 velocity because are timer is set to a max time
            if(MACHINE::velocity_squared > 0){
                current_move->is_complete = step_bresenham(current_line);
            }
            if (current_move->acceleration_factor_times_two != 0)
            {
                float V2 = MACHINE::velocity_squared;
                if(MACHINE::is_feedhold){
                    // If feedhold is active, reduce the velocity to zero to smoothly decel, no mater what we are doing
                    V2 -= current_move->acceleration_factor_times_two;
                }
                else{
                    float Vf2 = MIN(current_move->Vf2.target*MACHINE::feed_override_squared,current_move->Vf2.max);
                    float Vt2 = MIN(current_move->Vt2.target*MACHINE::feed_override_squared,current_move->Vt2.max);
                    if (current_line->delta_steps[current_line->index_of_dominate_axis] <= (V2 - Vf2) / (current_move->acceleration_factor_times_two))
                        V2 -= current_move->acceleration_factor_times_two;
                    else if (V2 < Vt2)
                        V2 += current_move->acceleration_factor_times_two;
                }

                // Hard velocity limit based on configuration of axis mechanical limits
                //if (V > current_move->Vt2.max)
                //    V = current_move->Vt2.max;

                if (V2 < 0.0001)
                    V2 = 0; //SMALLEST_FLOAT; // Smallest Value not zero
                MACHINE::velocity_squared = V2;
                if (MACHINE::velocity_squared == 0)
                    usec_between_steps = Config::step_engine_config.max_usec_between_steps;
                else
                    usec_between_steps = Q_rsqrt(MAX(MACHINE::velocity_squared, 0), 0) * current_move->sec_to_usec_multiplied_by_unit_vector;
                if (usec_between_steps < Config::step_engine_config.min_usec_between_steps)
                    usec_between_steps = Config::step_engine_config.min_usec_between_steps;
            }
            fire_interrupt_in_usec = usec_between_steps;
            return;
        }
        int32_t last_delta_time = 1;
        void IRAM_ATTR write_on_step_pin_timer(float usec)
        {
            usec = clipValue(usec, Config::step_engine_config.usec_direction_pin_settle, Config::step_engine_config.max_usec_between_steps);
            float r = usec * clock_usec_divider;
            last_delta_time = (int32_t)r;
            timerAlarmWrite(step_pins_on_timer, r, true);
        }
        void IRAM_ATTR on_step_pin_off_timer()
        {
            portENTER_CRITICAL_ISR(&timerMux);
            for (uint8_t i = 0; i < Config::axis_count; i++)
            {
                if (Config::axis[i].step_pin >= 0)
                    digitalWrite(Config::axis[i].step_pin, Config::axis[i].step_pin_invert);
            }
            //is_active = !rolling_fifo::is_empty();
            portEXIT_CRITICAL_ISR(&timerMux);
        }
        int usec_in_event;
        bool limit_switch_active = false;
        int32_t *debounce;
        uint32_t *debounce_usec;
        void IRAM_ATTR on_step_pin_on_timer()
        {
            if (block_timer_execution)
                return;
            portENTER_CRITICAL_ISR(&timerMux);

            if (MACHINE::machine_mode >= MACHINE::run)
            {
                // Enable floating point sub processor and make a copy of the existing registry
                //<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>
                xthal_set_cpenable(1);
                uint32_t cp0_regs[18];
                xthal_save_cp0(cp0_regs);

                block_timer_execution = true;
                fire_interrupt_in_usec = Config::step_engine_config.max_usec_between_steps;
                if (current_line == nullptr)
                {
                    operation_class *operation = operation_controller.operation_queue.getHeadItemPtr();
                    if (operation)
                    {
                        if (operation->execute_in_interrupt)
                        {
                            current_move = static_cast<movement_class *>(operation);
                            current_line = &(current_move->bresenham);
                        }
                    }
                }
                long s = micros();

                if (current_line != nullptr)
                {
                    if (set_direction_pins)
                    {
                        for (uint8_t i = 0; i < Config::axis_count; i++)
                        {
                            if (Config::axis[i].dir_pin >= 0)
                                digitalWrite(Config::axis[i].dir_pin, (current_line->direction[i] > 0) != Config::axis[i].dir_pin_invert);
                        }
                        fire_interrupt_in_usec = Config::step_engine_config.usec_direction_pin_settle;
                        set_direction_pins = false;
                    }
                    else
                    {
                        bool safe_to_continue = true;
                        for (uint8_t i = 0; i < Config::axis_count; i++)
                        {
                            if (current_line->direction[i] < 0)
                            {
                                // Handle min
                                if (bitRead(MACHINE::critical_status_bits, i * 3)) // Status bits are [min, max, home]
                                    safe_to_continue = false;
                            }
                            else if (current_line->direction[i] > 0)
                            {
                                // Handle max
                                if (bitRead(MACHINE::critical_status_bits, (i * 3) + 1))
                                    safe_to_continue = false;
                            }
                        }
                        if (safe_to_continue)
                        {
                            handle_step_pin_on_timer();
                            if (current_line->is_complete)
                            {
                                current_line = nullptr;
                                if (current_move->execute_in_interrupt)
                                {
                                    operation_controller.operation_queue.dequeue();
                                }
                                set_direction_pins = true;
                            }
                            timerAlarmEnable(step_pins_off_timer); // !!!!!!! esp32-hal-timer.c, function needs IRAM_ATTR attribute added to timerAlarmEnable.  If ESP crashes after updates, ensure IRAM_ATTR still there  !!!!!!!
                            usec_in_event = micros() - s;
                        }
                    }
                }
                write_on_step_pin_timer(fire_interrupt_in_usec);
                block_timer_execution = false;

                // Restore floating point sub processor registry to original state
                xthal_restore_cp0(cp0_regs);
                //<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>
            }
            portEXIT_CRITICAL_ISR(&timerMux);
        }
        void init()
        {
            debounce = new int32_t[Config::axis_count * 3];
            for (uint8_t i = 0; i < Config::axis_count * 3; i++)
            {
                debounce[i] = 0;
            }
            debounce_usec = new uint32_t[Config::axis_count];
            for (uint8_t i = 0; i < Config::axis_count; i++)
            {
                debounce_usec[i] = Config::axis[i].home_switch_debounce_msec * 1000;
            }

            step_pins_on_timer = timerBegin(0, 80 / clock_usec_divider, true);
            timerAttachInterrupt(step_pins_on_timer, &on_step_pin_on_timer, true);
            write_on_step_pin_timer(Config::step_engine_config.max_usec_between_steps);
            timerAlarmEnable(step_pins_on_timer);

            step_pins_off_timer = timerBegin(1, 80, true);
            timerAttachInterrupt(step_pins_off_timer, &on_step_pin_off_timer, true);
            timerWrite(step_pins_off_timer, 20);

            set_direction_pins = true;
            block_timer_execution = false;
        }
        //bresenham_return_enum add_move(int32_t steps[],float _Vi, float _Vt, float _Vf){
        //    return rolling_fifo::enqueue(steps,_Vi,_Vt,_Vf);
        //}
    }; // namespace step_engine
};     // namespace CNC_ENGINE