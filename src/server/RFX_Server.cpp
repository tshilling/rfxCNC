// Import core functionality
#include <console.h>        // Common serial port out interface
#include "RFX_Server.h"
#include "../RFX_File_System.h"  
#include "sections.h"
#include <EEPROM.h>

#include "WiFi.h"
#include <ArduinoJson.h>
#include <ESPmDNS.h>
//#include "AsyncTCP.h"
#include <Update.h>
/*
################ Server - Isolated to Core 1 ################### 
*/

namespace RFX_Server{
    AsyncWebServer server(80);
    bool restart_requested = false;
    struct configStruct{
        String SSID= DEFAULT_SSID;
        String Password= DEFAULT_PASSWORD;
        String dns_name = DEFUALT_DNS_NAME;
        String username = DEFAULT_USERNAME;
        String user_password = DEFAULT_USER_PASSWORD;
    } config;
    bool write_config(){
        console.log("Writing EEPROM...");
        EEPROM.write(0,'~'); //Mark that a write has occurred
        int i = 1;
        EEPROM.writeString(i,config.SSID);
        i+=config.SSID.length()+1;
        EEPROM.writeString(i,config.Password);
        i+=config.Password.length()+1;
        EEPROM.writeString(i,config.dns_name);
        i+=config.dns_name.length()+1;
        EEPROM.writeString(i,config.username);
        i+=config.username.length()+1;
        EEPROM.writeString(i,config.user_password);
        i+=config.user_password.length()+1;
        console.logln("done");
        return true;
    }
    bool read_config(){
        console.log("Reading EEPROM...");
        if(EEPROM.read(0) !='~'){
            console.logln("EEPROM is fresh, initializing...");
            write_config();
        }
        int i = 1;
        config.SSID = EEPROM.readString(i);
        i+=config.SSID.length()+1;
        config.Password = EEPROM.readString(i);
        i+=config.Password.length()+1;
        config.dns_name = EEPROM.readString(i);
        i+=config.dns_name.length()+1;
        config.username = EEPROM.readString(i);
        i+=config.username.length()+1;
        config.user_password = EEPROM.readString(i);
        i+=config.user_password.length()+1;
        console.logln("done");
        return true;
    }
    

