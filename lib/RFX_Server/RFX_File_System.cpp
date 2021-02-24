
#include "SPIFFS.h"
//#include "FFat.h"
#include <ArduinoJson.h>
#include <RFX_Console.h>
#include "RFX_File_System.h"
namespace RFX_FILE_SYSTEM{
    bool ready = false;
    FS fileSystem = SPIFFS;
    
    bool init(){
        if(ready == true)
            return true;
        if(!SPIFFS.begin()){
            //console.logln("SPIFFS File system failed to mount",console.error);
            return false;
        }  
        ready = true;
        return true;
    }
    bool show_file(String filename){
        console.logln("");
        console.bar(filename,100);
        if(ready)
        {
            File f = fileSystem.open(filename,"r");
            if(!f){
                console.logln(filename + " not Found",console.error);
            }
            else
            {
                while(f.available()){
                    console.write(f.read());
                }
                f.close();
                console.logln("");
                console.bar(100);
                return true;
            }
        }
        console.logln("");
        console.bar("",100);
        return false;
    }
};
