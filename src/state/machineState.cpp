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
        String machine_mode_description[]={
            "Locked","Need Homing","idle","run","Home","Probe"
        };

        machine_state_class *machine_state;
        machine_state_class *planner_state;

        machine_mode_enum machine_mode = locked;
        bool hard_limit_enabled = true;
        uint16_t home_required = 0xFFFF;
        bool is_emergency_stop = false;
        bool is_feedhold = false;
        bool is_active = false;
        bool optional_stop = false;

        float feed_override = 1.0f; // 0.0 - 2.0f are allowable.  Applied in planner only.  
        bool feed_override_allowed = true;
        float spindle_override = 1.0f;

        void reset_home_require(){
            home_required = 0;
            for(uint8_t i = 0;i<Config::axis_count;i++){
                bitWrite(home_required,i,1);
            }
        }
        void init_machine_state()
        {
            machine_state = new machine_state_class();
            planner_state = new machine_state_class();
            machine_mode = locked;
            reset_home_require();
        }
        /*
        Inputs are mapped to machine_state->critical_status_bits, a 32bit uint.
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
            previous_critical_status_bits = machine_state->critical_status_bits;
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
                bitWrite(machine_state->critical_status_bits, i * 3, Config::critical_inputs[Config::axis[i].limit_min_map].state);
                bitWrite(machine_state->critical_status_bits, i * 3 + 1, Config::critical_inputs[Config::axis[i].limit_max_map].state);
                bitWrite(machine_state->critical_status_bits, i * 3 + 2, Config::critical_inputs[Config::axis[i].home_map].state);
            }
            bitWrite(machine_state->critical_status_bits, ESTOP_BIT, Config::critical_inputs[Config::estop_map].state);
            bitWrite(machine_state->critical_status_bits, PROBE_BIT, Config::critical_inputs[Config::probe_map].state);
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
        bool perform_cycle_start(){
            console.logln("CYCLE START");
            if (MACHINE::machine_mode < MACHINE::need_homing)
                return false;
            operation_class* operation = operation_controller.operation_queue.getHeadItemPtr();
            if(operation){
                if(MACHINE::machine_mode == MACHINE::need_homing){
                    console.logln("Type: "+operation->get_type());
                    if(operation->get_type() == "G28"){
                        machine_mode = MACHINE::home;
                        MACHINE::is_feedhold = false;
                        return true;
                    }
                    return false;
                }
                if(MACHINE::machine_mode == MACHINE::idle){
                    console.log("-HEHEHEHE-");
                    machine_mode = MACHINE::run;
                    MACHINE::is_feedhold = false;
                }
                console.log("-end-");
                return true;
            } 
            
            return true;
        }
        bool perform_feed_hold(){
            if(MACHINE::machine_mode <= MACHINE::idle)
            {
                return false;
            }
            is_feedhold = true;
            return true;
        }
        bool perform_unlock(){
            if(machine_mode == locked){
                if(home_required)
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
            Immediately halts and safely resets Grbl without a power-cycle.
            • Accepts and executes this command at any time.
            • If reset while in motion, Grbl will throw an alarm to indicate position may be lost from the motion halt.
            • If reset while not in motion, position is retained and re-homing is not required.
            • An input pin is available to connect a button or switch. 
            */
            machine_mode = locked;

            if (machine_state->velocity_squared != 0)
            {
                reset_home_require();
            }
            machine_state->velocity_squared = 0;

            STEP_ENGINE::current_line = nullptr;
            STEP_ENGINE::current_move = nullptr;

            disable_all_drives();
            operation_controller.operation_queue.flush();
            //TODO Stop Spindle
            //TODO Stop Coolant
            console.logln("EMERGENCY STOP TRIGGERED: "+msg);
            return true;
        }
        bool perform_emergency_stop()
        {
            return perform_emergency_stop("");

        }
        void handle_inputs()
        {
            String msg = "";
            if (bitRead(machine_state->critical_status_bits, ESTOP_BIT))
            {
                perform_emergency_stop();
            }

            if (bitRead(machine_state->critical_status_bits, ESTOP_BIT) + (bitRead(previous_critical_status_bits, ESTOP_BIT) << 1) == 1) //Low to High
                msg.concat("E-STOP ");
            if (bitRead(machine_state->critical_status_bits, PROBE_BIT) + (bitRead(previous_critical_status_bits, PROBE_BIT) << 1) == 1) //Low to High
                msg.concat("Probe ");

            for (uint8_t i = 0; i < Config::axis_count; i++)
            {
                uint8_t index = i * 3;
                uint8_t c0 = bitRead(machine_state->critical_status_bits, index) + (bitRead(previous_critical_status_bits, index) << 1);
                uint8_t c1 = bitRead(machine_state->critical_status_bits, index + 1) + (bitRead(previous_critical_status_bits, index + 1) << 1);
                uint8_t c2 = bitRead(machine_state->critical_status_bits, index + 2) + (bitRead(previous_critical_status_bits, index + 2) << 1);
                if (c0 == 1 || c1 == 1 || c2 == 1)
                    msg.concat(String(Config::axis[i].id));
                if (c0 == 1) //Low to High
                    msg.concat("-");
                if (c1 == 1) //Low to High
                    msg.concat("+");
                if (c2 == 1) //Low to High
                    msg.concat("H");
                if((c0==1)||(c1==1)){
                    if(machine_mode==run)
                        perform_emergency_stop("Limit Switch");
                }
            }
            if (msg.length() > 0)
            {
                console.logln(msg);
            }
        }
        void handle_outputs(){

        }
    }
}