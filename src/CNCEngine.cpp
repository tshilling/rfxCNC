
#include <Arduino.h>
#include "CNCEngineConfig.h"

#include "step_engine/step_engine.h"

#include "CNCEngine.h"
#include <RFX_Console.h> // Common serial port out interface

//#include "RFX_File_System.h"

#include "state/machineState.h"
#include "operations/operation_controller.h"
#include "operations/command_block.h"

#include <RFX_Server.h>
/*
    CORE:   0
*/
#define LONG_PERIOD 100000
#define SHORT_PERIOD 1000

namespace RFX_CNC
{
  void add_non_standard_endpoints()
  {

    RFX_Server::server.on("/engine/status", HTTP_OPTIONS, RFX_Server::sendCrossOriginHeader);
    RFX_Server::server.on("/engine/status", HTTP_GET, []() {
      //RFX_Server::setCrossOrigin();
      StaticJsonDocument<512> doc;
      String binaryString = "";
      char binary[33];
      for (uint8_t i = 0; i < 32; i++)
      {
        if (bitRead(RFX_CNC::MACHINE::critical_status_bits, i) == 0)
        {
          binary[i] = '0';
          //binaryString+="0";
        }
        else
        {
          binary[i] = '1';
          //binaryString+="1";
        }
      }

      binary[32] = '\0'; // Null character for string termination

      doc["status"] = String(binary);
      doc["time"] = String(millis());
      JsonArray axis_array = doc.createNestedArray("axis");
      //JsonArray axis_array = doc.to<JsonArray>();
      for (uint8_t i = 0; i < config.axis.size(); i++)
      {
        JsonObject axis = axis_array.createNestedObject();
        axis["position"] = MACHINE::machine_state->absolute_position_steps[i];
      }

      String output = "";
      serializeJson(doc, output);
      RFX_Server::server.send(200, "application/json", output);
    });
    RFX_Server::server.on("/engine/api", HTTP_POST, []() {
      console.logln("param rx'd: ");
      RFX_Server::server.send(200, "text/plain", "ok");
    });

    RFX_Server::server.on("/engine/api", HTTP_OPTIONS, RFX_Server::sendCrossOriginHeader);
    RFX_Server::server.on("/engine/api", HTTP_GET, []() {
      RFX_Server::setCrossOrigin();
      console.logln("param rx'd: ");
      RFX_Server::server.send(200, "text/plain", "ok");
    });
  }
  // rfx_queue<command_class> command_buffer;

  unsigned long startTime = 0;
  TaskHandle_t taskHandle;

  
  void _engineLoop(void *parameter)
  {
    // Turn off watchdog, allows for long running processess
    disableCore0WDT();

    // TODO: Make this more module
    add_non_standard_endpoints();

    config.init();
    MACHINE::init_machine_state();
    STEP_ENGINE::init();
    operation_controller.init(64);

    MACHINE::perform_unlock();

    unsigned long sp_previous_time = startTime;

    operation_class *operation = nullptr;

    adaptor.send_welcome();
    for (;;)
    {
      vTaskDelay(1); // Not always garanteed to get to end of loop but must always have a vTaskDelay, keep at start of loop
      while (Serial.available()){
        if(!adaptor.push_stream_in(Serial.read()))
          break;
      }
      adaptor.service();
      //###################################
      //###### Short Period Function ######
      unsigned long short_delta = micros() - sp_previous_time;
      if (short_delta >= SHORT_PERIOD)
      {                                                             // 10 msec or 100Hz
        sp_previous_time = sp_previous_time + SHORT_PERIOD; // Deals with time variations, little bit of excess in one will result in a little short in the next
        MACHINE::scan_inputs(short_delta);
        MACHINE::handle_inputs();
        MACHINE::handle_outputs();
      }
      MACHINE::set_machine_mode();

      if (MACHINE::machine_mode >= MACHINE::run)
      {
        operation = operation_controller.operation_queue.getHeadItemPtr();      
        if (operation){
          //if(operation->block->modal_flag > mg_motion || operation->block->modal_flag < mg_motion)
            operation->execute(MACHINE::machine_state);
        }
        if(operation->block->modal_flag==0){
            operation_controller.operation_queue.dequeue();
        }
      }
    }
  }
 
  void init()
  {
    //RFX_Server::setWebSocketCallback(process_string);
    // Initiate task on core
    xTaskCreatePinnedToCore(
        _engineLoop, /* Function to implement the task */
        "CNCEngine", /* Name of the task */
        20000,       /* Stack size in words */
        NULL,        /* Task input parameter */
        2,           /* Priority of the task */
        &taskHandle, /* Task handle. */
        0);          /* Core where the task should run */
  }
}; // namespace RFX_CNC
