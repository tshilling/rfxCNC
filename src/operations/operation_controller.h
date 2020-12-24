#pragma once
#include "Arduino.h"
#include "RFXQueue.h"
#include "..\state\machineState.h"
#include "operations.h"
#include "GCodeParser.h"
#include "CNCHelpers.h"
#include "G1.h"
#include "G4.h"
#include "G28.h"
#include "M0.h"
#include "M1.h"
#include "M2.h"
#include "M3.h"
#include "M4.h"
#include "M5.h"
#include "M6.h"
#include "M7.h"
#include "M8.h"
#include "M9.h"
 /* 
        --- MODAL COMMANDS ---
        Motion Mode	G0, G1, G2, G3, G38.2, G38.3, G38.4, G38.5, G80
        Coordinate System Select	G54, G55, G56, G57, G58, G59
        Plane Select	G17, G18, G19
        Distance Mode	G90, G91
        Arc IJK Distance Mode	G91.1
        Feed Rate Mode	G93, G94
        Units Mode	G20, G21
        NOT - Cutter Radius Compensation	G40
        NOT - Tool Length Offset	G43.1, G49
        Program Mode	M0, M1, M2, M30
        Spindle State	M3, M4, M5
        Coolant State	M7, M8, M9

        --- NON MODAL ---
        G4, G10 L2, G10 L20, G28, G30, G28.1, G30.1, G53, G92, G92.1

        -- Order of operations ---
        Comment (including message)
        Set feed rate mode (G93, G94).
        Set feed rate (F).
        Set spindle speed (S).
        Select tool (T).
        Change tool (M6).
        Spindle on or off (M3, M4, M5).
        Coolant on or off (M7, M8, M9).
        Enable or disable overrides (M48, M49).
        Dwell (G4).
        Set active plane (G17, G18, G19).
        Set length units (G20, G21).
        Cutter radius compensation on or off (G40, G41, G42)
        Cutter length compensation on or off (G43, G49)
        Coordinate system selection (G54, G55, G56, G57, G58, G59, G59.1, G59.2, G59.3).
        Set path control mode (G61, G61.1, G64)
        Set distance mode (G90, G91).
        Set retract mode (G98, G99).
        Go to reference location (G28, G30) or change coordinate system data (G10) or set axis offsets (G92, G92.1, G92.2, G94).
        Perform motion (G0 to G3, G33, G73, G76, G80 to G89), as modified (possibly) by G53.
        Stop (M0, M1, M2, M30, M60).
        */
namespace CNC_ENGINE{
    class operation_controller_class{
        private:
            operation_class* current_operation = nullptr; 
            float sticky_parameters[26] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        public:
        rfxQueue<operation_class> operation_queue;
        operation_controller_class(){
        }
        ~operation_controller_class(){

        }
        void init(uint16_t queue_buffer_size){
            operation_queue.resize_queue(queue_buffer_size);  
            for(uint8_t i = 0; i < 26; i++){
                sticky_parameters[i] = 0;
            }
        }
        void clip_speed_to_achievable(float* V0_squared, float* Vc_squared, float* Vf_squared, float* acceleration, float* distance){
            *Vf_squared = MIN(abs((*V0_squared) + 2 * (*acceleration) * (*distance)),abs((*Vc_squared)));
        }
        rfxQueue<operation_class>::Node* get_motion_working_backward(rfxQueue<operation_class>::Node* start){
            while(start!=nullptr){
                if(start->item->is_movement)
                    return start;
                start = start->previous;
            }
            return nullptr;
        }
        rfxQueue<operation_class>::Node* get_motion_working_forward(rfxQueue<operation_class>::Node* start){
            while(start!=nullptr){
                if(start->item->is_movement)
                    return start;
                start = start->previous;
            }
            return nullptr;
        }
        void plan(bool update_all){
            rfxQueue<operation_class>::Node* node = operation_queue.getTailPtr();
            if(node==nullptr)
                return;
            // There are two different modes of planning, ALL, and optimized
            operation_class* operation = node->item;
            if(node->item == nullptr)
                return; 
            operation->set_Vf_squared(0);
            while(1){
                if(operation->is_movement){
                    operation->set_Vc_squared();
                    operation->clip_velocity_kinematically(backward);
                }
                else{
                    if(!update_all)
                        break;
                }
                if(!node->previous)
                    break;

                float V = operation->get_V0_squared()*operation->get_dot_product_with_previous_segment();
                node->previous->item->set_Vf_squared(V);
                operation->set_V0_squared(V);

                operation = node->previous->item;
                node = node->previous;
            }
            operation->set_V0_squared(0);
            while(1){
                operation->clip_velocity_kinematically(forward);
                if(!node->next)
                    break;
                operation_class* next = node->next->item;  
                float V = operation->get_Vf_squared()*next->get_dot_product_with_previous_segment();
                V = MIN(MIN(V,next->get_V0_squared()),operation->get_Vf_squared());
                next->set_V0_squared(V);
                operation->set_Vf_squared(V);
                operation = next;
                node = node->next;
            } 
            operation->set_Vf_squared(0);
        }
        operation_result_enum add_operation_to_queue(operation_class* operation){
            if(operation_queue.isFull())            return queue_full;
            if(!operation_queue.enqueue(operation)) return queue_full;
            if(operation->is_movement){
                movement_class* move = static_cast<movement_class*>(operation);
                for(uint8_t i = 0; i < Config::axis_count;i++){
                    //queue_state.position_steps[i] = move->absolute_steps[i];
                    planner_state.unit_vector_of_last_move[i] = move->unit_vector[i];
                    planner_state.absolute_position_steps[i] += move->delta_steps[i];
                }
            }
            plan(false);
            return success;
        }
        /*
        -- Order of operations ---
        Comment (including message)
    -    Set feed rate mode (G93, G94).
    -    Set feed rate (F).
 
    -> Change tool (M6).
    -> Spindle on or off (M3, M4, M5).
    -> Coolant on or off (M7, M8, M9).
    -    Enable or disable overrides (M48, M49).
    > Dwell (G4).
    -    Set active plane (G17, G18, G19).
    -    Set length units (G20, G21).
    -    Cutter radius compensation on or off (G40, G41, G42)
    -    Cutter length compensation on or off (G43, G49)
    -    Coordinate system selection (G54, G55, G56, G57, G58, G59, G59.1, G59.2, G59.3).
    -    Set path control mode (G61, G61.1, G64)
    -    Set distance mode (G90, G91).
        Set retract mode (G98, G99).
    > Go to reference location (G28, G30) or change coordinate system data (G10) or set axis offsets (G92, G92.1, G92.2, G94).
    > Perform motion (G0 to G3, G33, G73, G76, G80 to G89), as modified (possibly) by G53.
    >  Stop (M0, M1, M2, M30, M60).
        */
        
