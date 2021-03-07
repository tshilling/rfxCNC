//#include "config.h"
#include <RFX_Console.h> // Common serial port out interface
#include "CNCEngine.h"
#include <RFX_Server.h>
#include "esp32-hal-cpu.h"
#include <rom/rtc.h>
#include "nuts_and_bolts.h"
#include "parsers/commandParser.h"


// Parse and pass the input stream to the appropriate handler
void print_reset_reason(RESET_REASON reason)
{
  switch ( reason)
  {
    case 1 : console.logln ("POWERON_RESET");break;          /**<1,  Vbat power on reset*/
    case 3 : console.logln ("SW_RESET");break;               /**<3,  Software reset digital core*/
    case 4 : console.logln ("OWDT_RESET");break;             /**<4,  Legacy watch dog reset digital core*/
    case 5 : console.logln ("DEEPSLEEP_RESET");break;        /**<5,  Deep Sleep reset digital core*/
    case 6 : console.logln ("SDIO_RESET");break;             /**<6,  Reset by SLC module, reset digital core*/
    case 7 : console.logln ("TG0WDT_SYS_RESET");break;       /**<7,  Timer Group0 Watch dog reset digital core*/
    case 8 : console.logln ("TG1WDT_SYS_RESET");break;       /**<8,  Timer Group1 Watch dog reset digital core*/
    case 9 : console.logln ("RTCWDT_SYS_RESET");break;       /**<9,  RTC Watch dog Reset digital core*/
    case 10 : console.logln ("INTRUSION_RESET");break;       /**<10, Instrusion tested to reset CPU*/
    case 11 : console.logln ("TGWDT_CPU_RESET");break;       /**<11, Time Group reset CPU*/
    case 12 : console.logln ("SW_CPU_RESET");break;          /**<12, Software reset CPU*/
    case 13 : console.logln ("RTCWDT_CPU_RESET");break;      /**<13, RTC Watch dog Reset CPU*/
    case 14 : console.logln ("EXT_CPU_RESET");break;         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : console.logln ("RTCWDT_BROWN_OUT_RESET");break;/**<15, Reset when the vdd voltage is not stable*/
    case 16 : console.logln ("RTCWDT_RTC_RESET");break;      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : console.logln ("NO_MEAN");
  }
}

void verbose_print_reset_reason(RESET_REASON reason)
{
  switch ( reason)
  {
    case 1  : console.logln ("Vbat power on reset");break;
    case 3  : console.logln ("Software reset digital core");break;
    case 4  : console.logln ("Legacy watch dog reset digital core");break;
    case 5  : console.logln ("Deep Sleep reset digital core");break;
    case 6  : console.logln ("Reset by SLC module, reset digital core");break;
    case 7  : console.logln ("Timer Group0 Watch dog reset digital core");break;
    case 8  : console.logln ("Timer Group1 Watch dog reset digital core");break;
    case 9  : console.logln ("RTC Watch dog Reset digital core");break;
    case 10 : console.logln ("Instrusion tested to reset CPU");break;
    case 11 : console.logln ("Time Group reset CPU");break;
    case 12 : console.logln ("Software reset CPU");break;
    case 13 : console.logln ("RTC Watch dog Reset CPU");break;
    case 14 : console.logln ("For APP CPU, reset by PRO CPU");break;
    case 15 : console.logln ("Reset when the vdd voltage is not stable");break;
    case 16 : console.logln ("RTC Watch dog reset digital core and rtc module");break;
    default : console.logln ("NO_MEAN");
  }
}
TaskHandle_t taskHandle;
void setup()
{
  Serial.begin(SERIALBAUD);
  Serial.setRxBufferSize(SERIAL_SIZE_RX);
  console.init(&Serial);
  console.timestamp = false;

  console.logln("CPU0 reset reason:");
  console.tabIndex++;
  print_reset_reason(rtc_get_reset_reason(0));
  verbose_print_reset_reason(rtc_get_reset_reason(0));
  console.tabIndex--;

  console.logln("CPU1 reset reason:");
  console.tabIndex++;
  print_reset_reason(rtc_get_reset_reason(1));
  verbose_print_reset_reason(rtc_get_reset_reason(1));
  console.tabIndex--;

  setCpuFrequencyMhz(240); //Set CPU clock to 240 MHz

  RFX_FILE_SYSTEM::init();
  //RFX_Server::init();
  //delay(1000);

  RFX_CNC::init();

  // Setup the input stream callback.
  // RFX_Server::setWebSocketCallback(RFX_CNC::process_line_in);
}
// Loop runs on Core 1
void loop()
{

}
