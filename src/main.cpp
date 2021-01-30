#include <Arduino.h>
//#include "config.h"
#include <RFX_Console.h> // Common serial port out interface
#include "CNCEngine.h"
#include <RFX_Server.h>
#include "esp32-hal-cpu.h"

#include "parsers/commandParser.h"

// Parse and pass the input stream to the appropriate handler
void parse(String input)
{
  PARSER::result_struct result = PARSER::parse(input);
  if (result.status == PARSER::ok)
  {
    if (result.command->parameter.size() == 0)
      return;
    if (result.command->parameter[0].key[0] == '>')
    {
      // Custom command
    }
    else
    {
      // Normal GRBL inputs assumed
      RFX_CNC::process_command(result.command);
    }
  }
  else
  {
    console.logln("Parsing line - "+PARSER::result_description[(uint8_t)result.status],console_class::error);
  }
}
TaskHandle_t taskHandle;
void setup()
{
  Serial.begin(SERIALBAUD);
  console.init(&Serial);
  console.timestamp = true;
  console.logln();
  console.bar(64);
  console.bar("RFX", 64);
  console.bar(64);
  console.logln();

  setCpuFrequencyMhz(240); //Set CPU clock to 240 MHz
  console.logln("CPU Set To (MHz): " + String(getCpuFrequencyMhz()) + "\t RAM (kB free): " + String(esp_get_free_heap_size() / 1000), console.routine);

  RFX_FILE_SYSTEM::init();
  RFX_Server::init();

  delay(1000);

  RFX_CNC::init();

  // Setup the input stream callback.
  RFX_Server::setWebSocketCallback(parse);
}
// Loop runs on Core 1
void loop()
{
// ESP32 has multiple cores and plays well with virtual tasks,
// if the target is an ESP32 we will use vtasks, else you need to service manually.
#ifndef ESP32
  RFX_Server::tend();
#endif
}
