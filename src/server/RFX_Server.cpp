#include <Arduino.h>
#include "main.h"
#include "config.h"
#include "CNCEngine.h"

#include "RFX_Server.h"

#include "WiFi.h"
#include "ESPAsyncWebServer.h"

#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <string.h>
#include "CNCFileSystem.h"
#include "console.h"
#include "RFXQueue.h"
#include "GCodeParser.h"
#include "operations\operation_controller.h"
#include "CNCEngine.h"
#include "state\machineState.h"

#include <ESPmDNS.h>
//#include "..\asyncTCP\ESPAsyncTCP.h"
#include "AsyncTCP.h"
#include "Update.h"

/*
################ Server - Isolated to Core 1 ################### 
*/

namespace RFX_Server{
    AsyncWebServer server(80);
    bool restart_requested = false;
    struct configStruct{
        String SSID="";
        String Password="";
        String dns_name = "rfx";
    } config;

    bool read_config_file(){
        console::logln("Reading Config File");
        File configFile = SPIFFS.open("/public/serverConfig.json", "r");
        if (configFile)
        {
            StaticJsonDocument<512> doc;
            DeserializationError error = deserializeJson(doc, configFile);
            if(!error)
            {
                if(!doc["ssid"].isNull())
                    config.SSID = String((const char*) doc["ssid"]);
                if(!doc["password"].isNull())
                    config.Password = String((const char*) doc["password"]);
                if(!doc["dns_name"].isNull())
                    config.dns_name = String((const char*) doc["dns_name"]);
                configFile.close();
                CNCFileSystem::show_file("/public/serverConfig.json");
                return true;
            }
            console::logln("Config error: "+String(error.c_str()));
        }
        else{
            console::logln("Config File Not Found", console::error);
        }
        configFile.close();
        return false;        
    }
    bool write_config_file(){
        console::logln("Writing server config file");
        File configFile = SPIFFS.open("/public/serverConfig.json", "w");
        if (configFile)
        {
            StaticJsonDocument<512> doc;
            doc["ssid"] = config.SSID;
            doc["password"] = config.Password;
            doc["dns_name"] = config.dns_name;
            serializeJson(doc, configFile);
            configFile.close();
            CNCFileSystem::show_file("/public/serverConfig.json");
            return true;
        }
        else
        {
            console::logln("Server config file not found", console::error);
        } 
        configFile.close();
        return false;  
    }

