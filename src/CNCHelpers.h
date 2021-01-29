#pragma once
#ifndef CNCHelpers_h
#define CNCHelpers_h

#include "Arduino.h"

#define MIN_FLOAT_VALUE     -3.4028235E+38
#define MAX_FLOAT_VALUE      3.4028235E+38
#define MIN_INT16_VALUE     -32768
#define MAX_INT16_VALUE      32767
#define MIN_UINT16_VALUE     0
#define MAX_UINT16_VALUE     65535
#define MIN_INT32_VALUE     -2147483648
#define MAX_INT32_VALUE      2147483647
#define MIN_UINT32_VALUE     0
#define MAX_UINT32_VALUE     4294967295
#define MIN_INT64_VALUE     -9223372036854780000
#define MAX_INT64_VALUE      9223372036854780000
#define MIN_UINT64_VALUE     0
#define MAX_UINT64_VALUE     18446744073709600000
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define _A_ 0
#define _B_ 1
#define _C_ 2
#define _D_ 3
#define _E_ 4
#define _F_ 5
#define _G_ 6
#define _H_ 7
#define _I_ 8
#define _J_ 9
#define _K_ 10
#define _L_ 11
#define _M_ 12
#define _N_ 13
#define _O_ 14
#define _P_ 15
#define _Q_ 16
#define _R_ 17
#define _S_ 18
#define _T_ 19
#define _U_ 20
#define _V_ 21
#define _W_ 22
#define _X_ 23
#define _Y_ 24
#define _Z_ 25

namespace RFX_CNC{
    float get_length(float vector[], uint8_t count);
    float Q_rsqrt( float number, uint8_t refinement);
    bool unit_vector(float vector_in[], float result_out[], uint8_t count, uint8_t refinement);
    float unit_vector_return_length(float vector_in[], float result_out[], uint8_t count, uint8_t refinement);
    float unit_vector_return_length(int32_t vector_in[], float result_out[], uint8_t count, uint8_t refinement);
    float IRAM_ATTR  clipValue(float input, float min, float max);
    uint8_t index_of_max(float input[], uint8_t count);
    uint8_t index_of_max(int32_t input[], uint8_t count);
    uint8_t index_of_max(uint32_t input[], uint8_t count);
    float powf2(float input);
    // The smallest float value possible without being zero.
    // Particularlly useful for values where divide by zero could happen
    extern const float SMALLEST_FLOAT;
//    struct keyValuePair{
//        String key;
//        float value;
//    };
    enum direction_enum{
        backward = 0,
        forward = 1
    };
    extern const float unit_convert[5][5];

    const String unitIdentifiers[]  = {"m","cm","mm","ft","in"};
    enum units_enum{
        units_m = 0,
        units_cm = 1,
        units_mm = 2,
        units_ft = 3,
        units_in = 4
    };
}

#endif