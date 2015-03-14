#include "ofxGifDecoder.h"

ofEvent<ofxGifDecodedEvent> ofxGifDecodedEvent::DECODED;


void ofxGifDecoder::setup()
{
    ofAddListener(ofEvents().exit, this, &ofxGifDecoder::handleExit);
}

void ofxGifDecoder::decode(string path)
{
    if(isThreadRunning())
    {
        return;
    }
    gifPath = path;
    
    ofLogVerbose("threadedDecoder::Decoding: "+gifPath);
    startThread(true);   // blocking, verbose
}

void ofxGifDecoder::threadedFunction()
{
    ofxGifType *currentGif = new ofxGifType; //the gif is deleted in gifHandler.cpp
    currentGif->filePath = gifPath;
    currentGif->status = "error";
   
    while( isThreadRunning() != 0 )
    {
        lock();
  
        createGif(currentGif, gifPath);

        stopThread();
        unlock();
    }
    ofxGifDecodedEvent event;
    event.path = gifPath;
    event.gif = currentGif;
    ofNotifyEvent(ofxGifDecodedEvent::DECODED, event);
}

void ofxGifDecoder::createGif(ofxGifType *currentGif, string filePath)
{
    ofxGIF::fiGifLoader decoder;
    decoder.load(gifPath);
    int duration = 0;
    
        int length =decoder.pages.size();
        if(length>0)
        {
            currentGif->length = length;
            currentGif->frames.resize(length);
            currentGif->delays.resize(length);

            for(int i=0; i < decoder.pages.size() ; i++)
            {
                currentGif->frames[i] = decoder.pages[i];
                currentGif->delays[i] = decoder.frameDurations[i];
                duration += decoder.frameDurations[i];
            }
            currentGif->duration = duration;
            currentGif->width = currentGif->frames[0].getWidth();
            currentGif->height = currentGif->frames[0].getHeight();
            currentGif->status = "ok";
        }
        else
        {
            currentGif->filePath = gifPath;
            currentGif->status = "error";
            currentGif->length = 0;
            currentGif->frames.resize(0);
            currentGif->delays.resize(0);
            currentGif->duration = duration;
            currentGif->width = 0;
            currentGif->height = 0;
        }
}

void ofxGifDecoder::handleExit(ofEventArgs &e)
{
    ofLogVerbose("threadedDecoder:: exiting");
    if(isThreadRunning())
        waitForThread();
    ofLogVerbose("threadedDecoder:: exited");
}
