// Import core functionality
#include <RFX_Console.h>        // Common serial port out interface
#include "RFX_Server.h"
#include <EEPROM.h>

#include "WiFi.h"
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <Update.h>


/*
################ Server - Isolated to Core 1 ################### 
*/

namespace RFX_Server{

    // https://tomeko.net/online_tools/cpp_text_escape.php?lang=en
    String template_start_head = "<!DOCTYPE html><html lang=\"en\"><head><title>RFX CONTROL</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";

    String template_style = "<style>:root {\n    --caution: #ffd26f;\n}\n\nbody {\n    font-family: Verdana, sans-serif;\n}\n\n.rfx-caution {\n    color: #000;\n    background-color: var(--caution) !important\n}\n\n.rfx-caution-invert {\n    color: var(--caution) !important;\n}\n\n.rfx-good {\n    color: #000;\n    background-color: #75eb5d !important\n}\n\n.rfx-good-invert {\n    color: #75eb5d !important;\n}\n\n.rfx-warning {\n    color: #000;\n    background-color: #d65656 !important\n}\n.rfx-panel {\n    border: 1px solid rgb(150, 149, 149);\n    border-radius: 4px;\n    margin: 8px 0 8px 0;\n    box-sizing: unset;\n  }\n  \n  .rfx-panel+.rfx-panel {\n    margin-top: -4px;\n  }\n  \n  .rfx-panel>*:first-child {\n    background: rgb(0,0,0);\n    color:white;\n    border-bottom: 1px solid #aaa;\n    padding: 2px 4px 2px 4px;\n    font-weight: bold;\n    display: flex;\n    align-items: center;\n    width: calc(100%-4px);\n  }\n  \n  .rfx-panel>*:not(first-child) {\n    padding: 8px;\n  }\n\ninput{\n    font-family: 'Courier New', monospace;\n    border: 1px solid #CCC;\n    width: calc(100% - 5px);\n    padding: 2px;\n    font-size: 1.2rem;\n    margin-bottom:8px;\n}\n\n.rfx-button {\n    display: block;\n    border-radius: 4px;\n    border: 2px solid #000;\n    transition-duration: 0.2s;\n    box-shadow: 0 2px 4px 0 rgba(0, 0, 0, 0.2), 0 1px 5px 0 rgba(0, 0, 0, 0.19);\n    cursor: pointer;\n    font-size: 1.2rem;\n    width: 100%;\n    margin-bottom:4px;\n}\n\n.rfx-button:hover {\n    box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 2px 5px 0 rgba(0, 0, 0, 0.19);\n    filter: brightness(110%)\n}\n\n.rfx-button:active {\n    box-shadow: 0 1px 2px 0 rgba(0, 0, 0, 0.2), 0 1px 2px 0 rgba(0, 0, 0, 0.19);\n    filter: brightness(90%)\n}\ninput[type=\"file\"] {\n  font-family: 'Courier New', monospace;\n  display: none;\n}\n.custom-file-upload {\n  display: inline-block;\n}\n.disabled{\n  cursor:not-allowed;\n  opacity:0.3;\n  pointer-events:none\n}\n.progress{\n    border-radius: 4px;\n    border: 2px solid #000;\n    cursor: pointer;\n    font-size: 1.2rem;\n    width: calc(100%-4px);\n    margin-bottom:4px;\n}\n</style>";
    String template_end_head = "</head>";

    String template_default = "<body>\n <div class=\"rfx-panel\" style=\"max-width: 400px; margin:auto\">\n <label id=\"header-label\">RFX-ESP</label>\n <div>\n <button class=\"rfx-button\" onclick=\"window.location.href='connect'\">WiFi Connection</a>\n <button class=\"rfx-button\" onclick=\"window.location.href='stats'\">System Stats</button>\n <button class=\"rfx-button\" onclick=\"window.location.href='update'\">Firmware Update</button>\n <button id=\"restart_button\" class=\"rfx-button\">Restart</button>\n </div>\n </div>\n</body>\n<script>\n document.getElementById('restart_button').addEventListener('click', event => {\n if (!confirm(\"Perform Hardware Restart (Power Cycle)?\"))\n return\n var xhr = new XMLHttpRequest();\n function bad_response() {\n\n }\n xhr.onload = () => {\n if (xhr.response === 'ok') {\n console.log(\"restart recieved\")\n }\n else {\n console.log(\"restart bad response\")\n }\n\n };\n xhr.onerror = () => {\n console.log(\"restart error\")\n }\n xhr.ontimeout = () => {\n console.log(\"restart timeout\")\n }\n xhr.timeout = 2000\n xhr.open(\"GET\", \"/server/restart\", true);\n xhr.setRequestHeader(\"Content-Type\", \"text/html\");\n xhr.send();\n })\n</script>";

