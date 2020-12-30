#include "CNCEngineConfig.h"
#include "Arduino.h"
#include "CNCFileSystem.h"
#include "console.h"
#include <ArduinoJson.h>
#include "CNCHelpers.h"

namespace CNC_ENGINE{
    namespace Config
    {       
        step_engine_struct step_engine_config;

        bool good_config = false;
        units_enum machine_units = units_mm;
        String machine_name = "Default Name";
        u_char axis_count = 0; 
        axisStruct axis[axisCountLimit];

        bool init(){
            if(!CNCFileSystem::ready)
                CNCFileSystem::init();
            if(readConfigFile()){
                console::logln("Config file, \"engineConfig.json\" opened",console::routine);
            }
            else
            {
                console::logln("Could not open config file, \"engineConfig.json\"",console::error);
            }
            for(uint8_t i = 0; i < Config::axis_count;i++){
                Config::axis[i].max_distance = 2147483647 / Config::axis[i].steps_per_unit;
            }
            // Configure Pins
            for(uint8_t i = 0; i < Config::axis_count;i++){
                if(Config::axis[i].step_pin >= 0)
                    pinMode(Config::axis[i].step_pin,OUTPUT);

                if(Config::axis[i].dir_pin >= 0)
                    pinMode(Config::axis[i].dir_pin,OUTPUT);

                if(Config::axis[i].enable_pin >= 0)
                    pinMode(Config::axis[i].enable_pin,OUTPUT);

                if(Config::axis[i].limit_pin_min >= 0)
                    pinMode(Config::axis[i].limit_pin_min,INPUT);

                if(Config::axis[i].limit_pin_max >= 0)
                    pinMode(Config::axis[i].limit_pin_max,INPUT);

                if(Config::axis[i].home_pin >= 0)
                    pinMode(Config::axis[i].home_pin,INPUT);

            }

            return CNCFileSystem::ready;
        }
        int getUnitIndex(String units){
            int unitIndex = 0;
            for(int i = 0; i < 5;i++)
            {
                if(unitIdentifiers[i].equals(units)){
                    unitIndex = i;
                    break;
                }
            }
            return unitIndex;
        }
        bool readConfigFile(){
            good_config = false;
            console::tabIndex++;

            File configFile = CNCFileSystem::fileSystem.open("/public/engineConfig.json", "r");
            console::logln("Reading engineCongig.json");
            if (configFile)
            {
                StaticJsonDocument<4096> doc;
                DeserializationError error = deserializeJson(doc, configFile);
                if(error){
                    console::logln("An Error Occurrred: "+String(error.c_str()));
                }
                
                if(!error){
                    doc["machine_name"].isNull()? Config::machine_name="none"       : Config::machine_name = String((const char*)doc["machine_name"]);
                    doc["machine_units"].isNull()?Config::machine_units = units_mm  : Config::machine_units = (units_enum) getUnitIndex(doc["machine_units"]);
                    
                    if(!doc["axis"].isNull())
                    {
                        JsonArray axis = doc["axis"];
                        Config::axis_count = axis.size();
                        if(Config::axis_count!=0){
                            for(uint8_t i = 0; i<Config::axis_count;i++){
                                axis[i]["axis_id"].isNull()?        Config::axis[i].id = 0                  : Config::axis[i].id = ((const char*)axis[i]["axis_id"])[0];
                                axis[i]["follow"].isNull()?         Config::axis[i].follow = 0              : Config::axis[i].follow = ((const char*)axis[i]["follow"])[0];
                                axis[i]["is_rotary"].isNull()?      Config::axis[i].is_rotary = false       : Config::axis[i].is_rotary = axis[i]["is_rotary"];
                                axis[i]["is_discrete"].isNull()?    Config::axis[i].is_discrete = false     : Config::axis[i].is_discrete = axis[i]["is_discrete"];
                                axis[i]["home_negative"].isNull()?  Config::axis[i].home_negative = false   : Config::axis[i].home_negative = axis[i]["home_negative"];
                                axis[i]["hard_limit"].isNull()?     Config::axis[i].hard_limit = false      : Config::axis[i].hard_limit = axis[i]["hard_limit"];
                                axis[i]["soft_limit"].isNull()?     Config::axis[i].soft_limit = false      : Config::axis[i].soft_limit = axis[i]["soft_limit"];
                                
                                axis[i]["min"].isNull()?                        Config::axis[i].min = 0 :                       Config::axis[i].min = axis[i]["min"];
                                axis[i]["max"].isNull()?                        Config::axis[i].max = 0 :                       Config::axis[i].min = axis[i]["max"];
                                axis[i]["home"].isNull()?                       Config::axis[i].home = 0 :                      Config::axis[i].home = axis[i]["home"];
                                axis[i]["home_feed_fine"].isNull()?             Config::axis[i].home_feed_fine = 0 :            Config::axis[i].home_feed_fine = axis[i]["home_feed_fine"];
                                axis[i]["home_feed_coarse"].isNull()?           Config::axis[i].home_feed_coarse = 0 :          Config::axis[i].home_feed_coarse = axis[i]["home_feed_coarse"];
                                axis[i]["home_switch_debounce_msec"].isNull()?  Config::axis[i].home_switch_debounce_msec = 0 : Config::axis[i].home_switch_debounce_msec = axis[i]["home_switch_debounce_msec"];
                                axis[i]["home_pulloff_units"].isNull()?         Config::axis[i].home_pulloff_units = 0 :        Config::axis[i].home_pulloff_units = axis[i]["home_pulloff_units"];

                                axis[i]["steps_per_unit"].isNull()?             Config::axis[i].steps_per_unit = 0 :            Config::axis[i].steps_per_unit = axis[i]["steps_per_unit"];
                                axis[i]["max_feed_units_per_sec"].isNull()?     Config::axis[i].max_feed_units_per_sec = 0 :    Config::axis[i].max_feed_units_per_sec = axis[i]["max_feed_units_per_sec"];
                                axis[i]["acceleration"].isNull()?               Config::axis[i].acceleration = 0 :              Config::axis[i].acceleration = axis[i]["acceleration"];
                                
                                axis[i]["dir_pin"].isNull()?                    Config::axis[i].dir_pin = -1 :                   Config::axis[i].dir_pin = axis[i]["dir_pin"];
                                axis[i]["dir_pin_invert"].isNull()?             Config::axis[i].dir_pin_invert = false :        Config::axis[i].dir_pin_invert = axis[i]["dir_pin_invert"];
                                axis[i]["step_pin"].isNull()?                   Config::axis[i].step_pin = -1 :                  Config::axis[i].step_pin = axis[i]["step_pin"];
                                axis[i]["step_pin_invert"].isNull()?            Config::axis[i].step_pin_invert = false :       Config::axis[i].step_pin_invert = axis[i]["step_pin_invert"];
                                axis[i]["enable_pin"].isNull()?                 Config::axis[i].enable_pin = -1 :                Config::axis[i].enable_pin = axis[i]["enable_pin"];
                                axis[i]["enable_pin_invert"].isNull()?          Config::axis[i].enable_pin_invert = false :     Config::axis[i].enable_pin_invert = axis[i]["enable_pin_invert"];
                                
                                axis[i]["limit_pin_min"].isNull()?              Config::axis[i].limit_pin_min = -1 :             Config::axis[i].limit_pin_min = axis[i]["limit_pin_min"];
                                axis[i]["limit_pin_min_invert"].isNull()?       Config::axis[i].limit_pin_min_invert = false :  Config::axis[i].limit_pin_min_invert = axis[i]["limit_pin_min_invert"];
                                axis[i]["limit_pin_max"].isNull()?              Config::axis[i].limit_pin_max = -1 :             Config::axis[i].limit_pin_max = axis[i]["limit_pin_max"];
                                axis[i]["limit_pin_max_invert"].isNull()?       Config::axis[i].limit_pin_max_invert = false :  Config::axis[i].limit_pin_max_invert = axis[i]["limit_pin_max_invert"];
                                axis[i]["home_pin"].isNull()?                   Config::axis[i].home_pin = -1 :                  Config::axis[i].home_pin = axis[i]["home_pin"];
                                axis[i]["home_pin_invert"].isNull()?            Config::axis[i].home_pin_invert = false :       Config::axis[i].home_pin_invert = axis[i]["home_pin_invert"];
                            }
                        }
                    }
                }
            }
            else{
                console::logln("Config File not Found",console::error);
            }
            configFile.close();

            console::tabIndex--;
            good_config = true;
            return true;
        }
        bool writeConfigFile(){
            File configFile = CNCFileSystem::fileSystem.open("/public/engineConfig.json", "w");
            if (configFile)
            {
                DynamicJsonDocument doc(2048);
                doc["machine_name"] =   Config::machine_name;
                doc["machine_units"] =  unitIdentifiers[(uint8_t)Config::machine_units];
                JsonArray axis_array = doc.createNestedArray("axis");
                //JsonArray axis_array = doc.to<JsonArray>();
                for(uint8_t i=0;i<Config::axis_count;i++){
                    JsonObject axis = axis_array.createNestedObject();
                    axis["axis_id"] = String(Config::axis[i].id);
                    axis["follow"] = String(Config::axis[i].follow);
                    axis["is_rotary"] = Config::axis[i].is_rotary;
                    axis["is_discrete"] = Config::axis[i].is_discrete;
                    axis["min"] = Config::axis[i].min;
                    axis["max"] = Config::axis[i].max;
                    axis["home"] = Config::axis[i].home;
                    axis["home_negative"] = Config::axis[i].home_negative;
                    axis["hard_limit"] = Config::axis[i].hard_limit;
                    axis["soft_limit"] = Config::axis[i].soft_limit;
                    axis["home_feed_fine"] = Config::axis[i].home_feed_fine;
                    axis["home_feed_coarse"] = Config::axis[i].home_feed_coarse;
                    axis["home_switch_debounce_msec"] = Config::axis[i].home_switch_debounce_msec;
                    axis["home_pulloff_units"] = Config::axis[i].home_pulloff_units;
                    axis["steps_per_unit"] = Config::axis[i].steps_per_unit;
                    axis["max_feed_units_per_sec"] = Config::axis[i].max_feed_units_per_sec;
                    axis["acceleration"] = Config::axis[i].acceleration;
                    axis["dir_pin"] = Config::axis[i].dir_pin;
                    axis["dir_pin_invert"] = Config::axis[i].dir_pin_invert;
                    axis["step_pin"] = Config::axis[i].step_pin;
                    axis["step_pin_invert"] = Config::axis[i].step_pin_invert;
                    axis["enable_pin"] = Config::axis[i].enable_pin;
                    axis["enable_pin_invert"] = Config::axis[i].enable_pin_invert;
                    axis["home_pin"] = Config::axis[i].home_pin;
                    axis["home_pin_invert"] = Config::axis[i].home_pin_invert;
                    axis["limit_pin_min"] = Config::axis[i].limit_pin_min;
                    axis["limit_pin_min_invert"] = Config::axis[i].limit_pin_min_invert;
                    axis["limit_pin_max"] = Config::axis[i].limit_pin_max;
                    axis["limit_pin_max_invert"] = Config::axis[i].limit_pin_max_invert;
                }
                if(serializeJsonPretty(doc,configFile)==0){
                    console::logln("Failed to write engine config file",console::error);
                    
                    configFile.close();
                    return false;
                }
                configFile.close();
                return true;
            }
            configFile.close();
            return false;  
        }
    }
};