#ifndef console_h
#define console_h

#include <Arduino.h>

namespace console{
    extern Stream* stream;
    extern char tabIndex;
    extern bool timestamp;
    void init(Stream *_stream);
    enum msgType{
        routine = 0,
        note = 1,
        caution = 2,
        warning = 3,
        error = 4,
    };
    extern msgType logLevel;
    void write(int i);
    void bar(String msg, char length);    
    void bar(char length);  
    void log(String msg);
    void logln(String msg);
    void logln();
    void log(String msg, int pos);
    void logln(String msg, int pos);
    void log(String msg, int pos);
    void logln(String msg, int pos);
    void logln(String msg, msgType _msgType);
    void log(String msg, msgType _msgType);
    void print(String msg);
    void println(String msg);
    void resetOrigin();
    
};

#endif