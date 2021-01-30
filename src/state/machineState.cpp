#include "machineState.h"
#include <RFX_Console.h> // Common serial port out interface
#include "Arduino.h"
#include "../operations/operation_controller.h"
#define ESTOP_BIT 31
#define PROBE_BIT 30
namespace RFX_CNC
{
    namespace MACHINE
    {
        String machine_mode_description[] = {
            "Locked", "Need Homing", "idle", "run", "Home", "Probe"};

        machine_state_class *machine_state;
        machine_state_class *planner_state;

        machine_mode_enum machine_mode = locked;
        bool hard_limit_enabled = true;
        uint16_t home_required = 0xFFFF;
        bool require_home_after_reset = false;
        bool is_emergency_stop = false;
        bool is_feedhold = false;
        bool is_active = false;
        bool optional_stop = false;

        float feed_override_squared = 1.0f;
        bool feed_override_allowed = true;
        float spindle_override = 1.0f;
        bool spindle_override_allowed = true;

        float velocity_squared = 0;
        float spindle_speed = 0;
        uint32_t critical_status_bits = 0;

        void print_gcode_mode(){
            String result = "Gcode";
            for(uint8_t i = 0; i < 26;i++){
                result += String(machine_state->parameter[i]) + " ";
            }
            console.logln(result);
        }
        float getVelocity()
        {
            return sqrtf(velocity_squared);//(1.0f / v);
        }
        void reset_home_require()
        {
            home_required = 0;
            if (!require_home_after_reset)
                return;
            for (uint8_t i = 0; i < Config::axis_count; i++)
            {
                bitWrite(home_required, i, 1);
            }
        }
        bool set_feed_override(float value)
        {
            if (feed_override_allowed)
            {
                if (value < 0.01)
                    value = 0.01;
                feed_override_squared = powf2(value);
                return true;
            }
            return false;
        }
        float get_feed_override()
        {
            return sqrtf(feed_override_squared);
        }
        bool set_spindle_override(float value)
        {
            if (spindle_override_allowed)
            {
                if (value < 0.01)
                    value = 0.01;
                spindle_override = value;
                return true;
            }
            return false;
        }
        float get_spindle_override()
        {
            return spindle_override;
        }
        void init_machine_state()
        {
            machine_state = new machine_state_class();
            planner_state = new machine_state_class();
            machine_mode = locked;
            reset_home_require();
            set_feed_override(1.0f);
        }
        String get_state_log(String pre)
        {
            String result = "[" + MACHINE::machine_mode_description[(uint8_t)MACHINE::machine_mode];
            if (MACHINE::machine_mode == MACHINE::need_homing)
            {
                result += " ";
                for (uint8_t i = 0; i < Config::axis_count; i++)
                {
                    if (bitRead(MACHINE::home_required, i))
                    {
                        result += String(Config::axis[i].id);
                    }
                }
            }
            result += "] "+pre+"- S: "+String(spindle_speed)+"  F: " + String(MACHINE::getVelocity()) + " (units/sec)  Coord:";
            for (int i = 0; i < Config::axis_count; i++)
            {
                result += " " + String(((float)MACHINE::machine_state->absolute_position_steps[i]) / ((float)Config::axis[i].steps_per_unit));
            }
            result += " (units)";
            return result;
        }
        String get_state_log()
        {
            return get_state_log("");
        }
        /*
        Inputs are mapped to critical_status_bits, a 32bit uint.
        Status_bits are for critical debounced inputs (limits, E-Stop, Probing) Things that stop the machine
            bit         Assigned to:
             0-2    Axis 0 [Min, Max, Home]
             3-5    Axis 1 [Min, Max, Home]
             6-8    Axis 2 [Min, Max, Home]
             9-11   Axis 3 [Min, Max, Home]
            12-14   Axis 4 [Min, Max, Home]
            15-17   Axis 5 [Min, Max, Home]
            18-20   Axis 6 [Min, Max, Home]
            21-23   Axis 7 [Min, Max, Home]
            24-27   Axis 8 [Min, Max, Home]
            28      
            29      
            30      Probe
            31      E-STOP
                    
        Control_bits... TBD
                    single line operation on/off    Digital
                    spindle on/off                  Digitial
                    coolant on/off                  Digital
                    feed override                   Analog
                    spindle override                Analog
    */
        uint32_t previous_critical_status_bits = 0;

