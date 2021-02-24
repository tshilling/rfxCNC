#pragma once
#ifndef adaptor_h
#define adaptor_h

#include "Arduino.h"
#include "../state/machineState.h"
#include "../operations/operation_controller.h"
#include "../parsers/commandParser.h"
#include "../CNCEngineConfig.h"
#include <RFX_Console.h>
#include "../nuts_and_bolts.h"

#define grbl_version "1.1h"
#define data_in_buffer_size 200

namespace RFX_CNC
{
    // The base adaptor is modeled after GRBL, but exposes the interfaces needed to build other adaptors, for other standards

    class adaptor_class
    {
    public:
        virtual void send_welcome()
        {
            console.logln("Grbl v" + String(grbl_version) + " [\'$\" for help]");
        }
        virtual void send_state()
        {
            String result = "<";
            result += MACHINE::machine_mode_description[(uint8_t)MACHINE::machine_mode] + "|";
            result += "MPos:";
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                result += String((float)MACHINE::machine_state->absolute_position_steps[i] / (float)config.axis[i].steps_per_unit);
                if (i != config.axis.size() - 1)
                    result += ",";
            }
            result += "|Bf:" + String(operation_controller.operation_queue.get_available()) + "," + String(SERIAL_SIZE_RX - Serial.available());
            result += "|FS:" + String(MACHINE::machine_state->get_feed_rate());
            result += "," + String(MACHINE::machine_state->get_spindle_speed());
            result += "|WCO:"; // TODO, what the heck is WCO?
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                result += String(MACHINE::machine_state->steps_to_coordinate(MACHINE::machine_state->absolute_position_steps[i], i));
                if (i != config.axis.size() - 1)
                    result += ",";
            }
            result += ">";
            console.logln(result);
        }
        virtual void send_msg(String msg)
        {
            console.logln("[MSG: " + msg + "]");
        }

    private:
        String line_buffer = ""; //TODO, convert String to char[] buffer
        virtual bool process_immediate(char in)
        {
            switch (in)
            {
            case 0x18:                             // 0x18 (ctrl-x) : Soft-Reset
                MACHINE::perform_emergency_stop(); // TODO, GRBL calls this a soft_reset, look into how that should be handled different
                send_welcome();
                break;
            case '?':
                send_state();
                break;
            case '~': // Cycle Start
                MACHINE::perform_cycle_start();
                break;
            case '!':
                MACHINE::perform_feed_hold();
                break;
            case 0x90: // Set 100% feed rate.  If parameter is passed, us that instead (in percent)
                MACHINE::set_feed_override(1.0);
                break;
            case 0x91: // Increase feed overide by 10%
                MACHINE::set_feed_override(MACHINE::get_feed_override() + 0.1);
                break;
            case 0x92: // Decrease feed overide by 10%
                MACHINE::set_feed_override(MACHINE::get_feed_override() - 0.1);
                break;
            case 0x93: // Increase feed overide by 1%
                MACHINE::set_feed_override(MACHINE::get_feed_override() + 0.01);
                break;
            case 0x94: // Decrease feed overide by 1%
                MACHINE::set_feed_override(MACHINE::get_feed_override() - 0.01);
                break;
            case 0x99: // Set 100% spindle rate.  If parameter is passed, us that instead (in percent)
                MACHINE::set_spindle_override(1.0);
                break;
            case 0x9A: // Increase spindle overide by 10%
                MACHINE::set_spindle_override(MACHINE::get_spindle_override() + 0.1);
                break;
            case 0x9B: // Decrease spindle overide by 10%
                MACHINE::set_spindle_override(MACHINE::get_spindle_override() - 0.1);
                break;
            case 0x9C: // Increase spindle overide by 1%
                MACHINE::set_spindle_override(MACHINE::get_spindle_override() + 0.01);
                break;
            case 0x9D: // Decrease spindle overide by 1%
                MACHINE::set_spindle_override(MACHINE::get_spindle_override() - 0.01);
                break;
            // TODO
            // 0x84 door
            // 0x85 Jog Cancel
            // 0x9E : Toggle Spindle Stop
            // 0xA0 : Toggle Flood Coolant
            // 0xA1 : Toggle Mist Coolant
            default:
                return false;
            }
            return true;
        }
        virtual status_enum process_line(String input)
        {
            if (input.length() == 0)
            {
                return status_ok;
            }
            PARSER::result_struct result = PARSER::parse(input);
            if (result.status != status_ok)
                return result.status;

            PARSER::command_struct *commands = result.command;
            if (commands->parameter.size() == 0)
                return status_ok;
            if (commands->parameter[0].key.equals("$$"))
            {
                print_settings();
                return status_ok;
            }
            if (commands->parameter[0].key.equals("$"))
            {
                if (!isnan(commands->parameter[0].value))
                {
                    // TODO
                    console.logln("$# Not Currently Supported");
                    return status_ok;
                }
                console.logln(
                    "[HLP:"
                    "$$ (view Grbl settings)\n"
                    "$# (view # parameters)\n"
                    "$G (view parser state)\n"
                    "$I (view build info)\n"
                    "$N (view startup blocks)\n"
                    "$x=value (save Grbl setting)\n"
                    "$Nx=line (save startup block)\n"
                    "$C (check gcode mode)\n"
                    "$X (kill alarm lock)\n"
                    "$H (run homing cycle)\n"
                    "~ (cycle start)\n"
                    "! (feed hold)\n"
                    "? (current status)\n"
                    "ctrl-x (reset Grbl)"
                    "]");
                return status_ok;
            }
            if (commands->parameter[0].key.equals("$X"))
            {
                MACHINE::perform_unlock();
                return status_ok;
            }
            if (commands->parameter[0].key.equals("$G"))
            {
                String result = "[GC:";
                for (uint8_t i = 0; i < modal_group_enum::mg_max_value; i++)
                {
                    result += modal_description[MACHINE::machine_state->block.modal[i]];
                    result += " ";
                }
                result += "T0"; // TODO, make real tool number
                result += " F" + String(MACHINE::machine_state->get_feed_rate());
                result += " S" + String(MACHINE::machine_state->get_spindle_speed());
                result += "]";
                console.logln(result);
                //      status_report();
                return status_ok;
            }
            if (commands->parameter[0].key.equals("$I"))
            {
                console.logln("[VER:" + String(grbl_version) + ":]");
                console.logln("[OPT:]"); // TODO log options
                return status_ok;
            }
            if (commands->parameter[0].key.equals("$N"))
            {
                // TODO, startup gcode
                console.logln("$N0= Not Implemented");
                console.logln("$N1= Not Implemented");
                return status_ok;
            }
            if (commands->parameter[0].key.equals("$C"))
            {
                // TODO, toggle gcode check mode
                return status_ok;
            }
            if (commands->parameter[0].key.equals("$X"))
            {
                // TODO, kill alarm lock
                return status_ok;
            }
            if (commands->parameter[0].key.equals("$H"))
            {
                // TODO, run home cycle
                return status_ok;
            }
            if (commands->parameter[0].key.equals("$J"))
            {
                // TODO, Run Jogging Motion
                return status_ok;
            }
            if (commands->parameter[0].key.equals("$RST"))
            {
                // TODO, restore settings to defualt
                return status_ok;
            }
            if (commands->parameter[0].key.equals("$SLP"))
            {
                // TODO, Sleep
                return status_ok;
            }
            return RFX_CNC::operation_controller.add_operation_to_queue(commands);
        }
        void print_settings()
        {
            console.logln("$0=" + String(config.step_pulse_on_usec) + "\t(Step pulse, usec)");
            console.logln("$1=" + String(config.step_idle_delay_msec) + "\t(Step idle delay, msec)");
            console.logln("$2=" + String(config.get_step_port_invert_mask(), 2) + "\t(Step port invert, mask)");
            console.logln("$3=" + String(config.get_dir_port_invert_mask(), 2) + "\t(Direction port invert, mask)");
            console.logln("$4=" + String(config.get_step_enable_port_invert_mask(), 2) + "\t(Step Enable port invert, mask)");
            console.logln("$5=" + String(config.get_limit_pin_min_invert_mask(), 2) + "\t(Limit pin min invert, mask)");
            console.logln("$5.1=" + String(config.get_limit_pin_max_invert_mask(), 2) + "\t(Limit pin max invert, mask)");
            console.logln("$6=" + String(config.probe_pin_invert) + "\t(Probe pin invert, mask)");
            console.logln("$10=0 \t(Status report, mask)");
            console.logln("$11=" + String(config.junction_deviation, 3) + "\t(Junction deviation, mm)");
            console.logln("$12=" + String(config.arc_tolerance, 3) + "\t(Arc tolerance, mm)");
            console.logln("$13=0 \t(Report inches)");

            console.logln("$20=" + String(config.get_soft_limit_mask(), 2) + "\t(Soft limits, mask)");
            console.logln("$21=" + String(config.get_hard_limit_mask(), 2) + "\t(Hard limits, mask)");
            console.logln("$22=" + String(config.get_home_cycle_mask(), 2) + "\t(Require Homing, mask)");
            console.logln("$23=" + String(config.get_home_dir_invert_mask(), 2) + "\t(Homing direction invert, mask)");

            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                if (i < 1)
                    console.logln("$24=" + String((float)config.axis[i].home_feed_fine, 3) + "\t(" + config.axis[i].id + " Homing feed, mm/min)");
                else
                    console.logln("$24." + String(i) + "=" + String((float)config.axis[i].home_feed_fine, 3) + "\t(" + config.axis[i].id + " Homing feed, mm/min)");
            }
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                if (i < 1)
                    console.logln("$25=" + String((float)config.axis[i].home_feed_coarse, 3) + "\t(" + config.axis[i].id + " Homing seek, mm/min)");
                else
                    console.logln("$25." + String(i) + "=" + String((float)config.axis[i].home_feed_coarse, 3) + "\t(" + config.axis[i].id + " Homing seek, mm/min)");
            }
            console.logln("$26=" + String((float)config.input_debounce_msec, 3) + "\t(Input debounce, msec)");
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                if (i < 1)
                    console.logln("$27=" + String((float)config.axis[i].home_pulloff_units, 3) + "\t(" + config.axis[i].id + " Homing pulloff, mm)");
                else
                    console.logln("$27." + String(i) + "=" + String((float)config.axis[i].home_pulloff_units, 3) + "\t(" + config.axis[i].id + " Homing pulloff, mm)");
            }

            for (uint8_t i = 0; i < config.spindle.size(); i++)
            {
                if (i < 1)
                    console.logln("$30=" + String((float)config.spindle[i].max_value, 3) + "\t(Max spindle speed, RPM)");
                else
                    console.logln("$30." + String(i) + "=" + String((float)config.spindle[i].max_value, 3) + "\t(Max spindle speed, RPM)");
            }
            for (uint8_t i = 0; i < config.spindle.size(); i++)
            {
                if (i < 1)
                    console.logln("$31=" + String((float)config.spindle[i].min_value, 3) + "\t(Max spindle speed, RPM)");
                else
                    console.logln("$31." + String(i) + "=" + String((float)config.spindle[i].min_value, 3) + "\t(Max spindle speed, RPM)");
            }
            console.logln("$32=" + String(config.get_ramp_mask(), 2) + "\t(Laser mode - ramp, mask)");

            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                if (i <= 10)
                    console.logln("$" + String(100 + i) + "=" + String((float)config.axis[i].steps_per_unit, 3) + "\t(" + config.axis[i].id + " steps/mm)");
                else
                    console.logln("$109." + String(i) + "=" + String((float)config.axis[i].steps_per_unit, 3) + "\t(" + config.axis[i].id + " steps/mm)");
            }
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                if (i <= 10)
                    console.logln("$" + String(110 + i) + "=" + String(config.axis[i].max_feed_units_per_sec, 3) + "\t(" + config.axis[i].id + " Max rate, mm/min)");
                else
                    console.logln("$119." + String(i) + "=" + String((float)config.axis[i].max_feed_units_per_sec, 3) + "\t(" + config.axis[i].id + " Max rate, mm/min)");
            }
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                if (i <= 10)
                    console.logln("$" + String(120 + i) + "=" + String(config.axis[i].acceleration, 3) + "\t(" + config.axis[i].id + " Acceleration, mm/sec^2)");
                else
                    console.logln("$129." + String(i) + "=" + String((float)config.axis[i].acceleration, 3) + "\t(" + config.axis[i].id + " Acceleration, mm/sec^2)");
            }
            for (uint8_t i = 0; i < config.axis.size(); i++)
            {
                if (i <= 10)
                    console.logln("$" + String(130 + i) + "=" + String(config.axis[i].max_distance, 3) + "\t(" + config.axis[i].id + " Max travel, mm)");
                else
                    console.logln("$139." + String(i) + "=" + String(config.axis[i].max_distance, 3) + "\t(" + config.axis[i].id + " Max travel, mm)");
            }
        }

    public:
        adaptor_class()
        {
        }
        ~adaptor_class()
        {
        }
        bool service()
        {
            for (uint8_t i = 0; i < line_buffer.length(); i++)
            {
                if (line_buffer[i] == '\n')
                {
                    status_enum result = process_line(line_buffer.substring(0, i));
                    if (result == status_ok)
                    {
                        console.logln("ok");
                        line_buffer = line_buffer.substring(i + 1, line_buffer.length());
                        return true;
                    }
                    else if (result == status_queue_full)
                    {
                        return false;
                    }
                    else
                    {
                        console.logln("error:" + String(result));
                        line_buffer = line_buffer.substring(i, line_buffer.length());
                        return false;
                    }
                }
            }
            return true;
        }
        status_enum push_stream_in(char in)
        {
            if (!process_immediate(in))
            {
                if (line_buffer.length() >= data_in_buffer_size)
                {
                    console.logln("error:" + String(status_max_line_length_exceeded));
                    return status_max_line_length_exceeded;
                }
                line_buffer += in;
            }
            return status_ok;
        }
        status_enum push_stream_in(String in)
        {
            status_enum result = status_ok;
            for (uint8_t i = 0; i < in.length(); i++)
            {
                result = push_stream_in(in[i]);
                if (result != status_ok)
                    break;
            }
            return result;
        }
    };
    extern adaptor_class adaptor;
};
#endif