    String template_connect = "<body>\n <div class=\"rfx-panel\" style=\"max-width: 400px; margin:auto\">\n <label id=\"header-label\">Wifi</label>\n <div>\n <label><b>Name on network</b></label>\n <input id=\"dns_name\" class=\"rfx-input\" type=\"text\" placeholder=\"Access server via name\">\n <label><b>SSID</b></label>\n <input id=\"ssid\" class=\"rfx-input\" type=\"text\" placeholder=\"Wifi Network Name\">\n <label><b>Password</b></label>\n <input id=\"password\" class=\"rfx-input\" type=\"password\" placeholder=\"Wifi Password\">\n <button id=\"submit-button\" class=\"rfx-button\" style=\"width:100%;\"\n onclick=\"submit()\">Submit</button>\n </div>\n </div>\n</body>\n\n<script>\n document.querySelectorAll('.rfx-input').forEach(item => {\n item.addEventListener('change', event => {\n console.log(\"change\")\n function mark(el) {\n with (el.classList) {\n remove(\"rfx-good\")\n add(\"rfx-caution\")\n remove(\"rfx-warning\")\n }\n }\n mark(item)\n mark(document.getElementById('submit-button'))\n document.getElementById('header-label').innerText = \"Wifi Settings: Need to submit\"\n })\n })\n function getServerInfo() {\n var xhr = new XMLHttpRequest();\n xhr.onload = () => {\n let response = JSON.parse(xhr.responseText)\n document.getElementById(\"dns_name\").placeholder = response.dns_name;\n document.getElementById(\"ssid\").placeholder = response.ssid;\n };\n xhr.onerror = () => {\n setTimeout(getServerInfo, 1000);\n }\n xhr.open(\"GET\", \"http://192.168.4.62/server/status\", true);\n xhr.setRequestHeader(\"Content-Type\", \"application/json\");\n xhr.send();\n }\n function submit() {\n let data = {};\n // Build json data packet \n document.querySelectorAll('.rfx-input').forEach(item => {\n data[item.id] = item.value;\n })\n var xhr = new XMLHttpRequest();\n function bad_response() {\n function mark(el) {\n with (el.classList) {\n remove(\"rfx-good\")\n remove(\"rfx-caution\")\n add(\"rfx-warning\")\n }\n }\n mark(document.getElementById('submit-button'))\n }\n xhr.onload = () => {\n if (xhr.response === 'ok') {\n function mark(el) {\n with (el.classList) {\n add(\"rfx-good\")\n remove(\"rfx-caution\")\n remove(\"rfx-warning\")\n }\n }\n mark(document.getElementById('submit-button'))\n document.getElementById('header-label').innerText = \"Wifi Settings: Saved\"\n document.querySelectorAll('.rfx-input').forEach(item => {\n item.classList.remove(\"rfx-good\")\n item.classList.remove(\"rfx-caution\")\n item.classList.remove(\"rfx-warning\")\n })\n }\n else {\n document.getElementById('header-label').innerText = \"Wifi Settings: Error- Bad Response\";\n bad_response()\n }\n\n };\n xhr.onerror = () => {\n document.getElementById('header-label').innerText = \"Wifi Settings: Error- Server\";\n bad_response()\n }\n xhr.ontimeout = () => {\n document.getElementById('header-label').innerText = \"Wifi Settings: Error- Server Unreachable\";\n bad_response()\n }\n xhr.timeout = 2000\n xhr.open(\"POST\", \"/server/settings\", true);\n xhr.setRequestHeader(\"Content-Type\", \"application/json\");\n console.log(JSON.stringify(data))\n xhr.send(JSON.stringify(data));\n }\n getServerInfo();\n</script>";
    String template_firmware = "<body>\n <div class=\"rfx-panel\" style=\"max-width: 400px; margin:auto\">\n <label id=\"header-label\">Firmware</label>\n <div>\n <label><b>Firmware: </b></label>\n <label id=\"firmware\" style=\"float:right\">NULL</label>\n </div>\n <div>\n <label><b>Version: </b></label>\n <label id=\"version\"style=\"float:right\">NULL</label>\n </div>\n <div>\n\n <button id=\"OTA_file_select\" class=\"rfx-button\" onclick=\"document.getElementById('fileinput').click()\">\n <input id=\"fileinput\" type='file' name='update' onchange=\"document.getElementById('OTA_Execute').classList.remove('disabled'); document.getElementById('progress_label').innerText=document.getElementById('fileinput').value; document.getElementById('progress_container').style.backgroundColor = 'lightgray'\">\n Select Firmware for Update\n </button>\n <div id= \"progress_container\" class=\"progress\" style=\"margin-top:8px; text-align: center; position: relative; height:26px; background-color: lightgray;\">\n <div id=\"progress\" style=\"position: absolute; background-color: rgb(74, 128, 172); width:0%; height:100% \"></div>\n <div id=\"progress_label\" style=\"position: absolute; top:0px; width:100%;text-align: center;\"></div>\n </div>\n <button id=\"OTA_Execute\" class=\"rfx-button disabled\" style=\"margin-top:8px\">Upload</button>\n </div>\n </div>\n</body>\n\n<script>\n document.querySelectorAll('.rfx-input').forEach(item => {\n item.addEventListener('change', event => {\n console.log(\"change\")\n function mark(el) {\n with (el.classList) {\n remove(\"rfx-good\")\n add(\"rfx-caution\")\n remove(\"rfx-warning\")\n }\n }\n mark(item)\n mark(document.getElementById('submit-button'))\n document.getElementById('header-label').innerText = \"Wifi Settings: Need to submit\"\n })\n })\n function getServerInfo() {\n var xhr = new XMLHttpRequest();\n xhr.onload = () => {\n let response = JSON.parse(xhr.responseText)\n document.getElementById(\"version\").innerText = response.version;\n document.getElementById(\"firmware\").innerText = response.firmware;\n };\n xhr.onerror = () => {\n setTimeout(getServerInfo, 1000);\n console.log(\"error\")\n }\n xhr.open(\"GET\", \"http://192.168.4.62/server/status\", true);\n xhr.setRequestHeader(\"Content-Type\", \"application/json\");\n xhr.send();\n }\n getServerInfo();\n document.getElementById('OTA_Execute').addEventListener('click', event => {\n var xhr = new XMLHttpRequest();\n function done(msg){\n document.getElementById('OTA_file_select').classList.remove('disabled');\n document.getElementById(\"progress_label\").innerText = msg;\n document.getElementById(\"progress\").style.width = 0;\n getServerInfo();\n }\n xhr.onload = () => {\n if (xhr.response === 'ok') {\n console.log(xhr.response);\n console.log(\"restart recieved\")\n done(\"Firmware Update Successful\");\n document.getElementById(\"progress_container\").style.backgroundColor = \"lightGreen\";\n }\n else {\n console.log(xhr.response);\n done(\"OTA Bad Response\")\n document.getElementById(\"progress_container\").style.backgroundColor = \"lightCoral\";\n }\n };\n xhr.onerror = () => {\n console.log(\"restart error\")\n done(\"Update Error\");\n document.getElementById(\"progress_container\").style.backgroundColor = \"lightCoral\";\n \n }\n xhr.ontimeout = () => {\n console.log(\"restart timeout\")\n done(\"Server Timeout\");\n document.getElementById(\"progress_container\").style.backgroundColor = \"lightCoral\";\n }\n xhr.upload.addEventListener('progress', function (evt) {\n if (evt.lengthComputable) {\n var per = evt.loaded / evt.total;\n console.log(per);\n document.getElementById(\"progress\").style.width = Math.round(per * 100) + \"%\";\n }\n \n })\n xhr.open(\"POST\", \"http://192.168.4.62/update\", true);\n let file = document.getElementById(\"fileinput\").files[0];\n var formData = new FormData();\n formData.append(\"update\", file, file.name);\n xhr.file = file;\n document.getElementById('fileinput').value='';\n document.getElementById('OTA_Execute').classList.add('disabled');\n document.getElementById('OTA_file_select').classList.add('disabled');\n document.getElementById(\"progress_label\").innerText = \"Uploading...\"\n xhr.send(formData);\n return false;\n })\n</script>";
    String template_stats = "<body>\n <div class=\"rfx-panel\" style=\"max-width: 400px; margin:auto\">\n <label id=\"header-label\">RFX-ESP</label>\n <div>\n <div style=\"margin-bottom: 8px;\">\n <label><b>Firmware / Ver: </b></label>\n <label id=\"version\" style=\"float:right\">NULL</label>\n <label style=\"float:right; padding:0 4px 0 4px\"> - </label>\n <label id=\"firmware\" style=\"float:right\">NULL</label>\n </div>\n\n <div class=\"rfx-panel\" style=\"max-width: 400px; margin:auto; margin-bottom: 8px;\">\n <label id=\"network-label\" style=\"color:black;background-color: lightgray;\">Network</label>\n <div>\n <div style=\"border-bottom: 2px solid lightgray;\">\n <label><b>IP / Name: </b></label>\n <label id=\"server_name\" style=\"float:right\">NULL</label>\n <label style=\"float:right; padding:0 4px 0 4px\"> - </label>\n <label id=\"server_ip\" style=\"float:right\">NULL</label>\n </div>\n <div style=\"border-bottom: 2px solid lightgray;\">\n <label><b>Mode: </b></label>\n <label id=\"server_mode\" style=\"float:right\">NULL</label>\n </div>\n <div style=\"border-bottom: 2px solid lightgray;\">\n <label><b>WiFi SSID: </b></label>\n <label id=\"server_ssid\" style=\"float:right\">NULL</label>\n </div>\n <div style=\"border-bottom: 2px solid lightgray;\">\n <label><b>Ping (msec): </b></label>\n <label style=\"float:right\"><b id=\"server_ping\" style=\"filter: brightness(80%);\">NULL</b></label>\n </div>\n </div>\n </div>\n <div class=\"rfx-panel\" style=\"max-width: 400px; margin:auto\">\n <label id=\"header-label\">CPU</label>\n <div>\n <div style=\"border-bottom: 2px solid lightgray;\">\n <label><b>CPU Freq (MHz): </b></label>\n <label id=\"server_freq\" style=\"float:right\">NULL</label>\n </div>\n <div style=\"border-bottom: 2px solid lightgray;\">\n <label><b>Up Time (sec): </b></label>\n <label id=\"up_time\" style=\"float:right\">NULL</label>\n </div>\n <div style=\"border-bottom: 2px solid lightgray;\">\n <label><b>Filesystem Used/Total (kb): </b></label>\n <label id=\"total_bytes\" style=\"float:right\">NULL</label>\n <label style=\"float:right; padding:0 4px 0 4px\"> / </label>\n <label id=\"used_bytes\" style=\"float:right\">NULL</label>\n </div>\n <div style=\"border-bottom: 2px solid lightgray;\">\n <label><b>RAM Used/Total(kb): </b></label>\n <label id=\"server_total_heap\" style=\"float:right\">NULL</label>\n <label style=\"float:right; padding:0 4px 0 4px\"> / </label>\n <label id=\"server_used_heap\" style=\"float:right\">NULL</label>\n </div>\n </div>\n </div>\n\n </div>\n </div>\n</body>\n<script>\n function update_server_status(status) {\n let color = \"lightCoral\";\n if (status == \"ok\") {\n color = \"lightGreen\"\n }\n if (status == \"caution\") {\n color = \"Khaki\"\n }\n document.getElementById(\"network-label\").style.backgroundColor = color\n document.getElementById(\"server_ping\").style.color = color\n }\n function getServerInfo() {\n var xhr = new XMLHttpRequest();\n let start_date = new Date();\n xhr.onload = () => {\n let end_date = new Date();\n let millis = end_date.getTime() - start_date.getTime();\n let response = JSON.parse(xhr.responseText)\n document.getElementById(\"firmware\").innerText = response.firmware;\n document.getElementById(\"version\").innerText = response.version;\n\n document.getElementById(\"server_ip\").innerText = response.ip;\n document.getElementById(\"server_name\").innerText = response.dns_name;\n document.getElementById(\"server_mode\").innerText = response.mode;\n document.getElementById(\"server_ssid\").innerText = response.ssid;\n document.getElementById(\"server_ping\").innerText = millis;\n\n document.getElementById(\"up_time\").innerText = Math.round(response.up_time / 1000)\n document.getElementById(\"total_bytes\").innerText = Math.round(response.total_bytes / 1000)\n document.getElementById(\"used_bytes\").innerText = Math.round(response.used_bytes / 1000)\n\n let total_heap = response.total_heap;\n let free_heap = response.free_heap;\n document.getElementById(\"server_total_heap\").innerText = Math.round(total_heap / 1000);\n document.getElementById(\"server_used_heap\").innerText = Math.round((total_heap-free_heap) / 1000);\n\n document.getElementById(\"server_freq\").innerText = response.freq;\n if (millis < 250)\n {\n update_server_status(\"ok\");\n document.getElementById(\"network-label\").innerText = \"Network\";\n }\n else if (millis < 750){\n update_server_status(\"caution\");\n document.getElementById(\"network-label\").innerText = \"Network\";\n }\n else{\n update_server_status(\"warning\");\n document.getElementById(\"network-label\").innerText = \"Network - Slow response (ping)\";\n }\n \n setTimeout(getServerInfo, 1000);\n };\n xhr.onerror = () => {\n update_server_status(\"warning\");\n document.getElementById(\"server_ping\").innerText = \"error\";\n document.getElementById(\"network-label\").innerText = \"Network - Error\";\n setTimeout(getServerInfo, 1000);\n }\n xhr.timeout = 2000;\n xhr.ontimeout = () => {\n update_server_status(\"warning\");\n document.getElementById(\"server_ping\").innerText = \"timeout\";\n document.getElementById(\"network-label\").innerText = \"Network - Timeout\";\n setTimeout(getServerInfo, 1000);\n document.getElementById(\"server\").classList.add(\"w3-disabled\");\n }\n xhr.open(\"GET\", \"/server/status\", true);\n xhr.setRequestHeader(\"Content-Type\", \"application/json\");\n xhr.send();\n }\n setTimeout(getServerInfo, 1000);\n</script>";



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



