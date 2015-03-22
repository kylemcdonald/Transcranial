#pragma once

#include "ofxEdsdk.h"

enum EdsdkDeviceType {
    EDSDK_T2I = 0,
    EDSDK_MKII
};

class ofxEdsdkCam : public ofxEdsdk::Camera, public ofBaseHasPixels, public ofBaseHasTexture {
protected:
    int forceWidth, forceHeight;
public:
    void setDeviceID(int deviceId) {
    }
    void setDeviceType(EdsdkDeviceType deviceType) {
        if(deviceType == EDSDK_T2I) {
            forceWidth = 1056;
            forceHeight = 704;
        } else if(deviceType == EDSDK_MKII) {
            forceWidth = 1024;
            forceHeight = 680;
        }
    }
    void initGrabber(int width, int height) {
        setup();
    }
    unsigned char* getData() {
        return getLivePixels().getData();
    }
    ofPixels& getPixels() {
        return getLivePixels();
    }
    ofTexture& getTextureReference() {
        return getLiveTexture();
    }
    void setUseTexture(bool useTexture) {
    }
    int getWidth() {
        return forceWidth;
    }
    int getHeight() {
        return forceHeight;
    }
};