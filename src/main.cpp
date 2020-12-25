#include <Arduino.h>
#include "main.h"
#include "config.h"
#include "console.h"
#include "CNCEngine.h"
#include "CNCServer.h"
#include "esp32-hal-cpu.h"
#include "CNCFileSystem.h"
#include "queue.h"
#include "RFXQueue.h"

#include "operations/bresenham.h"

#include "step_engine/step_engine.h"

void initConsole(){
  Serial.begin(SERIALBAUD);
  console::init(&Serial);
}
void initCPU(){
  uint32_t freq = 240;
  setCpuFrequencyMhz(freq); //Set CPU clock to 80MHz fo example
  console::log("CPU Set To: ",console::routine);
  console::log(String(freq),console::routine);
  console::log("\t\tRead as: ",console::routine);
  console::logln(String(getCpuFrequencyMhz()),console::routine); //Get CPU clock
  console::logln("RAM (free bytes): " + String(esp_get_free_heap_size()));
}

int queueSize = 20;
QueueHandle_t   server_to_engine_queue;
RingbufHandle_t server_to_engine_buffer;


TaskHandle_t taskHandle;
void setup()
{
  initConsole();
  initCPU();
  CNCFileSystem::init();
  disableCore0WDT();
  //disableCore1WDT();
  CNCServer::init();
  delay(1000);
  CNC_ENGINE::init();

}
// Loop runs on Core 1
void loop()
{
 
}
