#pragma once
#include "Arduino.h"
#include "../state/machineState.h"
#include "../CNCEngineConfig.h"
#include "../nuts_and_bolts.h"
#include "bresenham.h"
#include <RFX_Console.h>
#include "..\parsers\commandParser.h"
#include "command_block.h"
#include "motion.h"
//#include "..\adaptors\adaptor.h"


namespace RFX_CNC{

    class operation_class{
        public:
        bool is_complete = false;
        command_block* block = nullptr;
        motion_class* motion = nullptr;

        MACHINE::machine_mode_enum machine_mode = MACHINE::run;
        // Why are these here you ask?  Unit vector will return all zero for non movement, which will make the junction speed zero
        // which in turn will result in a zero velocity.

        operation_class(command_block* _block){
            block = _block;
        }
        virtual ~operation_class(){

        }
        //unsigned long G4_time = 0;
        uint32_t pass_count = 0;
        virtual bool sub_execute(MACHINE::machine_state_class* state){
            return true;
        }
        bool execute(MACHINE::machine_state_class* state){
            if(block->comment.length()>0){
                //adaptor.send_msg(block->comment);
                block->comment.clear();
            }
            if(pass_count==0){
                state->update_state(block, false);
            }
            if(pass_count < UINT32_MAX)
                pass_count++;
            if(block->modal_flag > mg_motion){
                // TODO handle M codes that change state....
                if(bitRead(block->modal_flag,mg_spindle)){
                    MACHINE::set_spindle_state(block->get_modal(mg_spindle));
                    bitWrite(block->modal_flag,mg_spindle,0);
                }
                if(bitRead(block->modal_flag,mg_coolant)){
                    MACHINE::set_coolant_state(block->get_modal(mg_coolant));
                    bitWrite(block->modal_flag,mg_coolant,0);
                }
                block->modal_flag = block->modal_flag & ((1<<(mg_motion +1))-1);  //save the bits before mg_non_modal
            }
            // Go to reference location (G28, G30) or change coordinate system data (G10) or 
            // set axis offsets (G92, G92.1, G92.2, G94).

            // Perform motion (G0 to G3)
            // Execution assumes one G_ operation per line.
            sub_execute(state);

            if(block->modal_flag >= mg_motion)
                return false;
            // Stop (M0, M1, M2, M30).
            if(bitRead(block->modal_flag,mg_program_flow)){
                switch(block->get_modal(mg_program_flow)){
                    case M0:
                        MACHINE::perform_stop();
                    break;
                    case M1:
                        if(MACHINE::optional_stop)
                            MACHINE::perform_stop();
                    break;
                    case M2:
                            MACHINE::perform_end_of_program();
                    break;
                    case M30:
                            MACHINE::perform_end_of_program();
                    break;
                }
                bitWrite(block->modal_flag,mg_program_flow,0);
            }
            is_complete = true;
            return true;
        }
        virtual status_enum init(MACHINE::machine_state_class* state){
            state->update_state(block, true);
            return status_ok;
        }
        virtual String get_type(){
            return "null";
        }
        virtual String get_log(){
            String result = "";
            for(uint8_t i = 0;i<mg_max_value;i++){
                if(block->modal[i]!=0){
                    result += modal_description[block->modal[i]];
                    result += " ";
                }
            }
            return result;
        }
    };
}