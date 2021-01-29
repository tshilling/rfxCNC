// Import core functionality
#include <RFX_Console.h>        // Common serial port out interface
#include "RFX_Server.h"
#include <EEPROM.h>

#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <Update.h>
//#include "sections.h"

// Import required libraries
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif

/*
################ Server - Targetted to run on Core 1, same as setup and loop ################### 
*/
// New //
#include <WiFiClient.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#define USE_SERIAL Serial
/////////
namespace RFX_Server{
    WebServer server(80);
    WebSocketsServer webSocket = WebSocketsServer(81);
    void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            console.logln("["+String(num)+"] Disconnected");
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
				webSocket.sendTXT(num, "\nConnected to Server Console...\n");
                console.logln(String(num)+": Connected from "+String(ip[0])+"."+String(ip[1])+"."+String(ip[2])+"."+String(ip[3]));
            }
            break;
        case WStype_TEXT:
            
            if(_wsCallback)
                _wsCallback(String((const char*)payload));
            break;
        case WStype_BIN:
            console.logln("["+String(num)+"] get binary length: " + String(length));
            break;
        default:
            break;
    }

}
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
        EEPROM.commit();
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
    
    String getContentType(String filename) {
        if (server.hasArg("download")) {
            return "application/octet-stream";
        } else if (filename.endsWith(".htm")) {
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
        } else if (filename.endsWith(".json")) {
            return "application/json";
        }
        
        return "text/plain";
    }

    bool exists(String path){
            bool yes = false;
            File file = RFX_FILE_SYSTEM::fileSystem.open(path, "r");
            if(!file.isDirectory()){
                yes = true;
            }
            file.close();
            return yes;
    }
    bool handleFileRead(String path) {
        String contentType = getContentType(path);
        String pathWithGz = path + ".gz";
        if (exists(pathWithGz) || exists(path)) {
            if (exists(pathWithGz)) {
            path += ".gz";
            }
            File file = RFX_FILE_SYSTEM::fileSystem.open(path, "r");
            server.streamFile(file, contentType);
            file.close();
            return true;
        }
        else
        {
            server.send(404, "text/plain", "FileNotFound");
        }
        return false;
    }
    
    void sendCrossOriginHeader(){
        //Serial.println(F("sendCORSHeader"));
        //server.sendHeader("Access-Control-Allow-Origin","*");


        server.sendHeader(F("Access-Control-Max-Age"), F("600"));
        server.sendHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS"));
        server.sendHeader(F("Access-Control-Allow-Headers"), F("*"));
        server.send(204);
    }
    void setCrossOrigin(){
        //server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
        //server.sendHeader(F("Access-Control-Max-Age"), F("600"));
        //server.sendHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS"));
        //server.sendHeader(F("Access-Control-Allow-Headers"), F("*"));
    };
    
    String used_sketch_space;
    String total_sketch_space;

    void add_standard_endpoints(){
        
        used_sketch_space = String(ESP.getSketchSize()/1000);
        total_sketch_space = String((ESP.getSketchSize()+ESP.getFreeSketchSpace())/1000);

        server.on("/", [](){
            setCrossOrigin();
            handleFileRead("/index.html");
        });
        /*
        server.on("/update", HTTP_GET, [](){
            setCrossOrigin();
            String result = head_html;// template_start_head + template_style + template_end_head + template_default;
            result += update_html;
            server.send(200, "text/html", result);
        });
        server.on("/connect", HTTP_GET, [](){
            setCrossOrigin();
            String connect = head_html;// template_start_head + template_style + template_end_head + template_default;
            connect += connect_html;
            server.send(200,"text/html",connect);
        }); 
        server.on("/stats", HTTP_GET, [](){
            setCrossOrigin();
            String connect = head_html;// template_start_head + template_style + template_end_head + template_default;
            connect += stats_html;
            server.send(200,"text/html",connect);
        }); */
        server.on("/server/ping",HTTP_OPTIONS, sendCrossOriginHeader);
        server.on("/server/ping", HTTP_GET,[](){
            server.send(200, "text/plain", "ok");
        });
        server.on("/server/status",HTTP_OPTIONS, sendCrossOriginHeader);
        server.on("/server/status", HTTP_GET,[](){
            setCrossOrigin();
            //server.sendHeader("Access-Control-Allow-Origin","*");
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
            
            doc["used_heap"] = String((ESP.getHeapSize()-ESP.getFreeHeap())/1000);// esp_get_free_heap_size();
            doc["total_heap"] = String(ESP.getHeapSize()/1000);
            
            doc["used_psram"] = String((ESP.getPsramSize()-ESP.getFreePsram())/1000);
            doc["total_psram"] = String((ESP.getPsramSize())/1000);
            
            doc["used_sketch_space"] = used_sketch_space;
            doc["total_sketch_space"] = total_sketch_space;

            esp_chip_info_t info;
            esp_chip_info(&info);
            doc["cores"] = info.cores;
            if(info.model==1)
                doc["model"] = "ESP32";
            doc["revision"] = info.revision;
            doc["freq"] = getCpuFrequencyMhz();
            doc["up_time"] = String(millis()/1000);
            doc["total_kbytes"] = String(SPIFFS.totalBytes()/1000);
            doc["used_kbytes"] = String(SPIFFS.usedBytes()/1000);
            doc["version"] = String(VERSION);
            doc["firmware"] = String(FIRMWARE);
            String output = "";
            serializeJson(doc, output);
            server.send(200, "application/json", output);
        });       
        
        server.on("/server/settings",HTTP_OPTIONS, sendCrossOriginHeader);
        server.on("/server/settings",HTTP_POST,[](){
            setCrossOrigin();
            server.send(200, "text/plain", "ok");
            String postBody = server.arg("plain");
            StaticJsonDocument<512> doc;
            console.logln(deserializeJson(doc, postBody).c_str());

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

        server.on("/update",HTTP_OPTIONS, sendCrossOriginHeader);
        server.on("/update", HTTP_POST, []()        {
            setCrossOrigin();
            server.sendHeader("Connection", "close");
            if(!Update.hasError()){
                server.send(200, "text/plain", "ok");
                delay(1000);
                ESP.restart();
            }
            else{
                server.send(200, "text/plain", "fail");
            }
            }, []() {
            HTTPUpload& upload = server.upload();
            if (upload.status == UPLOAD_FILE_START) {
                Serial.setDebugOutput(true);
                Serial.printf("Update: %s\n", upload.filename.c_str());
                if (!Update.begin()) { //start with max available size
                Update.printError(Serial);
                }
            } else if (upload.status == UPLOAD_FILE_WRITE) {
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
                }
            } else if (upload.status == UPLOAD_FILE_END) {
                if (Update.end(true)) { //true to set the size to the current progress
                Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
                } else {
                Update.printError(Serial);
                }
                Serial.setDebugOutput(false);
            } else {
                Serial.printf("Update Failed Unexpectedly (likely broken connection): status=%d\n", upload.status);
            }
            });
        server.on("/favicon.ico", HTTP_GET,[](){
            setCrossOrigin();
            handleFileRead("/favicon.png");
            //server.send(200,"image/png",favicon);
            //request->send(RFX_FILE_SYSTEM::fileSystem,"/favicon.png","image/png");
        }); 

        server.onNotFound([]() {
            setCrossOrigin();
            //if (!handleFileRead(server.uri())) {
                //setCrossOrigin();
                server.send(404, "text/plain", "FileNotFound");
            //}
        });
        server.serveStatic("/", RFX_FILE_SYSTEM::fileSystem, "/","max-age=31536000");
        server.on("/server/restart", HTTP_GET,[](){
            setCrossOrigin();
            console.logln("Restart received...");
            server.send(200, "text/html", "ok");
            delay(500);
            ESP.restart();
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

    void init_server(){       
        add_standard_endpoints();
        #ifdef ALLOW_CORS
            server.enableCORS(true);
            //DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
            //DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");
        #endif
        server.begin();
    }
    void init_file_system(){
        RFX_FILE_SYSTEM::init();
        delay(100);
        read_config();
        delay(100);
    }
    

    long previous_time = millis();
    void tend(){
        server.handleClient();
        //webSocket.loop();
                
        if(restart_requested){
            ESP.restart();
        }  
        if(millis()-previous_time > 1000){
            previous_time = millis();
        }
    }
    void websocket_loop(void * parameter){
        for(;;) {
            webSocket.loop();
            vTaskDelay(10);  // needed to keep watchdog timer happy
        }
    }
    void server_loop(void * parameter){
        for(;;) {
            //tend();
            server.handleClient();
                    
            if(restart_requested){
                ESP.restart();
            }  
            if(millis()-previous_time > 1000){
                previous_time = millis();
                //console.log(".");
            }
            vTaskDelay(1);  // needed to keep watchdog timer happy
        }
    }

    TaskHandle_t taskHandle;
    TaskHandle_t taskHandleWs;
    void socketSerialOut(String input){
        webSocket.broadcastTXT(input.c_str(),input.length());
    }
    
    void (*_wsCallback)(String);
    void setWebSocketCallback(void (*wsCallback)(String)){
        _wsCallback = wsCallback;
    }
   void init(){  
        console.setCallback(socketSerialOut);
        // Append chip ID to Defined apSsid
        // NOTE: As of writting, ESP32 does not have a chipID like the esp8266.  Instead we have to use the mac address
        uint8_t sta_mac[6];
        esp_efuse_mac_get_default(sta_mac);        
        apSsid = String(APSSID)+String(sta_mac[0])+String(sta_mac[1])+String(sta_mac[2])+String(sta_mac[3])+String(sta_mac[4])+String(sta_mac[5]);

        EEPROM.begin(512);
        init_file_system();
        init_wifi();
        init_server();

        webSocket.begin();
        webSocket.onEvent(webSocketEvent);
        
        #ifdef ESP32
            disableCore1WDT();
            xTaskCreatePinnedToCore(
                server_loop,    // Function to implement the task
                "Server",       // Name of the task
                20000,          // Stack size in words
                NULL,           // Task input parameter
                1,              // Priority of the task
                &taskHandle,    // Task handle.
                1);             // Core where the task should run
            xTaskCreatePinnedToCore(
                websocket_loop,    // Function to implement the task
                "Websocket",       // Name of the task
                20000,          // Stack size in words
                NULL,           // Task input parameter
                1,              // Priority of the task
                &taskHandleWs,    // Task handle.
                1);             // Core where the task should run
        #endif
        
    }
};



