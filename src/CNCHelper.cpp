#include "CNCHelpers.h"
namespace RFX_CNC{
    // "m","cm","mm","ft","in", [from][to]
    const float unit_convert[5][5] = {
        {1.000f, 100.0f, 1000.0f, 3.281f, 39.37f},
        {0.010f, 1.000f, 10.000f, 0.033f, 0.394f},
        {0.001f, 0.305f, 1.0000f, 0.003f, 0.039f},
        {0.305f, 30.48f, 304.80f, 1.000f, 12.00f},
        {0.025f, 2.540f, 25.400f, 0.083f, 1.000f}
    };
    const float SMALLEST_FLOAT = nextafterf(0.0f,1.0f);

    float powf2(float input){
        return input*input;
    }
    float get_length(float vector[], uint8_t count){
        float move_length = 0;
        for(uint_fast8_t i = 0; i < count;i++){
                move_length += (float)vector[i]*(float)vector[i];
        }
        move_length = sqrt(move_length);
        return move_length;
    }
    /* (https://makezine.com/2008/12/04/quakes-fast-inverse-square-roo/)
    Quick way to approximate a inverse sqrt:
    1) Divide number by 2, its floating point so bitshift wont work = xhalf
    2) Take a reasonable guess at a 1/sqrt value = g
    3) refine it (As many times as desired) using: g = g*(1.5 – xhalf * g *g)

    The trick is getting that initial guess. The game engine coders use trick with how floating point numbers are represented in binary, 
    with the exponent and mantissa broken up similar to scientific notation. In a 32-bit float, the left-most bit is a sign bit and is 0 
    for positive numbers. That’s followed by 8 bits of exponent (biased by 127 to represent negative and positive exponents), and the final 
    23 bits represent the mantissa. Well, to do an inverse square root, you basically need to multiply the exponent by -1/2. When you shift 
    those bits to the right (a very fast operation), the effect on the exponent is to divide it by 2. You still need to subtract the exponent 
    from 0 to change it’s sign, though, and what do you do about the mantissa which was also affected in the bit shift operation?

    This is where the magic 0x5f3759df number comes in. It’s absolutely bonkers, but by subtracting the bit shift result from 0x5f3759df, 
    the mantissa is reset to near to it’s original state and the exponent is subtracted from 0 (taking into account it’s bias of 127). The result 
    is very close to the inverse square root. Close enough for a single pass through Newton’s equation to end up with a value precise enough for 
    practical purposes.
    */
    float Q_rsqrt( float number, uint8_t refinement)
    {	
        // A union is very special, it allows the foat and long to exist in the same bytes.  It is the same 1s and 0s and we can access them as either. This is the bit copy needed.
        union {
            float f;
            unsigned long i;
        } conv  = { .f = number };
        conv.i  = 0x5f3759df - ( conv.i >> 1 ); // Make initial guess for sqrt.  Magic numbers get you close
        for(uint8_t i = 0; i <= refinement;i++){
            float half_of_number = number * 0.5F;
            conv.f  *= (1.5f- ( half_of_number * conv.f * conv.f ) );   // Perform one Newton step to refine
        }
        return conv.f;
    }
    // Unit_vector takes in a "count" dimensional vector and computes the unit vector.  The result is stored
    // in "result_out".  Result_out must be pre allocated and the correct size.  No checking occurs.
    // unit_vector returns true if successful (length>0) and false if unsuccessful (length==0)
    // unit_vector_return_length differs in that it returns the length of the input vector. This takes advantage 
    // of the fact that we often need both values and it inverse is already calculated. 
    bool unit_vector(float vector_in[], float result_out[], uint8_t count, uint8_t refinement){
        float length_squared = 0;
        for(uint8_t i = 0; i < count;i++){
            length_squared += vector_in[i]*vector_in[i];
        }
        if(length_squared == 0){
            for(uint8_t i = 0; i < count;i++){
                result_out[i] = 0;
            }
            return false;
        }
        float inverse_sqrt_of_length_squared = Q_rsqrt(length_squared,refinement);
        for(uint8_t i = 0; i < count;i++){
            result_out[i] = vector_in[i]*inverse_sqrt_of_length_squared;
        }
        return true;
    }
    float unit_vector_return_length(float vector_in[], float result_out[], uint8_t count, uint8_t refinement){
        float length_squared = 0;
        for(uint8_t i = 0; i < count;i++){
            length_squared += vector_in[i]*vector_in[i];
        }
        if(length_squared == 0){
            for(uint8_t i = 0; i < count;i++){
                result_out[i] = 0;
            }
            return 0;
        }
        float inverse_sqrt_of_length_squared = Q_rsqrt(length_squared,refinement);
        for(uint8_t i = 0; i < count;i++){
            result_out[i] = vector_in[i]*inverse_sqrt_of_length_squared;
        }
        return 1.0f / inverse_sqrt_of_length_squared;
    }
    float unit_vector_return_length(int32_t vector_in[], float result_out[], uint8_t count, uint8_t refinement){
        float length_squared = 0;
        for(uint8_t i = 0; i < count;i++){
            length_squared += vector_in[i]*vector_in[i];
        }
        if(length_squared == 0){
            for(uint8_t i = 0; i < count;i++){
                result_out[i] = 0;
            }
            return 0;
        }
        float inverse_sqrt_of_length_squared = Q_rsqrt(length_squared,refinement);
        for(uint8_t i = 0; i < count;i++){
            result_out[i] = vector_in[i]*inverse_sqrt_of_length_squared;
        }
        return 1.0f / inverse_sqrt_of_length_squared;
    }
    /*
    Takes in a float value and ensures the returned value is within the defined range. 
    */
    float IRAM_ATTR clipValue(float input, float min, float max){
        if(input<min)
            return min;
        if(input>max)
            return max;
        return input;
    }    
    uint8_t index_of_max(float input[], uint8_t count){
        uint8_t index = 0;
        for(uint8_t i = 0; i < count;i++){
            if(input[i] > input[index])
                index = i;
        }
        return index;
    }
    uint8_t index_of_max(int32_t input[], uint8_t count){
        uint8_t index = 0;
        for(uint8_t i = 0; i < count;i++){
            if(input[i] > input[index])
                index = i;
        }
        return index;
    }
    uint8_t index_of_max(uint32_t input[], uint8_t count){
        uint8_t index = 0;
        for(uint8_t i = 0; i < count;i++){
            if(input[i] > input[index])
                index = i;
        }
        return index;
    }
}