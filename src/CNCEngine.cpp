
#include <Arduino.h>
#include "CNCEngineConfig.h"

#include "step_engine/step_engine.h"

#include "CNCEngine.h"
#include <RFX_Console.h> // Common serial port out interface

//#include "RFX_File_System.h"

#include "state/machineState.h"
#include "operations/operation_controller.h"

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
        if (bitRead(RFX_CNC::MACHINE::machine_state->critical_status_bits, i) == 0)
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
      for (uint8_t i = 0; i < Config::axis_count; i++)
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

  void printData()
  {
    console.log(String(millis()));
    console.log(String(MACHINE::machine_state->getVelocity()), 10);
    for (uint8_t i = 0; i < 6; i++)
    {
      console.log(String((float)MACHINE::machine_state->absolute_position_steps[i]), (i + 2) * 10); ///(float)CNC_ENGINE::Config::axis[i].stepsPerUnit,2));
    }
    console.logln();
  }
  void status_report()
  {

    String result = MACHINE::machine_mode_description[(uint8_t) MACHINE::machine_mode]+": ";

    for(uint8_t i=0;i<16;i++){
      result += String(bitRead(MACHINE::home_required,i));
    }
    result += "\tPos: ";
    for (uint8_t i = 0; i < Config::axis_count; i++)
    {
      result += String(MACHINE::machine_state->get_position_in_coordinates(i));
      if (i != Config::axis_count - 1)
        result += ", ";
    }
    console.logln(result);
  }
  void _engineLoop(void *parameter)
  {
    // Turn off watchdog, allows for long running processess
    disableCore0WDT();

    // TODO: Make this more module
    add_non_standard_endpoints();

    Config::init();
    MACHINE::init_machine_state();
    STEP_ENGINE::init();
    operation_controller.init(128);

    startTime = micros();
    unsigned long lp_previous_time = startTime;
    unsigned long sp_previous_time = startTime;
    unsigned long new_time = startTime;
    unsigned long short_delta = 0;
    unsigned long long_delta = 0;
    operation_class *operation = nullptr;
    for (;;)
    {
      vTaskDelay(10); // Not always garanteed to get to end of loop but must always have a vTaskDelay, keep at start of loop

      new_time = micros();
      operation = operation_controller.operation_queue.getHeadItemPtr();

      //###################################
      //###### Short Period Function ######
      short_delta = new_time - sp_previous_time;
      if (short_delta >= SHORT_PERIOD)
      {                                                             // 10 msec or 100Hz
        sp_previous_time = new_time - (short_delta - SHORT_PERIOD); // Deals with time variations, little bit of excess in one will result in a little short in the next

          
        MACHINE::scan_inputs(short_delta);
        MACHINE::handle_inputs();
      }
      if(MACHINE::home_required && MACHINE::machine_mode != MACHINE::home)
      {
        MACHINE::machine_mode = MACHINE::need_homing;
      }
      if (MACHINE::machine_mode >= MACHINE::run)
      {
        //###### Long Period Functions #######
        long_delta = new_time - lp_previous_time;
        if (long_delta >= LONG_PERIOD)
        {
          if (long_delta > LONG_PERIOD)
            long_delta = LONG_PERIOD;                               // 100 msec or 10Hz
          lp_previous_time = new_time - (long_delta - LONG_PERIOD); // Deals with time variations, little bit of excess in one will result in a little short in the next

          if (operation)
          {
            console.logln(operation->get_log());
          }
        }
        // ###### Operation service routine, non-movement ######
        if (operation)
        {
          if (!operation->execute_in_interrupt)
          {
            if (operation->execute())
            {
              operation_controller.operation_queue.dequeue();
              console.logln(operation->get_log());
              operation = nullptr;
            }
          }
        }
        else{
          MACHINE::machine_mode = MACHINE::idle;
          status_report();
        }
      }
    }
  }

  void process_command(PARSER::command_struct *commands)
  {

    if (commands->parameter.size() == 0)
      return;
    if(commands->parameter[0].key.length() == 0)
      return;
    switch(commands->parameter[0].key[0]){
      case '$':
        break;
      case 0x18:
        MACHINE::perform_emergency_stop(); // TODO, GRBL calls this a soft_reset, look into how that should be handled different
        status_report();
        return;
      case '*':
        MACHINE::perform_emergency_stop(); // TODO, this is a custom typable command for E-Stop. Decide
        status_report();
        return;
      case '~':
        MACHINE::perform_cycle_start();
        status_report();
        return;
      case '^':
        MACHINE::perform_unlock();
        status_report();
        return;
      case '?':
        status_report();
        return;
      case '!':
        MACHINE::perform_feed_hold();
        status_report();
        return;
    }
    RFX_CNC::operation_result_enum result = RFX_CNC::operation_controller.add_operation_to_queue(commands);
    console.logln("Operation: "+RFX_CNC::operation_result_description[(uint8_t)result]);
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
}; // namespace CNC_ENGINE
