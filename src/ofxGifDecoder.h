#pragma once
#include "ofMain.h"
#include "decoder.h"
#include "gifHandler.h"
#include "ofxGifType.h"

class ofxGifDecodedEvent : public ofEventArgs {
    
public:
    
    string   path;
    ofxGifType *gif;
    
    ofxGifDecodedEvent()
    {
    }
    
    static ofEvent <ofxGifDecodedEvent> DECODED;
};

class ofxGifDecoder : public ofThread{
    
public:
    void setup();
    bool threadedEnabled;
    void decode(string path);
    
private:
    
    void handleExit(ofEventArgs &e);
    void threadedFunction();
    void createGif(ofxGifType *currentGif, string filePath);
    
    gifHandler *gifHandler;
    string gifPath = "";
};

