#pragma once
#include "Arduino.h"
#include "CNCEngineConfig.h"
namespace RFX_CNC{
class bresenham_line_class{
        public:

        std::vector<int32_t> delta_steps;
        std::vector<uint32_t> double_delta;
        std::vector<int32_t>  D;
        std::vector<int16_t>  smoothing_count;
        std::vector<int8_t>   direction;

        uint8_t  index_of_dominate_axis = 0;
        uint8_t  index_of_max_axis = 0;
        uint8_t  smoothing = 0;
        bool is_complete = false;
        bool is_infinate = false;
        bresenham_line_class(){
            delta_steps.resize(config.axis.size());
            double_delta.resize(config.axis.size());
            D.resize(config.axis.size());
            smoothing_count.resize(config.axis.size());
            direction.resize(config.axis.size());
        }
        ~bresenham_line_class(){
            
        }
        void init(int32_t steps[]){
            is_complete = true;
            for(uint8_t i = 0; i < config.axis.size();i++){
                delta_steps[i] = steps[i];
                if(delta_steps[i] < 0)
                    direction[i] = -1;
                else if(delta_steps[i] > 0)
                    direction[i] = 1;
                else
                    direction[i] = 0;
                delta_steps[i] = abs(delta_steps[i]);
            }
            index_of_dominate_axis = 0;
            for(uint8_t i = 0; i < config.axis.size();i++){
                if(config.axis[i].is_discrete)
                    if(delta_steps[i] > delta_steps[index_of_dominate_axis])
                        index_of_dominate_axis = i;
                if(delta_steps[i] > delta_steps[index_of_max_axis])
                    index_of_max_axis = i;
            }
            smoothing = 0;
            for(uint_fast8_t i = 0; i < config.axis.size();i++){
                double_delta[i]     = delta_steps[i]<<1;                          // Always positive, only ever 2x base value, uint32 works with int32 step value
                D[i]                = double_delta[i] - delta_steps[index_of_dominate_axis];
                smoothing_count[i]  = 0;
            } 
        }
        void smooth(uint8_t _smoothing){
            smoothing =_smoothing;
            uint32_t dominate_delta = delta_steps[index_of_dominate_axis]<<smoothing;
            // Initialize bresenham line algorithum values.
            for(uint_fast8_t i = 0; i < config.axis.size();i++){
                delta_steps[i]      = delta_steps[i]<<smoothing;
                double_delta[i]     = delta_steps[i]<<1;                          // Always positive, only ever 2x base value, uint32 works with int32 step value
                D[i]                = double_delta[i] - dominate_delta;
                smoothing_count[i]  = 0;
            } 
            is_complete = (delta_steps[index_of_dominate_axis]<=0);
        }
    };
}