#include "decoder.h"

#include "FreeImage.h"

namespace ofxGIF
{
	/// Pointer to a multi-page file stream
	FIMULTIBITMAP *gif = NULL;

	//----------------------------------------------------------
	// static variable for freeImage initialization:
	void ofInitFreeImage(bool deinit = false){
		// need a new bool to avoid c++ "deinitialization order fiasco":
		// http://www.parashift.com/c++-faq-lite/ctors.html#faq-10.15
		static bool	* bFreeImageInited = new bool(false);
		if (!*bFreeImageInited && !deinit){
			FreeImage_Initialise();
			*bFreeImageInited = true;
		}
		if (*bFreeImageInited && deinit){
			FreeImage_DeInitialise();
		}
	}

	void fiGifLoader::load(string filename)
	{
		ofxGIF::ofInitFreeImage();
		filename = ofToDataPath(filename);
		FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(filename.c_str());
		gif = FreeImage_OpenMultiBitmap(fif, filename.c_str(), FALSE, TRUE, TRUE, GIF_LOAD256 );
        
            
		// Get src page count   
		int count = FreeImage_GetPageCount(gif);
        //cout << "\nNum frames: "<<count;
		// Clone src to dst
		for (int page = 0; page < count; page++) {
			// Load the bitmap at position 'page'   
			FIBITMAP *dib = FreeImage_LockPage(gif, page);
            
			if (dib) {
                //get frames durations
                FITAG *tag = NULL;
                if( FreeImage_GetMetadata(FIMD_ANIMATION, dib, "FrameTime", &tag)) {
                     frameDurations.push_back((int)*(long *)FreeImage_GetTagValue(tag));
                }
                
                if(page == 0)
                {
                    RGBQUAD _bgColor;
                    if(FreeImage_GetBackgroundColor(dib, &_bgColor)){
                        bgColor = ofColor(_bgColor.rgbRed, _bgColor.rgbGreen, _bgColor.rgbBlue);
                    }
                }
                
                processFrame(dib, page);
                
            }
            FreeImage_UnlockPage(gif, dib, false);
            
		}

		// Close src   
		FreeImage_CloseMultiBitmap(gif, 0);

		// wired, sometimes last page get empty, check all
		for (int i = 0; i < pages.size(); i++)
		{
			if (pages[i].getWidth() == 0) pages.erase(pages.begin() + i--); 
		}
        //cout << "\nFinito";
	}
    
    void fiGifLoader::processFrame(FIBITMAP * bmp, int _frameNum)
    {
        
        FITAG *tag;
        ofPixels pix;
        
        unsigned int   frameLeft= 0;
        unsigned int frameTop = 0 ;
        float frameDuration;
        GifFrameDisposal disposal_method = GIF_DISPOSAL_BACKGROUND;
        
        if( FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "FrameLeft", &tag)) {
            frameLeft = *(unsigned short *)FreeImage_GetTagValue(tag);
        }
        
        if( FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "FrameTop", &tag)) {
            frameTop = *(unsigned short *)FreeImage_GetTagValue(tag);
        }
        
        if( FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "FrameTime", &tag)) {
            long frameTime = *(long *)FreeImage_GetTagValue(tag);// centiseconds 1/100 sec
            frameDuration =(float)frameTime/1000.f;
        }
        
        if( FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "DisposalMethod", &tag)) {
            disposal_method = (GifFrameDisposal) *(unsigned char *)FreeImage_GetTagValue(tag);
        }
        
        // we do this for drawing. eventually we should be able to draw 8 bits? at least to retain the data
        //    if(FreeImage_GetBPP(bmp) == 8) {
        //        // maybe we should only do this when asked for rendering?
        //        bmp = FreeImage_ConvertTo24Bits(bmp);
        //    }
        
        FIBITMAP* bmpConverted = NULL;
        if(FreeImage_GetColorType(bmp) == FIC_PALETTE || FreeImage_GetBPP(bmp) < 8) {
            if(FreeImage_IsTransparent(bmp)) {
                bmpConverted = FreeImage_ConvertTo32Bits(bmp);
            } else {
                bmpConverted = FreeImage_ConvertTo24Bits(bmp);
            }
            bmp = bmpConverted;
        }
        
        unsigned int width      = FreeImage_GetWidth(bmp);
        unsigned int height     = FreeImage_GetHeight(bmp);
        unsigned int bpp        = FreeImage_GetBPP(bmp);
        // changed this bc we're not using PixelType template anywhere else...
        unsigned int channels   = (bpp / sizeof( unsigned char )) / 8;
        unsigned int pitch      = FreeImage_GetPitch(bmp);
        
        // ofPixels are top left, FIBITMAP is bottom left
        FreeImage_FlipVertical(bmp);
        
        unsigned char * bmpBits = FreeImage_GetBits(bmp);
        
        if(bmpBits != NULL) {
            pix.setFromAlignedPixels(bmpBits, width, height, channels, pitch);
            
            #ifdef TARGET_LITTLE_ENDIAN
            pix.swapRgb();
            #endif
            
             //gifFile.addFrame(pix, frameLeft, frameTop, disposal_method, frameDuration);
            addFrame(pix, frameLeft, frameTop, disposal_method, frameDuration);
            
            
        } else {
            ofLogError() << "ofImage::putBmpIntoPixels() unable to set ofPixels from FIBITMAP";
        }
        if(bmpConverted!= NULL)
            FreeImage_Unload(bmpConverted);
    }
    
    void fiGifLoader::addFrame(ofPixels _px, int _left, int _top, GifFrameDisposal disposal, float _duration){

        int previousFrame = pages.size()-1;
        if (previousFrame < 0)
            previousFrame = 0;
        
        if(pages.size() == 0){
           //// cout << "\n" << bgColor;
            accumPx.allocate(_px.getWidth(),_px.getHeight(),OF_PIXELS_RGB);
            accumPx.setColor(bgColor);
            accumPx = _px; // we assume 1st frame is fully drawn
        }
        else {
            // add new pixels to accumPx
            int cropOriginX = _left;
            int cropOriginY = _top;
            
            // [todo] make this loop only travel through _px, not accumPx
            for (int i = 0; i < accumPx.getWidth() * accumPx.getHeight(); i++) {
                int x = i % accumPx.getWidth();
                int y = i / accumPx.getWidth();
                
                if (x >= _left  && x < _left + _px.getWidth()  &&
                    y >= _top   && y < _top  + _px.getHeight()){
                    int cropX = x - cropOriginX;  //   (i - _left) % _px.getWidth();
                    int cropY = y - cropOriginY;

                    if ( _px.getColor(cropX, cropY).a == 0 ){
                        switch ( disposal ) {
                            case GIF_DISPOSAL_BACKGROUND:
                                accumPx.setColor(x,y,bgColor);
                                break;
                                
                            case GIF_DISPOSAL_PREVIOUS:
                                cout << "\n" << "PREVIOUS";
                                accumPx.setColor(x,y,pages[previousFrame].getColor(cropX, cropY));
                                break;
                                
                            case GIF_DISPOSAL_LEAVE:
                                break;
                            
                            case GIF_DISPOSAL_UNSPECIFIED:
                                break;
                                
                        }
                    }
                    else
                    {
                        accumPx.setColor(x, y, _px.getColor(cropX, cropY) );
                    }
                }
                //Do nothing
            }
        }

        pages.push_back(accumPx);
        frameDurations.push_back(_duration  );
    }

};



