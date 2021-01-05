#include "Arduino.h"
#include "CNCEngineConfig.h"

#include "step_engine/step_engine.h"

#include "main.h"
#include "CNCEngine.h"
#include "GCodeParser.h"
#include <console.h>        // Common serial port out interface

#include "RFX_File_System.h"

#include "state/machineState.h"
#include "operations/operation_controller.h"
/*
    CORE:   0
*/

namespace CNC_ENGINE{

    rfxQueue<command_class> command_buffer;

    unsigned long startTime = 0;
    TaskHandle_t taskHandle;

    void printData(){

        console.log(String(millis()));       
        console.log(String(CNC_ENGINE::machine_state.getVelocity()),10); 
        for(uint8_t i = 0; i < 6;i++){
            console.log(String((float)CNC_ENGINE::machine_state.absolute_position_steps[i]),(i+2)*10);///(float)CNC_ENGINE::Config::axis[i].stepsPerUnit,2));
        }
        console.logln();
    }
    uint32_t old_status = 0;
    void _engineLoop(void * parameter){
        Config::init();

        init_machine_state();
        step_engine::init();
        console.logln("Axis Count: "+ String(Config::axis_count));
            for(uint8_t i=0;i<Config::axis_count;i++){
                console.log(String(Config::axis[i].id)+" ");
            }
        command_buffer.resize_queue(32);      
        operation_controller.init(128);
        command_class command = parse_gcode("G1X5F10");
        operation_controller.add_operation_to_queue(&command);
        //command = parse_gcode("G1Y10F10");
        //operation_controller.add_operation_to_queue(&command);

        startTime = micros();
        //machine_state.is_active = true;
        for(;;) {
            if(machine_state.status_bits != 0){
                machine_state.velocity_squared = 0; // reset velocity to zero
            }
            if(machine_state.is_active){
                operation_class* operation = operation_controller.operation_queue.getHeadItemPtr();
                if(operation){
                    console.logln(operation->get_log());
                    if(!operation->execute_in_interrupt){
                        if(operation->execute()){
                            operation_controller.operation_queue.dequeue();
                        }
                    }
                }
            }
            if(old_status != machine_state.status_bits){
                for(uint8_t i = 0; i<32;i++){
                    if(bitRead(machine_state.status_bits,i)==1){
                        console.log(" 1");
                    }
                    else{
                        console.log(" 0");
                    }
                }
                console.logln();
                old_status = machine_state.status_bits;
            }
            vTaskDelay(50);
        }
    }
    void init(){
        // Initiate task on core
        xTaskCreatePinnedToCore(
            _engineLoop, /* Function to implement the task */
            "CNCEngine", /* Name of the task */
            10000,  /* Stack size in words */
            NULL,  /* Task input parameter */
            2,  /* Priority of the task */
            &taskHandle,  /* Task handle. */
            1); /* Core where the task should run */
    }
};
