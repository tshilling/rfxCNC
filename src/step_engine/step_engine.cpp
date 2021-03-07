
#include "step_engine.h"
#include "state\machineState.h"
#include "operations\operation_controller.h"

namespace RFX_CNC
{
    /*
    uint32_t step_max_usec_between = 50000; // Interval shall never be greater than this value for step timer, (200steps/mm => 0.1mm/sec min speed, 20 Hz)
    uint32_t step_min_usec_between = 20;    // Interval shall never be lower than this value for step timer,   (200steps/mm => 240mm/sec max speed, 50 kHz)
    uint8_t step_pulse_on_usec = 10;        // usec (1-255)
    uint8_t step_idle_delay_msec = 25;      // TODO implement
    uint8_t dir_pin_settle_usec = 2;        // usec (1-255)

    config.axis.size()
    step_pin
    step_pin_invert
    dir_pin
    dir_pin_invert
    */
    namespace STEP_ENGINE
    {
        //uint32_t* status_bits_min_limit = nullptr;
        //uint32_t* status_bits_max_limit = nullptr;
        //uint32_t* status_bits_home = nullptr;
        //uint32_t* status_bits_probe = nullptr;
        //uint32_t* critical_bits = nullptr;

        float IRAM_ATTR handle_step_event();
        void IRAM_ATTR step_pin_on_timer_event();
        void IRAM_ATTR step_pin_off_timer_event();

#ifdef STEP_ENGINE_movement_buffer_size
        Queue<motion_class *> motion_queue(STEP_ENGINE_movement_buffer_size);
#else
        Queue<motion_class *> motion_queue(64);
#endif

        void plan()
        {
            float Vxt = 0;
            float Vxm = 0;
            // Walk the queue backwards.  Set the end speed of the last move to 0 (full stop) and figure out how fast we can get.
            for (uint8_t i = 0; i < motion_queue.count(); i++)
            {
                motion_class *move = motion_queue.tail(i);
                move->Vf2.target = Vxt;
                move->Vf2.max = Vxm;
                Vxt = move->dot_product_with_previous_segment * MIN(abs(move->Vf2.target + (2 * move->max_acceleration * move->length_in_units)), abs(move->Vt2.target));
                Vxm = move->dot_product_with_previous_segment * MIN(abs(move->Vf2.max + (2 * move->max_acceleration * move->length_in_units)), abs(move->Vt2.max));
            }
        }
        bool push(motion_class *motion)
        {
            bool result = motion_queue.enqueue(motion);
            if (result)
                plan();
            return result;
        }

