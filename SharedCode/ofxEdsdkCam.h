#pragma once

#include "ofxEdsdk.h"

enum EdsdkDeviceType {
    EDSDK_T2I = 0,
    EDSDK_MKII,
    EDSDK_MKIII
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
        } else if(deviceType == EDSDK_MKIII) {
            forceWidth = 960;
            forceHeight = 640;
        }
    }
    void initGrabber(int width, int height) {
        setup();
    }
    const unsigned char* getData() const {
        return getLivePixels().getData();
    }
    ofPixels& getPixels() {
        return const_cast<ofPixels&>(getLivePixels());
    }
    const ofPixels& getPixels() const {
        return getLivePixels();
    }
    ofTexture& getTexture() {
        return const_cast<ofTexture&>(getLiveTexture());
    }
    const ofTexture& getTexture() const {
        return getLiveTexture();
    }
    void setUseTexture(bool useTexture) {
    }
    bool isUsingTexture() const {
        return true;
    }
    int getWidth() {
        return forceWidth;
    }
    int getHeight() {
        return forceHeight;
    }
};