#pragma once

#ifndef CNCFileSystem_h
#define CNCFileSystem_h

//#include "SPIFFS.h"
#include "SPIFFS.h"

#include <ArduinoJson.h>
namespace CNCFileSystem{
    extern FS fileSystem;
    extern bool ready;
    bool init();
    bool show_file(String filename);
};
#endif