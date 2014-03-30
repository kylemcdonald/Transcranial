#pragma once

#include "ofxEdsdk.h"

class ofxEdsdkCam : public ofxEdsdk::Camera, public ofBaseHasPixels {
public:
    unsigned char* getPixels() {
        return getLivePixels().getPixels();
    }
    ofPixels& getPixelsRef() {
        return getLivePixels();
    }
    ofTexture& getTextureReference() {
        return getLiveTexture();
    }
};