    void add_non_standard_endpoints(){
        server.on("/engine/status",HTTP_GET,[](AsyncWebServerRequest *request){
            StaticJsonDocument<200> doc;
            String binaryString = "";
            char binary[33];
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
            
            binary[32] = '\0';  // Null character for string termination

            doc["status"] = String(binary);
            doc["time"] = String(millis());
            String output = "";
            serializeJson(doc, output);
            request->send(200, "application/json", output);
        });
        server.on("/engine/api", HTTP_GET, [](AsyncWebServerRequest *request){
            int paramsNr = request->params();
            Serial.println(paramsNr);
            for(int i=0;i<paramsNr;i++){
                AsyncWebParameter* p = request->getParam(i);
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
    }
    void add_standard_endpoints(){
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(CNCFileSystem::fileSystem,"/index.html","text/html");

        }); 
        server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(CNCFileSystem::fileSystem,"/favicon.png","image/png");
        });  
        //dealing with zipped file
        /*
        server.on("/bootstrap.css", HTTP_GET, [](AsyncWebServerRequest *request){
            AsyncWebServerResponse *response = request->beginResponse(CNCFileSystem::fileSystem,"/public/bootstrap.min.css.gz","text/css", false);
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
        });   */
        server.onNotFound([](AsyncWebServerRequest *request) {
            if (request->method() == HTTP_OPTIONS) {
                request->send(200); // Removing this breaks Cors support.... don't know why
            } else {
                request->send(404);
            }
        });
        server.on("/server/status", HTTP_GET,[](AsyncWebServerRequest *request){
            StaticJsonDocument<512> doc;
            doc["ip"] = WiFi.localIP().toString();
            doc["dns_name"] = config.dns_name;
            doc["ssid"] = WiFi.SSID();
            if(WiFi.getMode()==WIFI_MODE_AP)
                doc["mode"] = "Access Point";
            if(WiFi.getMode()==WIFI_MODE_STA)
                doc["mode"] = "Station";
            if(WiFi.getMode()==WIFI_MODE_APSTA)
                doc["mode"] = "Access Point and Station";
            doc["heap"] = esp_get_free_heap_size();
            esp_chip_info_t info;
            esp_chip_info(&info);
            doc["cores"] = info.cores;
            if(info.model==1)
                doc["model"] = "ESP32";
            doc["revision"] = info.revision;
            doc["freq"] = getCpuFrequencyMhz();
            doc["up_time"] = millis();
            doc["total_bytes"] = String(SPIFFS.totalBytes());
            doc["used_bytes"] = String(SPIFFS.usedBytes());
            String output = "";
            serializeJson(doc, output);
            request->send(200, "application/json", output);
        });
        server.on("/server/settings",HTTP_POST,[](AsyncWebServerRequest *request){
            request->send(200, "text/plain", "ok");
        },NULL,[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
            StaticJsonDocument<512> doc;
            console::logln(deserializeJson(doc, data).c_str());

            if(!doc["ssid"].isNull()){
                 String value = String((const char*)doc["ssid"]);
                 if(value.length()>0)
                    config.SSID = value;
            }
            if(!doc["password"].isNull()){
                String value  = String((const char*)doc["password"]);
                 if(value.length()>0)
                    config.Password = value;
            }
            if(!doc["dns_name"].isNull()){
                String value  = String((const char*)doc["dns_name"]);
                 if(value.length()>0)
                    config.dns_name = value;
            }
            write_config_file();
        });
        server.on("/server/restart", HTTP_GET,[](AsyncWebServerRequest *request){
            console::logln("Restart received...");
            restart_requested = true;
            request->send(200, "text/html", "ok");
        });
        server.serveStatic("/",CNCFileSystem::fileSystem,"/public");
        //#############
        /*handling uploading firmware file */
        server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
            
            // the request handler is triggered after the upload has finished... 
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", (Update.hasError())?"fail":"ok");
            request->send(response);
            restart_requested = true;  // Tell the main loop to restart the ESP

        },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
            //Upload handler chunks in data

            if(!index){ // if index == 0 then this is the first frame of data
                console::logln("Firmware update started ("+filename+")...");
                
                // calculate sketch space required for the update
                uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
                if(!Update.begin(maxSketchSpace)){//start with max available size
                    Update.printError(*console::stream);
                }
            }
            //Write chunked data to the free sketch space
            if(Update.write(data, len) != len){
                    Update.printError(*console::stream);
            }
            if(final){ // if the final flag is set then this is the last frame of data
                if(Update.end(true)){ //true to set the size to the current progress
                    console::logln("Success, rebooting...");
                } else {
                    Update.printError(*console::stream);
                }
            }
        });

