#include "machineState.h"
#include "Arduino.h"
#include "../operations/operation_controller.h"
#include "../operations/command_block.h"
#define ESTOP_BIT 31
#define PROBE_BIT 30
namespace RFX_CNC
{
    String modal_description[modal_enum::MAX_VALUE] = {
        "not_set",

        "G0",
        "G1",
        "G2",
        "G3",
        "G4",
        "G38.2",
        "G38.3",
        "G38.4",
        "G38.5",
        "G80", // Motion
        "G54",
        "G55",
        "G56",
        "G57",
        "G58",
        "G59", // Coordinate Select
        "G92",
        "G28",
        "G30",
        "G60",
        "G17",
        "G18",
        "G19", // Plane Select
        "G90",
        "G90.1",
        "G91",   // Distance Mode
        "G91.1", // Distance Arc Mode
        "G93",
        "G94", // Feed Rate Mode
        "G20",
        "G21", // Units
        "G40", // Cutter Compensation
        "G43.1",
        "G49", // Tool Length
        "G61", // Control Mode
        "M0",
        "M1",
        "M2",
        "M30", // Program Flow
        "M3",
        "M4",
        "M5", // Spindle
        "M7",
        "M8",
        "M9",
        "M48",
        "M49"};
        
    namespace MACHINE
    {
        String machine_mode_description[] = {
            "ALARM", "DOOR","Need Homing", "IDLE", "RUN", "HOLD", "HOMING", "PROBING", "JOG"};

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
        float spindle_override = 1.0f;
        bool spindle_override_allowed = true;

        float velocity_squared = 0;
        uint32_t critical_status_bits = 0;

        uint32_t critical_min_mask = 0;
        uint32_t critical_max_mask = 0;
        uint32_t critical_home_mask = 0;
        uint32_t critical_other_mask = 0;
       

        float getVelocity()
        {
            return sqrtf(velocity_squared); //(1.0f / v);
        }
        float getSpindle()
        {
            return machine_state->get_spindle_speed() * MACHINE::spindle_override; ////(1.0f / v);
        }
        void reset_home_require()
        {
            home_required = 0;
            if (!require_home_after_reset)
                return;
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                bitWrite(home_required, i, 1);
            }
        }
        void set_feed_override(float value)
        {
            if (value < 0.01)
                value = 0.01;
            feed_override_squared = powf2(value);
        }
        float get_feed_override()
        {
            if (machine_state->is_feed_overried_allowed())
                return sqrtf(feed_override_squared);
            return 1.0f;
        }
        float get_feed_override_squared()
        {
            if (machine_state->is_feed_overried_allowed())
                return feed_override_squared;
            return 1.0f;
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
            critical_min_mask = 0;
            critical_max_mask = 0;
            critical_home_mask = 0;
            critical_other_mask = 0;
            for(uint8_t i = 0; i < 30 ; i+=3){
                bitWrite(critical_min_mask,i,1);
                bitWrite(critical_max_mask,i+1,1);
                bitWrite(critical_home_mask,i+2,1);
            }
            for(uint8_t i = 30;i<32;i++){
                bitWrite(critical_other_mask,i,1);
            }
            
            machine_state = new machine_state_class();
            planner_state = new machine_state_class();
            machine_mode = locked;

            machine_state->block.set_modal(mg_motion, G0);
            machine_state->block.set_modal(mg_coordinate, G54);
            machine_state->block.set_modal(mg_plane, G17);
            machine_state->block.set_modal(mg_distance, G90);
            machine_state->block.set_modal(mg_arc, G91_1);
            machine_state->block.set_modal(mg_feed_rate, G94);
            machine_state->block.set_modal(mg_units, G21);
            machine_state->block.set_modal(mg_cutter_compensation, G40);
            machine_state->block.set_modal(mg_tool_length, G49);
            machine_state->block.set_modal(mg_program_flow, M0);
            machine_state->block.set_modal(mg_spindle, M5);
            machine_state->block.set_modal(mg_coolant, M9);
            machine_state->block.set_modal(mg_override, M48);

            for(uint8_t i = 0; i < config.axis.size(); i++){
                config.get_coordinate_system(G92-G54)[i] = 0;
                config.get_coordinate_system(G28-G54)[i] = 0;
                config.get_coordinate_system(G30-G54)[i] = 0;
            }

            reset_home_require();
            set_feed_override(1.0f);
        }
        String get_state_log(String pre)
        {
            String result = "<";
            result += MACHINE::machine_mode_description[(uint8_t)MACHINE::machine_mode] + "|";
            result += "MPos:";
            for(uint8_t i = 0; i <config.axis.size();i++){
                result += String((float)machine_state->absolute_position_steps[i]/(float)config.axis[i].steps_per_unit);
                if(i!= config.axis.size()-1)
                    result+=",";
            }
            result += "|WPos:";
            for(uint8_t i = 0; i <config.axis.size();i++){
                result += String(machine_state->steps_to_coordinate(machine_state->absolute_position_steps[i],i));
                if(i!= config.axis.size()-1)
                    result+=",";
            }
            result+="|Bf:"+String(operation_controller.operation_queue.get_available()) + ","+String(SERIAL_SIZE_RX - Serial.available());
            result+="|F:"+String(machine_state->get_feed_rate());
            result+="|S:"+String(machine_state->get_spindle_speed());
            result += ">";
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
            for (uint8_t i = 0; i < config.critical_inputs.size(); i++)
            {
                input_struct *input = &config.critical_inputs[i];
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
                    if (input->debounce < config.input_debounce_msec*1000)
                    {
                        input->debounce += delta_time;
                    }
                    else
                    {
                        input->debounce = config.input_debounce_msec*1000;
                        input->state = 1;
                    }
                }
            }

