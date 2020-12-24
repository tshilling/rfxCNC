#include <Arduino.h>
#include "main.h"
#include "config.h"
#include "CNCEngine.h"
#include "CNCServer.h"

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
//#include "WebServer.h"

#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <string.h>
#include "CNCFileSystem.h"
#include "Console.h"
#include "RFXQueue.h"
#include "GCodeParser.h"
#include "operations\operation_controller.h"
#include "CNCEngine.h"
#include "state\machineState.h"


/*
################ Server - Isolated to Core 1 ################### 
*/

namespace CNCServer{
    rfxQueue<String> buffer_in;
    Stream* stream;     // Parent type for Serial Streams (And all streams)
    struct configStruct{
        String SSID="";
        String Password="";
    };

    configStruct config;
    
    bool tryToConnectToStation(){
        // Try to connect to exisiting network
        stream->print("Connecting to ");
        stream->print(config.SSID);

        WiFi.begin(config.SSID.c_str(),config.Password.c_str());

        int i = 0;
        while (WiFi.status() != WL_CONNECTED) 
        {
            delay(500);
            stream->print(".");
            i = i + 1;
            if(i > 20)  // Wait for 10sec
            {
                stream->println();
                stream->print("ERROR: could not connect to ");
                stream->println(config.SSID);
                return false;
            }
        }
        stream->println();
        stream->print("SUCCESS: WiFi connected, IP: ");
        stream->println(WiFi.localIP());
        return true;
    }
    bool tryToEstablishAP(){
        stream->print("Establishing Access Point as: ");
        stream->println(apSsid);
        if(WiFi.softAP(apSsid.c_str(), APPASSWORD)){
            IPAddress myIP = WiFi.softAPIP();
                   stream->println("Done");
                    stream->println("AP Mode Wifi");
                    stream->print("  SSId:\t\t");
                    stream->println(apSsid);
                    stream->print("  PW:\t\t");
                    stream->println(APPASSWORD);
                    stream->print("  Local IP:\t");
                    stream->println(myIP);
                    config.SSID = apSsid;
            return true;
        }
        else
        {
            stream->println("ERROR: Could not establish Access Point");
        }
        return false;
    }
    bool initWifi(){
        // If there is anything set in the config file for SSID
        if(config.SSID.length()!=0)
            if(tryToConnectToStation()){
                return true;
            }
        if(tryToEstablishAP()){
            return true;
        }
        return false;
    }
     
    // Stores LED state
    String ledState;
    String processor(const String& var){
        if(var == "SSID")
        {
            return config.SSID;
        }
        if(var == "MODE")
        {
            WiFiMode_t M = WiFi.getMode();
            if(M==WIFI_AP)
                return F("WiFi Host");
            if(M==WIFI_STA)
                return F("WiFi Client");
            if(M==WIFI_AP_STA)
                return F("WiFi Host & Client");
        }
        if(var == "IP")
            return WiFi.localIP().toString();
        return String();
    }


    void printFile(const char* params){
        stream->println();
        stream->print("===== ");
        stream->print(params);
        stream->println(" =====");
      if(params==nullptr)
        return;
        File f = SPIFFS.open(params,"r");
        while(f.available()){
            stream->write(f.read());
        }
          stream->println();
          f.close();
        stream->print("======");
        for(int i = 0;i<strlen(params);i++){
            stream->print("=");
        }
        stream->println("======");
    }

