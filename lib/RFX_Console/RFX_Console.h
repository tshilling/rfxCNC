#ifndef console_h
#define console_h

#include <Arduino.h>

class console_class{
    private:
    	void (*_writeCallback)(String);
    public:
    void setCallback(void (*writeCallback)(String)){
        _writeCallback = writeCallback;
    }
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
    String active_msg = "";
    void init(Stream *_stream){
        stream = _stream;
        tabIndex = 0;
    }
    void print(){
        if(stream==nullptr)
            return;
        if(timestamp && lineLength==0)
            active_msg = String(millis()/1000.0f,4)+" "+active_msg;

        lineLength +=stream->print(active_msg);
        if(_writeCallback)
            _writeCallback(active_msg);
        active_msg = "";
    }
    void write(int i){
        stream->write(i);
    }
    void println(){
        if(stream==nullptr)
            return;   
        if(timestamp && lineLength==0)
            active_msg = String(millis()/1000.0f,4)+" "+active_msg;
        stream->println(active_msg);
        if(_writeCallback)
            _writeCallback(active_msg+"\n");
        active_msg = "";
        lineLength = 0;
    }
    void addTabs(){
        if(lineLength==0)
        {
            for(int i = 0; i<tabIndex;i++){
                active_msg += '\t';
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
            active_msg +="=";
        }
        active_msg +=msg;
        for(int i = halfLength -h+l; i < length; i++){
            active_msg +="=";
        }
        println();
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
        for(int i = active_msg.length();i<pos;i++){
            active_msg +=" ";
        }
        log(msg, routine);
    }
    void logln(String msg, int pos){
        addTabs();
        for(int i = active_msg.length();i<pos;i++){
            active_msg +=" ";
        }
        logln(msg, routine);
    }
    void log(String msg, msgType _msgType){
        if(stream==nullptr)
            return;
        // Used to filter out only the most important things based on user preference
        if(_msgType < logLevel)
            return;
        addTabs();
        switch(_msgType){
            case routine:

                break;
            case note:
                active_msg+="NOTE: ";
                break;
            case caution:
                active_msg+="CAUTION: ";
                break;
            case warning:
                active_msg+="WARNING: ";
                break;
            case error:
                active_msg+="ERROR: ";
                break;
            default:

                break;
        }
        active_msg += msg;
        print();
    }
    void logln(String msg, msgType _msgType){
        if(stream==nullptr)
            return;
        log(msg, _msgType);
        println();
    }
    void resetOrigin(){
        lineLength = 0;
    }
};
extern console_class console;

#endif