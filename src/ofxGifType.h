#pragma once

#include "ofMain.h"

    struct ofxGifType
    {
        //constructor
        ofxGifType()
        {
            status = "Empty";
            fileName = "";
            filePath = "";
           
            width = 0;
            height = 0;
            length = 0;
            delays.resize(0);
            frames.resize(0);
            duration = 0;
        };
        

        //copy constructor
        ofxGifType(const ofxGifType &gif)
        {
            fileName = gif.fileName;
            filePath = gif.filePath;
            status = gif.status;
            width = gif.width;
            height = gif.height;
            length = gif.length;
            delays = gif.delays;
            frames.resize(gif.length);
            duration = gif.duration;
            
            for(int i =0; i < gif.length; i++)
                frames[i] = gif.frames[i];
        };
        
        //assignement operator
        ofxGifType& operator=(const ofxGifType gif)
        {
            if(this == &gif)
                return *this;
            
            fileName = gif.fileName;
            filePath = gif.filePath;
            status = gif.status;
            width = gif.width;
            height = gif.height;
            length = gif.length;
            delays = gif.delays;
            frames.resize(gif.length);
            duration = gif.duration;
            
  
            for(int i =0; i < gif.length; i++)
                frames[i] = gif.frames[i];
        
            
            return *this;   
        }

        string fileName;
        string filePath;
        string status; //Empty/Error/Ok
        int width;
        int height;
        int length; //number of frames
        vector<ofPixels> frames; //all the frames
        vector<int> delays; //delays per frame in ms
        int duration; //total duration in ms
    };

    //destructor

    /*struct RGBA
    {
        unsigned char red;
        unsigned char green;
        unsigned char blue;
        unsigned char alpha;
    };*/