    bool readConfigFile(){
        stream->println("Reading Config File");
        File configFile = SPIFFS.open("/public/config.json", "r");
        if (configFile)
        {
            StaticJsonDocument<512> doc;
            DeserializationError error = deserializeJson(doc, configFile);
            if(!error)
            {
                const char* tSSID = doc["ssid"];
                const char* tPW = doc["password"];
                if(tSSID!=NULL)
                    config.SSID = String(tSSID);
                if(tPW!=NULL)
                    config.Password = String(tPW);
                configFile.close();
                printFile("/config.json");
                return true;
            }
        }
        else{
            stream->println("\tERROR: Config File Not Found");
        }
        configFile.close();
        return false;        
    }
    bool writeConfigFile(){
        stream->println("Writing Config File");
        File configFile = SPIFFS.open("/public/config.json", "w");
        if (configFile)
        {
            StaticJsonDocument<512> doc;
            const char* tSSID = config.SSID.c_str();
            const char* tPW = config.Password.c_str();
            doc["ssid"] = tSSID;
            doc["password"] = tPW;
            serializeJson(doc, configFile);
            configFile.close();
            printFile("/config.json");
            return true;
        }
        else
        {
            stream->println("\tERROR: Config File Not Found");
        }
        
        configFile.close();
        return false;  
    }
    
    AsyncWebServer server(80);
    //WebServer server(80);
    //This functions returns a String of content type
    String getContentType(String filename) {
        if (filename.endsWith(".htm")) { //check if the string filename ends with ".htm"
            return "text/html";
        } else if (filename.endsWith(".html")) {
            return "text/html";
        } else if (filename.endsWith(".css")) {
            return "text/css";
        } else if (filename.endsWith(".js")) {
            return "application/javascript";
        } else if (filename.endsWith(".png")) {
            return "image/png";
        } else if (filename.endsWith(".gif")) {
            return "image/gif";
        } else if (filename.endsWith(".jpg")) {
            return "image/jpeg";
        } else if (filename.endsWith(".ico")) {
            return "image/x-icon";
        } else if (filename.endsWith(".xml")) {
            return "text/xml";
        } else if (filename.endsWith(".pdf")) {
            return "application/x-pdf";
        } else if (filename.endsWith(".zip")) {
            return "application/x-zip";
        } else if (filename.endsWith(".gz")) {
            return "application/x-gzip";
        }
        return "text/plain";
    }
    int disable_count = 0;
    