        operation_result_enum add_operation_to_queue(command_class* command){
            if(operation_queue.isFull(16)) return queue_full;
            if(command->parameter.size()==0)
                return invalid_operation;
            for(uint8_t i = 0; i < command->parameter.size();i++){
                keyValuePair pair = command->parameter[i];
                if(pair.key == "G"){
                    if(pair.value == 93)
                        planner_state.modal.feed_rate_mode = machine_state_class::G93;
                    if(pair.value == 94)
                        planner_state.modal.feed_rate_mode = machine_state_class::G93;
                    if(pair.value == 17)
                        planner_state.modal.plane= machine_state_class::plane_XY;
                    if(pair.value == 18)
                        planner_state.modal.plane = machine_state_class::plane_ZX;
                    if(pair.value == 19)
                        planner_state.modal.plane = machine_state_class::plane_YZ;
                    if(pair.value == 20)
                        planner_state.modal.units = units_in;
                    if(pair.value == 21)
                        planner_state.modal.units = units_mm; 
                    if(pair.value == 40)
                        planner_state.modal.cutter_radius_compentation = machine_state_class::off;
                    if(pair.value == 41)
                        planner_state.modal.cutter_radius_compentation = machine_state_class::left;
                    if(pair.value == 42)
                        planner_state.modal.cutter_radius_compentation = machine_state_class::right;
                    if(pair.value == 43)
                        planner_state.modal.cutter_length_compentation = machine_state_class::negative;
                    if(pair.value == 44)
                        planner_state.modal.cutter_radius_compentation = machine_state_class::positive;
                    if(pair.value == 49)
                        planner_state.modal.cutter_radius_compentation = machine_state_class::off;
                    if(pair.value >= 54 && pair.value <= 59)
                        planner_state.modal.coordinate_sytem = pair.value - 54;
                    if(pair.value > 59 && pair.value < 60)
                        planner_state.modal.coordinate_sytem = (uint8_t)((pair.value - 54.0f)*10.0f);
                    if(pair.value == 61)
                        planner_state.modal.path_control = machine_state_class::exact;
                    if(pair.value == 64)
                        planner_state.modal.path_control = machine_state_class::off;
                    if(pair.value == 90)
                        planner_state.modal.distance_mode = machine_state_class::absolute;
                    if(pair.value == 91)
                        planner_state.modal.distance_mode = machine_state_class::incremental;
                    if(pair.value == 0)
                        planner_state.modal.motion = machine_state_class::G0;
                    if(pair.value == 1)
                        planner_state.modal.motion = machine_state_class::G1;
                    if(pair.value == 2)
                        planner_state.modal.motion = machine_state_class::G2;
                    if(pair.value == 3)
                        planner_state.modal.motion = machine_state_class::G3;
                }
                if(pair.key == "M"){
                    if(pair.value == 48)
                        planner_state.feed_override_allowed = true;
                    if(pair.value == 49)
                        planner_state.feed_override_allowed = false;
                }
            }   

            bool coordinate_specified = false; // if there is a coordinate, that means a move was requested (maybe), we will use what ever is the current motion mode.
            // Handle parameters and get them in our standard units
            uint32_t parameter_in_command = 0;
            for(uint8_t i = 0; i < command->parameter.size();i++){
                keyValuePair pair = command->parameter[i];
                char p = pair.key[0];
                bitWrite(parameter_in_command,pair.key[0]-'A',1);
                switch(p){
                    // Feed is always units per minute
                    case 'A':
                        coordinate_specified = true;
                        sticky_parameters[_A_] = pair.value *  unit_convert[planner_state.modal.units][Config::machine_units];
                    break;
                    case 'B':
                        coordinate_specified = true;
                        sticky_parameters[_B_] = pair.value *  unit_convert[planner_state.modal.units][Config::machine_units];
                    break;
                    case 'C':
                        coordinate_specified = true;
                        sticky_parameters[_C_] = pair.value *  unit_convert[planner_state.modal.units][Config::machine_units];
                    break;
                    case 'D':
                        sticky_parameters[_D_] = pair.value *  unit_convert[planner_state.modal.units][Config::machine_units];
                    break;
                    case 'E':
                        if(planner_state.modal.feed_rate_mode == machine_state_class::inverse_time)
                            sticky_parameters[_E_] = 1.0f/pair.value;     // (3)
                        else
                            sticky_parameters[_E_] = pair.value; 
                        sticky_parameters[_E_] *= unit_convert[planner_state.modal.units][Config::machine_units];
                    break;
                    case 'F':
                        if(planner_state.modal.feed_rate_mode == machine_state_class::inverse_time)
                            sticky_parameters[_F_] = 1.0f/pair.value;     // (3)
                        else
                            sticky_parameters[_F_] = pair.value; 
                        sticky_parameters[_F_] *= unit_convert[planner_state.modal.units][Config::machine_units];
                    break;
                    case 'G':
                        sticky_parameters[_G_] = pair.value;
                    break;
                    case 'H':
                        sticky_parameters[_H_] = pair.value *  unit_convert[planner_state.modal.units][Config::machine_units];
                    break;
                    case 'I':
                        sticky_parameters[_I_] = pair.value *  unit_convert[planner_state.modal.units][Config::machine_units];
                    break;
                    case 'J':
                        sticky_parameters[_J_] = pair.value *  unit_convert[planner_state.modal.units][Config::machine_units];
                    break;
                    case 'K':
                        sticky_parameters[_K_] = pair.value *  unit_convert[planner_state.modal.units][Config::machine_units];
                    break;
                    case 'L':
                        sticky_parameters[_L_] = pair.value *  unit_convert[planner_state.modal.units][Config::machine_units];
                    break;
                    case 'M':
                        sticky_parameters[_M_] = pair.value;
                    break;
                    case 'N':
                        sticky_parameters[_N_] = pair.value;
                    break;
                    case 'O':
                        sticky_parameters[_O_] = pair.value;
                    break;
                    case 'P':
                        sticky_parameters[_P_] = pair.value;
                    break;
                    case 'Q':
                        sticky_parameters[_Q_] = pair.value;
                    break;
                    case 'R':
                        sticky_parameters[_L_] = pair.value *  unit_convert[planner_state.modal.units][Config::machine_units];
                    break;
                    case 'S':
                        sticky_parameters[_S_] = pair.value;
                    break;
                    case 'T':
                        sticky_parameters[_T_] = pair.value;
                    break;
                    case 'U':
                        coordinate_specified = true;
                        sticky_parameters[_U_] = pair.value *  unit_convert[planner_state.modal.units][Config::machine_units];
                    break;
                    case 'V':
                        coordinate_specified = true;
                        sticky_parameters[_V_]= pair.value * unit_convert[planner_state.modal.units][Config::machine_units];
                    break;
                    case 'W':
                        coordinate_specified = true;
                        sticky_parameters[_W_] = pair.value * unit_convert[planner_state.modal.units][Config::machine_units];
                    break;
                    case 'X':
                        coordinate_specified = true;
                        sticky_parameters[_X_] = pair.value *  unit_convert[planner_state.modal.units][Config::machine_units];
                    break;
                    case 'Y':
                        coordinate_specified = true;
                        sticky_parameters[_Y_]= pair.value * unit_convert[planner_state.modal.units][Config::machine_units];
                    break;
                    case 'Z':
                        coordinate_specified = true;
                        sticky_parameters[_Z_] = pair.value * unit_convert[planner_state.modal.units][Config::machine_units];
                    break;
                }         
            }
            // Change Tool
            
            for(uint8_t i = 0; i < command->parameter.size();i++){
                keyValuePair pair = command->parameter[i];
                if(pair.key == "M"){
                    if(pair.value == 6){
                        operation_class* operation = new M6;
                        operation_queue.enqueue(operation);
                    }
                }
            }
            // Spindle
            for(uint8_t i = 0; i < command->parameter.size();i++){
                keyValuePair pair = command->parameter[i];
                if(pair.key == "M"){
                    if(pair.value == 3){
                        operation_class* operation = new M3;
                        operation_queue.enqueue(operation);
                    }
                    if(pair.value == 4){
                        operation_class* operation = new M4;
                        operation_queue.enqueue(operation);
                    }
                    if(pair.value == 5){
                        operation_class* operation = new M5;
                        operation_queue.enqueue(operation);
                    }
                }
            }
            // Coolant
            for(uint8_t i = 0; i < command->parameter.size();i++){
                keyValuePair pair = command->parameter[i];
                if(pair.key == "M"){
                    if(pair.value == 7){
                        operation_class* operation = new M7;
                        operation_queue.enqueue(operation);
                    }
                    if(pair.value == 8){
                        operation_class* operation = new M8;
                        operation_queue.enqueue(operation);
                    }
                    if(pair.value == 9){
                        operation_class* operation = new M9;
                        operation_queue.enqueue(operation);
                    }
                }
            }
            // Dwell

            for(uint8_t i = 0; i < command->parameter.size();i++){
                keyValuePair pair = command->parameter[i];
                if(pair.key == "G"){
                    if(pair.value == 4){
                        operation_class* operation = new G4;
                        CNC_ENGINE::operation_result_enum r = operation->init(sticky_parameters);// operation_queue.getTailItemPtr());
                        if(r == success)
                            add_operation_to_queue(operation);
                    }
                } 
            }
    //> Go to reference location (G28, G30) or change coordinate system data (G10) or set axis offsets (G92, G92.1, G92.2, G94).
           
            for(uint8_t i = 0; i < command->parameter.size();i++){
                keyValuePair pair = command->parameter[i];
                if(pair.key == "G"){
                    if(pair.value == 28){   // Home
                        operation_class* operation = new G28;
                        CNC_ENGINE::operation_result_enum r = operation->init(sticky_parameters, parameter_in_command);
                        if(r == success){
                            add_operation_to_queue(operation);
                            console::logln("G28 - SUCCESS");
                        }
                        else{
                            console::logln("G28 - BAD: "+String(r));
                        }
                    }
                    if(pair.value == 30){   // Probe z axis

                    }
                    if(pair.value == 92){   // Set position, will give cooridnates, but not require move 
                        coordinate_specified = false;

                    }
                }
            }
    //> Perform motion (G0 to G3, G33, G73, G76, G80 to G89), as modified (possibly) by G53.
    
            if(coordinate_specified){
                if(planner_state.modal.motion==machine_state_class::G0){

                }
                else if(planner_state.modal.motion==machine_state_class::G1){

                        operation_class* operation = new G1;
                        CNC_ENGINE::operation_result_enum r = operation->init(sticky_parameters);
                        if(r == success){
                            add_operation_to_queue(operation);
                            console::logln("G1 - SUCCESS");
                        }
                        else{
                            console::logln("G1 - BAD: "+String(r));
                        }
                }
                else if(planner_state.modal.motion==machine_state_class::G2){
                    
                }
                else if(planner_state.modal.motion==machine_state_class::G3){
                    
                }
            }

    //>  Stop (M0, M1, M2, M30, M60).
            for(uint8_t i = 0; i < command->parameter.size();i++){
                keyValuePair pair = command->parameter[i];
                if(pair.key == "M"){
                    if(pair.value == 0){
                        M0* operation = new M0;
                        operation_queue.enqueue(operation);
                    }
                    if(pair.value == 1){
                        M1* operation = new M1;
                        operation_queue.enqueue(operation);
                    }
                    if(pair.value == 2 || pair.value == 30){
                        M2* operation = new M2;
                        operation_queue.enqueue(operation);
                    }
                } 
            }

            plan(false);
            console::logln("SUCCESS");
            return success;
        }
    };
    extern operation_controller_class operation_controller;
}