        inline void step_bresenham(bresenham_line_class *bresenham)
        {
            /* takes in a bresenham struct pointer and increments it.  Again, using an IRAM based function
            * To allow for safe interrupt based operations.
            * Bresenham's line algorithm is a line drawing algorithm that determines the points of an n-dimensional 
            * raster that should be selected in order to form a close approximation to a straight line between two points. 
            * It is commonly used to draw line primitives in a bitmap image (e.g. on a computer screen), as it uses only 
            * integer addition, subtraction and bit shifting, all of which are very cheap operations in standard computer 
            * architectures. It is an incremental error algorithm.
            * I have modified it to allow for axis that are not discrete.  These are handled via the subCount
            */
            for (uint_fast8_t i = 0; i < config.axis.size(); i++)
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
                            if (config.axis[i].step_pin >= 0)
                                digitalWrite(config.axis[i].step_pin, !config.axis[i].step_pin_invert);
                        }
                    }
                }
                bresenham->D[i] += bresenham->double_delta[i];
            }
            if (bresenham->delta_steps[bresenham->index_of_dominate_axis] <= 0)
            {
                bresenham->state = bresenham_line_class::complete;
            }
        }

        inline float Q_rsqrt(float number, uint8_t refinement)
        {
            /* (https://makezine.com/2008/12/04/quakes-fast-inverse-square-roo/)
            Quick way to approximate a inverse sqrt:
            1) Divide number by 2, its floating point so bitshift wont work = xhalf
            2) Take a reasonable guess at a 1/sqrt value = g
            3) refine it (As many times as desired) using: g = g*(1.5 – xhalf * g *g)

            The trick is getting that initial guess. The game engine coders use a trick with how floating point numbers are represented in binary, 
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

        hw_timer_t *step_pins_on_timer = NULL;
        hw_timer_t *step_pins_off_timer = NULL;
        portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
        void IRAM_ATTR step_pin_on_timer_event()
        {
            portENTER_CRITICAL_ISR(&timerMux);
            // ESP32 has a floating point sub processor (It kind of sucks actually), but, if you
            // try and use it in an interrupt, you will corupt any "in process" floaitng math.
            // Here we enable it, make a copy of whatever is in the regestries.  We will copy the original
            // values back in after the ISR is done

            // Enable floating point sub processor and make a copy of the existing registry
            xthal_set_cpenable(1);
            uint32_t cp0_regs[18];
            xthal_save_cp0(cp0_regs);

            timerAlarmWrite(step_pins_on_timer,
                            clipValue(
                                handle_step_event(),
                                config.step_min_usec_between,
                                config.step_max_usec_between) *
                                clock_usec_divider,
                            true);

            // Restore floating point sub processor registry to original state
            xthal_restore_cp0(cp0_regs);
            portEXIT_CRITICAL_ISR(&timerMux);
        }
        void IRAM_ATTR step_pin_off_timer_event()
        {
            portENTER_CRITICAL_ISR(&timerMux);
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                if (config.axis[i].step_pin >= 0)
                    digitalWrite(config.axis[i].step_pin, config.axis[i].step_pin_invert);
            }
            portEXIT_CRITICAL_ISR(&timerMux);
        }
        float IRAM_ATTR handle_step_event()
        {
            float fire_interrupt_in_usec = config.step_max_usec_between;
            float internal_timer_adjust = 0;

            // ESTOP extra check, Its that important.
            if (config.critical_inputs[config.estop_map].state)
                return fire_interrupt_in_usec;

            if (MACHINE::machine_mode < MACHINE::run)
                return fire_interrupt_in_usec;

            if (motion_queue.count() == 0)
                return fire_interrupt_in_usec;
            motion_class *motion = motion_queue.head();
            bresenham_line_class *current_line = &(motion->bresenham);

            // If not valid, return
            if (!current_line)
                return fire_interrupt_in_usec; 

            // If this is the first pass for motion, then set direction bits and delay for 2us to allow them to settle.
            if (current_line->state == bresenham_line_class::first)
            {
                current_line->state = bresenham_line_class::running;
                for (uint8_t i = 0; i < config.axis.size(); i++)
                {
                    if (config.axis[i].dir_pin >= 0)
                        digitalWrite(config.axis[i].dir_pin, (current_line->direction[i] > 0) != config.axis[i].dir_pin_invert);
                }
                ets_delay_us(config.dir_pin_settle_usec);
                internal_timer_adjust = -config.dir_pin_settle_usec;
            }
            // This checks limit pins against requested direction.  Returns if there is a violstion
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                if (current_line->direction[i] < 0)
                {
                    // Handle min
                    // if (bitRead(MACHINE::critical_status_bits, i * 3)) // Status bits are [min, max, home]
                    if (config.critical_inputs[config.axis[i].limit_min_map].state)
                        return fire_interrupt_in_usec;
                }
                else if (current_line->direction[i] > 0)
                {
                    // Handle max
                    if (config.critical_inputs[config.axis[i].limit_max_map].state) //bitRead(MACHINE::critical_status_bits, (i * 3) + 1))
                        return fire_interrupt_in_usec;
                }
            }

            // This check is required because of how bresenham's work.  Each time the function is called
            // we will step even with 0 velocity because the timer is set to a max time
            if (MACHINE::velocity_squared > 0.00001) // floating point has 6-7 significant digits
            {
                step_bresenham(current_line);
                // If we get this far, then we have performed a step, Setup step off timer to bring step pin off at some time in the future
                timerAlarmEnable(step_pins_off_timer); // !!!!!!! WARNING esp32-hal-timer.c, function needs IRAM_ATTR attribute added to timerAlarmEnable.  If ESP crashes after updates, ensure IRAM_ATTR still there  !!!!!!!
            }
            if (motion->acceleration_per_step != 0)
            {
                float V2 = MACHINE::velocity_squared;
                // If feedhold is active, reduce the velocity to zero to smoothly decel, no mater what we are doing
                if (MACHINE::is_feedhold)
                {
                    V2 -= motion->acceleration_per_step;
                }
                else
                {
                    float Vf2 = MIN(motion->Vf2.target * MACHINE::get_feed_override_squared(), motion->Vf2.max);
                    float Vt2 = MIN(motion->Vt2.target * MACHINE::get_feed_override_squared(), motion->Vt2.max);
                    if (current_line->delta_steps[current_line->index_of_dominate_axis] <= (V2 - Vf2) / (motion->acceleration_per_step))
                        V2 -= motion->acceleration_per_step;
                    else if (V2 < Vt2)
                        V2 += motion->acceleration_per_step;
                }
                // handle potential floating point inaccuracies
                if (V2 < 0.00001)
                    V2 = 0;
                MACHINE::velocity_squared = V2;
                if (MACHINE::velocity_squared > 0)
                    fire_interrupt_in_usec = Q_rsqrt(MAX(MACHINE::velocity_squared, 0), 0) * motion->sec_to_usec_multiplied_by_unit_vector;
            }

            if (current_line->state == bresenham_line_class::complete)
            {
                motion->is_complete = true;
                motion_queue.deqeue();
            }
            return fire_interrupt_in_usec + internal_timer_adjust;
        }

        void init()
        {
            // Initialize step pins off event on timer 1
            step_pins_off_timer = timerBegin(1, 80, true);
            timerAttachInterrupt(step_pins_off_timer, &step_pin_off_timer_event, true);
            timerWrite(step_pins_off_timer, config.step_pulse_on_usec);

            // Initialize step pins on event on timer 0;
            step_pins_on_timer = timerBegin(0, 80 / clock_usec_divider, true);
            timerAttachInterrupt(step_pins_on_timer, &step_pin_on_timer_event, true);
            timerAlarmWrite(step_pins_on_timer, config.step_max_usec_between * clock_usec_divider, true);
            timerAlarmEnable(step_pins_on_timer);
        }
    }; // namespace STEP_ENGINE
};     // namespace RFX_CNC