            // Copy inputs to coorisponding bit
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                bitWrite(critical_status_bits, i * 3, config.critical_inputs[config.axis[i].limit_min_map].state);
                bitWrite(critical_status_bits, i * 3 + 1, config.critical_inputs[config.axis[i].limit_max_map].state);
                bitWrite(critical_status_bits, i * 3 + 2, config.critical_inputs[config.axis[i].home_map].state);
            }
            bitWrite(critical_status_bits, ESTOP_BIT, config.critical_inputs[config.estop_map].state);
            bitWrite(critical_status_bits, PROBE_BIT, config.critical_inputs[config.probe_map].state);
        }
        void disable_all_drives()
        {
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                digitalWrite(config.axis[i].enable_pin, config.axis[i].enable_pin_invert);
                digitalWrite(config.axis[i].dir_pin, config.axis[i].dir_pin_invert);
                digitalWrite(config.axis[i].step_pin, config.axis[i].step_pin_invert);
            }
        }
        void enable_all_drives()
        {
            if (is_emergency_stop)
                return;
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                digitalWrite(config.axis[i].enable_pin, !config.axis[i].enable_pin_invert);
            }
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
            if (home_required && machine_mode != homing)
            {
                machine_mode = need_homing;
                return;
            }
            if (operation_controller.operation_queue.isEmpty())
            {
                machine_mode = idle;
                return;
            }
            machine_mode = run;
            return;
        }

        bool perform_cycle_start()
        {
            if (MACHINE::machine_mode < MACHINE::need_homing)
                return false;
            operation_class *operation = operation_controller.operation_queue.getHeadItemPtr();
            if (operation)
            {
                if (MACHINE::machine_mode == MACHINE::need_homing)
                {
                    //console.logln("Type: " + operation->get_type());
                    if (operation->get_type() == "G28")
                    {
                        machine_mode = MACHINE::homing;
                        MACHINE::is_feedhold = false;
                        enable_all_drives();
                        return true;
                    }
                    return false;
                }
                if (MACHINE::machine_mode >= MACHINE::idle && MACHINE::machine_mode <= MACHINE::hold)
                {
                    machine_mode = MACHINE::run;
                    MACHINE::is_feedhold = false;
                }
                enable_all_drives();
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
            MACHINE::machine_mode = hold;
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

            set_spindle_state(M5);
            set_coolant_state(M9);
            //console.logln("EMERGENCY STOP TRIGGERED: " + msg);
            return true;
        }
        bool perform_emergency_stop()
        {
            return perform_emergency_stop("");
        }

        void perform_stop()
        {
            //console.logln("Stop");
            machine_mode = machine_mode_enum::idle;
        }
        void perform_end_of_program()
        {
            //console.logln("End Of Program");
            perform_stop();
            operation_controller.operation_queue.flush();
        }
        
        void set_spindle_state(uint8_t command)
        {
            // TODO
            //console.log("Spindle- ");
            if (command == M5){

            }
               // console.logln("off");
            if (command == M3)
            {
              //  console.log("cw ");
               // console.logln(String(machine_state->block.parameter[_S_]));
            }
            if (command == M4)
            {
               // console.log("ccw ");
               // console.logln(String(machine_state->block.parameter[_S_]));
            }
        }
        void set_coolant_state(uint8_t command)
        {
            // TODO
            //console.log("Coolant- ");
            if (command == M9){

            }
            //    console.logln("off");
            if (command == M7)
            {
            //    console.logln("mist");
            }
            if (command == M8)
            {
             //   console.logln("flood");
            }
        }
        
        void handle_inputs()
        {
            String msg = "";
            if (bitRead(critical_status_bits, ESTOP_BIT))
            {
                //console.logln("Critical: "+String(critical_status_bits,2));
                perform_emergency_stop();
            }

            if (bitRead(critical_status_bits, ESTOP_BIT) + (bitRead(previous_critical_status_bits, ESTOP_BIT) << 1) == 1) //Low to High
                msg.concat("E-STOP ");
            if (bitRead(critical_status_bits, PROBE_BIT) + (bitRead(previous_critical_status_bits, PROBE_BIT) << 1) == 1) //Low to High
                msg.concat("Probe ");

            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                uint8_t index = i * 3;
                uint8_t c0 = bitRead(critical_status_bits, index) + (bitRead(previous_critical_status_bits, index) << 1);
                uint8_t c1 = bitRead(critical_status_bits, index + 1) + (bitRead(previous_critical_status_bits, index + 1) << 1);
                uint8_t c2 = bitRead(critical_status_bits, index + 2) + (bitRead(previous_critical_status_bits, index + 2) << 1);
                if (c0 == 1 || c1 == 1 || c2 == 1)
                    msg.concat(String(config.axis[i].id));
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
        }

    } // namespace MACHINE
} // namespace RFX_CNC