        void scan_inputs(unsigned long delta_time)
        {
            previous_critical_status_bits = critical_status_bits;
            // Perform debounce calculations / tracking
            for (uint8_t i = 0; i < Config::critical_inputs.size(); i++)
            {
                Config::input_struct *input = &Config::critical_inputs[i];
                if (input->pin < 0)
                    continue;
                if ((digitalRead(input->pin) != input->invert) == 0)
                {
                    // Open Input
                    if (input->debounce > 0)
                    {
                        input->debounce -= delta_time;
                    }
                    else
                    {
                        input->debounce = 0;
                        input->state = 0;
                    }
                }
                else
                {
                    // Close Input
                    if (input->debounce < Config::input_debounce_usec)
                    {
                        input->debounce += delta_time;
                    }
                    else
                    {
                        input->debounce = Config::input_debounce_usec;
                        input->state = 1;
                    }
                }
            }

            // Copy inputs to coorisponding bit
            for (uint8_t i = 0; i < Config::axis_count; i++)
            {
                bitWrite(critical_status_bits, i * 3, Config::critical_inputs[Config::axis[i].limit_min_map].state);
                bitWrite(critical_status_bits, i * 3 + 1, Config::critical_inputs[Config::axis[i].limit_max_map].state);
                bitWrite(critical_status_bits, i * 3 + 2, Config::critical_inputs[Config::axis[i].home_map].state);
            }
            bitWrite(critical_status_bits, ESTOP_BIT, Config::critical_inputs[Config::estop_map].state);
            bitWrite(critical_status_bits, PROBE_BIT, Config::critical_inputs[Config::probe_map].state);
        }
        void disable_all_drives()
        {
            for (uint8_t i = 0; i < Config::axis_count; i++)
            {
                digitalWrite(Config::axis[i].enable_pin, Config::axis[i].enable_pin_invert);
                digitalWrite(Config::axis[i].dir_pin, Config::axis[i].dir_pin_invert);
                digitalWrite(Config::axis[i].step_pin, Config::axis[i].step_pin_invert);
            }
        }
        void enable_all_drives()
        {
            if (is_emergency_stop)
                return;
            for (uint8_t i = 0; i < Config::axis_count; i++)
            {
                digitalWrite(Config::axis[i].enable_pin, !Config::axis[i].enable_pin_invert);
            }
        }
        bool perform_cycle_start()
        {
            console.logln("CYCLE START");
            if (MACHINE::machine_mode < MACHINE::need_homing)
                return false;
            operation_class *operation = operation_controller.operation_queue.getHeadItemPtr();
            if (operation)
            {
                if (MACHINE::machine_mode == MACHINE::need_homing)
                {
                    console.logln("Type: " + operation->get_type());
                    if (operation->get_type() == "G28")
                    {
                        machine_mode = MACHINE::home;
                        MACHINE::is_feedhold = false;
                        return true;
                    }
                    return false;
                }
                if (MACHINE::machine_mode == MACHINE::idle)
                {
                    machine_mode = MACHINE::run;
                    MACHINE::is_feedhold = false;
                }
                return true;
            }

            return true;
        }
        bool perform_feed_hold()
        {
            if (MACHINE::machine_mode <= MACHINE::idle)
            {
                return false;
            }
            is_feedhold = true;
            return true;
        }
        bool perform_unlock()
        {
            if (machine_mode == locked)
            {
                if (home_required)
                {
                    machine_mode = need_homing;
                    return true;
                }
                machine_mode = idle;
            }
            return true;
        }
        void set_machine_mode()
        {
            if (is_emergency_stop)
            {
                machine_mode = locked;
                return;
            }
            if (machine_mode < idle)
                return;
            if (home_required && machine_mode != home)
            {
                machine_mode = need_homing;
                return;
            }
            if (operation_controller.operation_queue.isEmpty())
            {
                machine_mode = idle;
            }
        }
        bool perform_emergency_stop(String msg)
        {
            /*
            Immediately halts and safely resets without a power-cycle.
            • Accepts and executes this command at any time.
            • If reset while in motion, Grbl will throw an alarm to indicate position may be lost from the motion halt.
            • If reset while not in motion, position is retained and re-homing is not required.
            • An input pin is available to connect a button or switch. 
            */
            machine_mode = locked;

            if (velocity_squared != 0)
            {
                reset_home_require();
            }
            velocity_squared = 0;

            STEP_ENGINE::current_line = nullptr;
            STEP_ENGINE::current_move = nullptr;

            disable_all_drives();
            operation_controller.operation_queue.flush();
            //TODO Stop Spindle
            MACHINE::machine_state->spindle_state = MACHINE::machine_state_class::spindle_off;
            //TODO Stop Coolant
            console.logln("EMERGENCY STOP TRIGGERED: " + msg);
            return true;
        }
        bool perform_emergency_stop()
        {
            return perform_emergency_stop("");
        }
        void handle_inputs()
        {
            String msg = "";
            if (bitRead(critical_status_bits, ESTOP_BIT))
            {
                perform_emergency_stop();
            }

            if (bitRead(critical_status_bits, ESTOP_BIT) + (bitRead(previous_critical_status_bits, ESTOP_BIT) << 1) == 1) //Low to High
                msg.concat("E-STOP ");
            if (bitRead(critical_status_bits, PROBE_BIT) + (bitRead(previous_critical_status_bits, PROBE_BIT) << 1) == 1) //Low to High
                msg.concat("Probe ");

            for (uint8_t i = 0; i < Config::axis_count; i++)
            {
                uint8_t index = i * 3;
                uint8_t c0 = bitRead(critical_status_bits, index) + (bitRead(previous_critical_status_bits, index) << 1);
                uint8_t c1 = bitRead(critical_status_bits, index + 1) + (bitRead(previous_critical_status_bits, index + 1) << 1);
                uint8_t c2 = bitRead(critical_status_bits, index + 2) + (bitRead(previous_critical_status_bits, index + 2) << 1);
                if (c0 == 1 || c1 == 1 || c2 == 1)
                    msg.concat(String(Config::axis[i].id));
                if (c0 == 1) //Low to High
                    msg.concat("-");
                if (c1 == 1) //Low to High
                    msg.concat("+");
                if (c2 == 1) //Low to High
                    msg.concat("H");
                if ((c0 == 1) || (c1 == 1))
                {
                    if (machine_mode == run)
                        perform_emergency_stop("Limit Switch");
                }
            }
            if (msg.length() > 0)
            {
                console.logln(msg);
            }
        }
        void handle_outputs()
        {
            // Handle Spindle Output
            if(machine_state->spindle_state == machine_state_class::spindle_off){
                spindle_speed = 0; // TODO, control it
            }
            else if(machine_state->spindle_state == machine_state_class::spindle_CW){
                spindle_speed = spindle_override * machine_state->parameter[_S_]; // TODO, control it
            }
            else if(machine_state->spindle_state == machine_state_class::spindle_CCW){
                spindle_speed = spindle_override * -machine_state->parameter[_S_]; // TODO, control it
            }
        }
    } // namespace MACHINE
} // namespace RFX_CNC