 #include "operations.h"
 namespace CNC_ENGINE{
 const char* operation_result_string[]={
        "success",
        "zero length",
        "zero velocity",
        "max distance exceeded",
        "max velocity exceeded",
        "invalid operation",
        "queue full",
        "unrecognized_command"
    };
 }