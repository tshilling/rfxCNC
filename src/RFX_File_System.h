#pragma once

#ifndef RFX_FILE_SYSTEM_h
#define RFX_FILE_SYSTEM_h

#include "SPIFFS.h"
#include <ArduinoJson.h>

namespace RFX_FILE_SYSTEM{
    extern FS fileSystem;
    extern bool ready;
    bool init();
    bool show_file(String filename);
};
#endif