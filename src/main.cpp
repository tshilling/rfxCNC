#include <Arduino.h>
#include "main.h"
#include "config.h"
#include "console.h"
#include "CNCEngine.h"
#include "server\RFX_Server.h"
#include "CNCFileSystem.h"
#include "esp32-hal-cpu.h"

void init_console(){
  Serial.begin(SERIALBAUD);
  console::init(&Serial);
}
void init_CPU(){
  setCpuFrequencyMhz(240); //Set CPU clock to 240 MHz
  console::log("CPU Set To (MHz): "+String(getCpuFrequencyMhz())+"\t RAM (kB free): "+String(esp_get_free_heap_size()/1000),console::routine);
}

TaskHandle_t taskHandle;
void setup()
{
  init_console();
  init_CPU();

  CNCFileSystem::init();
  RFX_Server::init();

  delay(1000);
  CNC_ENGINE::init();

  disableCore0WDT();
  disableCore1WDT();
}
// Loop runs on Core 1
void loop()
{
  // All services are handled by loops in CNC_Server and CNC_Engine
}