/*
        server.on("/update", HTTP_POST, [&](AsyncWebServerRequest *request) {
            // the request handler is triggered after the upload has finished... 
            // create the response, add header, and send response
            AsyncWebServerResponse *response = request->beginResponse((Update.hasError())?500:200, "text/plain", (Update.hasError())?"FAIL":"OK");
            response->addHeader("Connection", "close");
            response->addHeader("Access-Control-Allow-Origin", "*");
            request->send(response);
            restart_requested = true;
        }, [&](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) 
        {
                //Upload handler chunks in data
                if (!index) {
                    if(!request->hasParam("MD5", true)) {
                        return request->send(400, "text/plain", "MD5 parameter missing");
                    }
                    if(!Update.setMD5(request->getParam("MD5", true)->value().c_str())) {
                        return request->send(400, "text/plain", "MD5 parameter invalid");
                    }
                    int cmd = (filename == "filesystem") ? U_SPIFFS : U_FLASH;
                    if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) { // Start with max available size
                        Update.printError(Serial);
                        return request->send(400, "text/plain", "OTA could not begin");
                    }
                }

                // Write chunked data to the free sketch space
                if(len){
                    if (Update.write(data, len) != len) {
                        return request->send(400, "text/plain", "OTA could not begin");
                    }
                }
                    
                if (final) { // if the final flag is set then this is the last frame of data
                    if (!Update.end(true)) { //true to set the size to the current progress
                        Update.printError(Serial);
                        return request->send(400, "text/plain", "Could not end OTA");
                    }
                }else{
                    return;
                }
            });
        
        //#############
    }
    
    */
    }
    bool try_to_connect_to_station(){

        // Try to connect to exisiting network
        console::log("Connecting to: "+config.SSID);
        WiFi.begin(config.SSID.c_str(),config.Password.c_str());
        console::logln("Hostname: "+ String(WiFi.getHostname()));
        int i = 0;
        while (WiFi.status() != WL_CONNECTED) 
        {
            delay(500);
            console::log(".");
            i = i + 1;
            if(i > 20)  // Wait for 10sec
            {
                console::logln(" Could not connect.", console::error);
                return false;
            }
        }

        
        console::logln(" Success.");
        console::logln("IP:\t"+WiFi.localIP().toString());
        return true;
    }
    bool try_to_establish_access_point(){
        console::log("Establishing Access Point as: "+apSsid+"... ");
        if(WiFi.softAP(apSsid.c_str(), APPASSWORD)){
            console::logln("Done, access point wifi mode establish:");
            console::tabIndex++;
            console::logln("SSId:\t\t"+apSsid);
            console::logln("PW:\t\t"+String(APPASSWORD));
            console::logln("Local IP:\t"+WiFi.softAPIP().toString());
            console::tabIndex--;
            config.SSID = apSsid;
            return true;
        }
        else
        {
            console::logln("Failed");
        }
        return false;
    }
    bool init_wifi(){
        // If there is anything set in the config file for SSID
        if(config.SSID.length()!=0){
            if(try_to_connect_to_station())
                return true;
        }
        return try_to_establish_access_point();
    }
     
    String processor(const String& var){
        /*
        if(var == "SSID")
        {
            return config.SSID;
        }
        if(var == "MODE")
        {
            wifi_mode_t M = WiFi.getMode();
            if(M==WIFI_AP)
                return F("WiFi Host");
            if(M==WIFI_STA)
                return F("WiFi Client");
            if(M==WIFI_AP_STA)
                return F("WiFi Host & Client");
        }
        if(var == "IP")
            return WiFi.localIP().toString();
        */
        return String();
    }

    void init_server(){       
 
        add_standard_endpoints();
        add_non_standard_endpoints();
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");
        server.begin();
    }
    void init_file_system(){
        CNCFileSystem::init();
        delay(100);
        read_config_file();
        delay(100);
    }
    
    //
    void server_loop(void * parameter){
        init_file_system();
        init_wifi();
        if(!MDNS.begin(config.dns_name.c_str())) {
            Serial.println("Error starting mDNS");
        }
        init_server();
        int count_down = 4000;
        long last_time = micros();
        for(;;) {
            //long delta = millis() - last_time;
            //if(delta >= 1000)
            //{
                if(restart_requested){
                    //count_down -= delta;
                    //console::logln("Restarting in: " + String(count_down));
                    //if(count_down<=0){
                        ESP.restart();
                    //}
                }
               // last_time = millis();
            //}
            vTaskDelay(50);  // needed to keep watchdog timer happy
        }
    }

    TaskHandle_t taskHandle;
    void init(){  
        // Append chip ID to Defined apSsid
        // NOTE: As of writting, ESP32 does not have a chipID like the esp8266.  Instead we have to use the mac address
        uint8_t sta_mac[6];
        esp_efuse_mac_get_default(sta_mac);        
        apSsid = String(APSSID)+String(sta_mac[0])+String(sta_mac[1])+String(sta_mac[2])+String(sta_mac[3])+String(sta_mac[4])+String(sta_mac[5]);
        
        xTaskCreatePinnedToCore(
            server_loop, /* Function to implement the task */
            "Server", /* Name of the task */
            10000,  /* Stack size in words */
            NULL,  /* Task input parameter */
            1,  /* Priority of the task */
            &taskHandle,  /* Task handle. */
            0); /* Core where the task should run */
    }
};



