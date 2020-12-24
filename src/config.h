#pragma once

#ifndef config_h
#define config_h

    #define SERIALBAUD  115200
    #define APPASSWORD  "rfxsetup"
    #define APSSID      "RFXSETUP" // A space and the unique chip ID will automatically be appended to APSSID by init function

    #define stateBufferLength 32

    #define server_to_engine_queue_size 1024

#endif