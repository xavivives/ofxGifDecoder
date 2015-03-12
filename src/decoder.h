#pragma once

#include "ofMain.h"
#include "FreeImage.h"

namespace ofxGIF
{
	class fiGifSaver
	{
	public:
		fiGifSaver(){}
		~fiGifSaver(){}

		void create(string filename);
		void save();

		void append(string filename);		// not alpha channel
		void append(ofPixels& pixels);
	};
    
    enum GifFrameDisposal {
        GIF_DISPOSAL_UNSPECIFIED,
        GIF_DISPOSAL_LEAVE,
        GIF_DISPOSAL_BACKGROUND,
        GIF_DISPOSAL_PREVIOUS
    };

	class fiGifLoader
	{
	public:

		fiGifLoader(){}
		~fiGifLoader(){}

		void load(string filename);
        void processFrame(FIBITMAP * bmp, int _frameNum);
        void addFrame(ofPixels _px, int _left , int _top, GifFrameDisposal disposal = GIF_DISPOSAL_PREVIOUS, float _duration = 0);
        vector <ofColor> getPalette();
        ofPixels accumPx;
		vector<ofPixels> pages;
        vector<int> frameDurations; //in miliseconds
        ofColor bgColor;
        

	};
    


};



