#ifndef console_h
#define console_h

#include <Arduino.h>

class console_class{
    public:
    enum msgType{
        routine = 0,
        note = 1,
        caution = 2,
        warning = 3,
        error = 4,
    };
    console_class(){
    }
    Stream* stream = nullptr;     // Parent type for Serial Streams (And all streams)
    char tabIndex = 0;
    msgType logLevel = msgType::routine;
    bool newline = true;
    int lineLength = 0;
    bool timestamp = false;
    void init(Stream *_stream){
        stream = _stream;
        tabIndex = 0;
    }
    void print(String msg){
        if(stream==nullptr)
            return;
        lineLength +=stream->print(msg);
    }
    void write(int i){
        stream->write(i);
    }
    void println(String msg){
        if(stream==nullptr)
            return;   
        stream->println(msg);
        lineLength = 0;
    }
    void addTabs(){
        if(lineLength==0)
        {
            for(int i = 0; i<tabIndex;i++){
                lineLength += stream->print("\t");
            }
        }
    }
    void bar(String msg, char length)
    {
        char halfLength = length/2;
        if(msg.length()>0)
            msg = " " + msg + " ";
        float l = msg.length();
        int h = ceil(l / 2.0);
        addTabs();
        for(int i = 0; i < halfLength - h; i++){
            print("=");
        }
        print(msg);
        for(int i = halfLength -h+l; i < length; i++){
            print("=");
        }
        println("");
        newline = true;
    }
    void bar(char length)
    {
        bar("",length);
    }

    void log(String msg){
        log(msg, routine);
    }
    void logln(String msg){
        logln(msg, routine);
    }    
    void logln(){
        logln("", routine);
    }
    void log(String msg, int pos){
        addTabs();
        for(int i = lineLength;i<pos;i++){
            print(" ");
        }
        log(msg, routine);
    }
    void logln(String msg, int pos){
        addTabs();
        for(int i = lineLength;i<pos;i++){
            print(" ");
        }
        logln(msg, routine);
    }
    void log(String msg, msgType _msgType){
        if(stream==nullptr)
            return;
        // Used to filter out only the most important things based on user preference
        if(_msgType < logLevel)
            return;
        if(timestamp && msg.length()!=0 && lineLength==0)
            lineLength += stream->print(String(millis()/1000.0f,4)+":\t");
        addTabs();
        switch(_msgType){
            case routine:
            
                break;
            case note:
                print("NOTE: ");
                break;
            case caution:
                print("CAUTION: ");
                break;
            case warning:
                print("WARNING: ");
                break;
            case error:
                print("ERROR: ");
                break;
            default:

                break;
        }
        print(msg);
    }
    void logln(String msg, msgType _msgType){
        if(stream==nullptr)
            return;
        log(msg, _msgType);
        println("");
        lineLength = 0;
    }
    void resetOrigin(){
        lineLength = 0;
    }
};
extern console_class console;

#endif