    void initServer(){  
            
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){

            console::logln("index.html");
            request->send(CNCFileSystem::fileSystem,"/index.html","text/html");

        }); 
        server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){

            console::logln("favicon.ico");
            request->send(CNCFileSystem::fileSystem,"/favicon.png","image/png");

        });  
        server.on("/bootstrap.css", HTTP_GET, [](AsyncWebServerRequest *request){
            AsyncWebServerResponse *response = request->beginResponse(CNCFileSystem::fileSystem,"/public/bootstrap.min.css.gz","text/css", false);
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
        });   
        server.onNotFound([](AsyncWebServerRequest *request) {
            if (request->method() == HTTP_OPTIONS) {
                request->send(200);
            } else {
                request->send(404);
            }
        });
        server.on("/serverStatus", HTTP_GET,[](AsyncWebServerRequest *request){
            StaticJsonDocument<200> doc;
            doc["IP"] = WiFi.localIP().toString();
            doc["SSID"] = WiFi.SSID();
            if(WiFi.getMode()==WIFI_MODE_AP)
                doc["MODE"] = "Access Point";
            if(WiFi.getMode()==WIFI_MODE_STA)
                doc["MODE"] = "Station";
            if(WiFi.getMode()==WIFI_MODE_APSTA)
                doc["MODE"] = "Access Point and Station";
            doc["HEAP"] = esp_get_free_heap_size();
            esp_chip_info_t info;
            esp_chip_info(&info);
            doc["CORES"] = info.cores;
            if(info.model==1)
                doc["MODEL"] = "ESP32";
            doc["REVISION"] = info.revision;
            doc["FREQ"] = getCpuFrequencyMhz();
            String output = "";
            serializeJson(doc, output);
            request->send(200, "application/json", output);
        });
        server.on("/settings",HTTP_POST,[](AsyncWebServerRequest *request){
            request->send(200, "text/plain", "ok");
        },NULL,[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
            StaticJsonDocument<200> doc;
            deserializeJson(doc, data);

            String ssid = doc["ssid"];
            String password = doc["password"];

            console::logln("ssid: "+String(ssid));
            console::logln("password: "+String(password));
            config.SSID = ssid;
            config.Password = password;
            writeConfigFile();
        });
        server.on("/api", HTTP_GET, [](AsyncWebServerRequest *request){
            int paramsNr = request->params();
            Serial.println(paramsNr);
            for(int i=0;i<paramsNr;i++){
                AsyncWebParameter* p = request->getParam(i);
                Serial.print("Param name: ");
                Serial.println(p->name());
                Serial.print("Param value: ");
                Serial.println(p->value());
                Serial.println("------");
                if(p->name() == "code"){
                    CNC_ENGINE::command_class command = CNC_ENGINE::parse_gcode(p->value());
                    CNC_ENGINE::operation_result_enum result = CNC_ENGINE::operation_controller.add_operation_to_queue(&command);
                    if(result == CNC_ENGINE::success){
                        CNC_ENGINE::machine_state.is_active = true;
                        request->send(200, "text/plain", "ok");
                        return;
                    }
                    else{
                        request->send(200, "text/plain", String(CNC_ENGINE::operation_result_string[result]));
                        return;
                    }
                }
            }
            request->send(200, "text/plain", "ok");
        });
        server.on("/keepalive",HTTP_GET,[](AsyncWebServerRequest *request){
            StaticJsonDocument<200> doc;
            String binaryString = "";
            char binary[32];
            for(uint8_t i = 0; i < 32;i++){
                if(bitRead(CNC_ENGINE::machine_state.status_bits,i)==0){
                    binary[i] = '0';
                    //binaryString+="0";
                }
                else{
                    binary[i] = '1';
                    //binaryString+="1";
                }
            }
            doc["status"] = String(binary);
            doc["time"] = String(millis());
            String output = "";
            serializeJson(doc, output);
            request->send(200, "application/json", output);
        });
        server.serveStatic("/",CNCFileSystem::fileSystem,"/public");

        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");
        server.begin();
    }
    void initFileSystem(){
        CNCFileSystem::init();
        delay(100);
        readConfigFile();
        delay(100);
    }
    
    //
    void _serverLoop(void * parameter){
        //vTaskEndScheduler();
        //buffer_in.resize_queue(32);
        initFileSystem();
        initWifi();
        initServer();
        for(;;) {
            /*
            if(!buffer_in.isEmpty()){
                String* line = buffer_in.getHeadItemPtr();
                if(line!=nullptr)
                    if(xQueueSend(server_to_engine_queue, &line,portMAX_DELAY)==pdTRUE){
                        buffer_in.dequeue();
                    }
            }
            */
            //server.handleClient();
            vTaskDelay(1);  // needed to keep watchdog timer happy
        }
    }

    TaskHandle_t taskHandle;
    void init(Stream *_stream){  
        stream = _stream;
        if(stream!=nullptr)
        {
            Serial.begin(SERIALBAUD);
            stream = &Serial;
        }

        // Append chip ID to Defined apSsid
        // NOTE: As of writting, ESP32 does not have a chipID like the esp8266.  Instead we have to use the mac address
        uint8_t sta_mac[6];
        esp_efuse_mac_get_default(sta_mac);        
        apSsid = String(APSSID)+String(sta_mac[0])+String(sta_mac[1])+String(sta_mac[2])+String(sta_mac[3])+String(sta_mac[4])+String(sta_mac[5]);
        
        xTaskCreatePinnedToCore(
            _serverLoop, /* Function to implement the task */
            "Server", /* Name of the task */
            10000,  /* Stack size in words */
            NULL,  /* Task input parameter */
            1,  /* Priority of the task */
            &taskHandle,  /* Task handle. */
            0); /* Core where the task should run */
    }
};