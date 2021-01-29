#pragma once

#ifndef RFX_Server_h
#define RFX_Server_h

#include <Arduino.h>
#include "RFX_File_System.h"  
//#include "ESPAsyncWebServer.h"
#include <WebServer.h>

#define APPASSWORD  "rfxsetup"
#define APSSID      "RFXSETUP"

#define DEFAULT_SSID            "Figment"
#define DEFAULT_PASSWORD        "306101105"
#define DEFAULT_USERNAME        "admin"
#define DEFAULT_USER_PASSWORD   "admin"
#define DEFUALT_DNS_NAME        "rfx"

#define FIRMWARE "RFX CORE A"
#define VERSION "0.1.1"

#define ALLOW_CORS
namespace RFX_Server{
    static String apSsid;
    //extern AsyncWebServer server;
    extern WebServer server;
    void init();
    void  _serverLoop(void * parameter);
    void socketSerialOut(String input);
    void tend();
    void sendCrossOriginHeader();
    void setCrossOrigin();    
    extern void (*_wsCallback)(String);
    void setWebSocketCallback(void (*wsCallback)(String));
};

#endif