    void add_non_standard_endpoints(){
       /*
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
        */
    }
    void add_standard_endpoints(){
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(RFX_FILE_SYSTEM::fileSystem,"/index.html","text/html");
        }); 
        server.on("/connect", HTTP_GET, [](AsyncWebServerRequest *request){
            String connect = template_start_head + template_style + template_end_head + template_connect;
            request->send(200,"text/html",connect);
        }); 
        server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
            String connect = template_start_head + template_style + template_end_head + template_firmware;
            request->send(200,"text/html",connect);
        }); 
        server.on("/default", HTTP_GET, [](AsyncWebServerRequest *request){
            String connect = template_start_head + template_style + template_end_head + template_default;
            request->send(200,"text/html",connect);
        }); 
        server.on("/stats", HTTP_GET, [](AsyncWebServerRequest *request){
            String connect = template_start_head + template_style + template_end_head + template_stats;
            request->send(200,"text/html",connect);
        }); 
        server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(RFX_FILE_SYSTEM::fileSystem,"/favicon.png","image/png");
        });  
        //dealing with zipped file
        /*
        server.on("/bootstrap.css", HTTP_GET, [](AsyncWebServerRequest *request){
            AsyncWebServerResponse *response = request->beginResponse(RFX_FILE_SYSTEM::fileSystem,"/public/bootstrap.min.css.gz","text/css", false);
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
                
            doc["free_heap"] = String(ESP.getFreeHeap());// esp_get_free_heap_size();
            doc["total_heap"] = String(ESP.getHeapSize());
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
            doc["version"] = String(VERSION);
            doc["firmware"] = String(FIRMWARE);
            String output = "";
            serializeJson(doc, output);
            request->send(200, "application/json", output);
        });
        server.on("/server/settings",HTTP_POST,[](AsyncWebServerRequest *request){
            request->send(200, "text/plain", "ok");
        },NULL,[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
            StaticJsonDocument<512> doc;
            console.logln(deserializeJson(doc, data).c_str());

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
            write_config();
        });
        server.on("/server/restart", HTTP_GET,[](AsyncWebServerRequest *request){
            console.logln("Restart received...");
            restart_requested = true;
            request->send(200, "text/html", "ok");
        });        
        server.on("/fs/internal", HTTP_GET,[](AsyncWebServerRequest *request){
            int paramsNr = request->params();
            Serial.println(paramsNr);
            DynamicJsonDocument doc(2048);
            //StaticJsonDocument<4096> doc;
            doc["total_bytes"] = String(SPIFFS.totalBytes());
            doc["used_bytes"] = String(SPIFFS.usedBytes());
            JsonArray filelist = doc.createNestedArray("files");
            File root = SPIFFS.open("/public");
            File file = root.openNextFile();
            while(file){
                JsonObject jsonFile = filelist.createNestedObject();
                jsonFile["name"] = String(file.name());
                jsonFile["size"] = file.size();
                jsonFile["is_dir"] = file.isDirectory();
                file = root.openNextFile();
            }
            String output = "";
            serializeJsonPretty(doc, output);
            request->send(200, "application/json", output);
        });
        
        server.serveStatic("/",RFX_FILE_SYSTEM::fileSystem,"/public");

        /*handling uploading firmware file */
        server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
            // the request handler is triggered after the upload has finished... 
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", (Update.hasError())?"fail":"ok");
            request->send(response);
            restart_requested = true;  // Tell the main loop to restart the ESP

        },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
            //Upload handler chunks in data

            if(!index){ // if index == 0 then this is the first frame of data
                console.logln("Firmware update started ("+filename+")...");
                
                // calculate sketch space required for the update
                uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
                if(!Update.begin(maxSketchSpace)){//start with max available size
                    Update.printError(*console.stream);
                }
            }
            //Write chunked data to the free sketch space
            if(Update.write(data, len) != len){
                    Update.printError(*console.stream);
            }
            if(final){ // if the final flag is set then this is the last frame of data
                if(Update.end(true)){ //true to set the size to the current progress
                    console.logln("Success, rebooting...");
                } else {
                    Update.printError(*console.stream);
                }
            }
        });
    }
    bool try_to_connect_to_station(){
        // Try to connect to exisiting network
        console.logln("Connecting to: "+config.SSID);
        WiFi.begin(config.SSID.c_str(),config.Password.c_str());
        console.logln("Hostname: "+ config.dns_name);
        int i = 0;
        while (WiFi.status() != WL_CONNECTED) 
        {
            delay(500);
            console.log(".");
            i = i + 1;
            if(i > 20)  // Wait for 10sec
            {
                console.logln(" Could not connect.", console.error);
                return false;
            }
        }
        console.logln(" Success.");
        console.logln("IP:\t"+WiFi.localIP().toString());
        return true;
    }
    bool try_to_establish_access_point(){
        console.log("Establishing Access Point as: "+apSsid+"... ");
        if(WiFi.softAP(apSsid.c_str(), APPASSWORD)){
            console.logln("Done, access point wifi mode establish:");
            console.tabIndex++;
            console.logln("SSId:\t\t"+apSsid);
            console.logln("PW:\t\t"+String(APPASSWORD));
            console.logln("Local IP:\t"+WiFi.softAPIP().toString());
            console.tabIndex--;
            config.SSID = apSsid;
            return true;
        }
        else
        {
            console.logln("Failed");
        }
        return false;
    }
    bool init_wifi(){
        if(config.dns_name.length()==0)
            config.dns_name = DEFUALT_DNS_NAME;
        WiFi.disconnect();
        WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);  // This is a MUST!
        if (!WiFi.setHostname(config.dns_name.c_str())) {
            Serial.println("Hostname failed to configure");
        }
        if(!MDNS.begin(config.dns_name.c_str())) {
            Serial.println("Error starting mDNS");
        }
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
            return config.SSID;
        */
        return String();
    }

    void init_server(){       
        
        add_standard_endpoints();
        add_non_standard_endpoints();
        #ifdef ALLOW_CORS
            DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
            DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");
        #endif
        server.begin();
    }
    void init_file_system(){
        RFX_FILE_SYSTEM::init();
        delay(100);
        read_config();
        delay(100);
    }
    
    //
    void server_loop(void * parameter){
        EEPROM.begin(512);
        init_file_system();
        init_wifi();
        init_server();
        for(;;) {
            if(restart_requested){
                ESP.restart();
            }
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



