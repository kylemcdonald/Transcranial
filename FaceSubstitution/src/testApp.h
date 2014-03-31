#pragma once

#define USE_VIDEO
//#define USE_EDSDK

#include "ofMain.h"
#include "ofxSlitScan.h"
#include "ofxUI.h"
#include "ofxCv.h"
#include "ofxFaceTrackerThreaded.h"
#include "ofxEdsdkCam.h"
#include "RateTimer.h"

#include "FaceOsc.h"
#include "ofxOscSender.h"
#include "Clone.h"
#include "MotionAmplifier.h"

class testApp : public ofBaseApp {
public:
    void setupGui();
	void setup();
    void exit();
	void update();
	void draw();
	void dragEvent(ofDragInfo dragInfo);
	void loadFace(string face);
	
	void keyPressed(int key);
    
    ofxUICanvas* gui;
    float offset = 85;
    float motionMax = 50;
    bool debug = true;
    
    ofxOscReceiver oscInput;
    
#ifdef USE_VIDEO
    ofVideoPlayer cam;
#else
    #ifdef USE_EDSDK
        ofxEdsdkCam cam;
    #else
        ofVideoGrabber cam;
    #endif
#endif
    
    ofxEdsdk::RateTimer camTimer;
    
    // face tracking, face substitution
    FaceOsc faceOsc;
	ofxFaceTracker camTracker;
	ofxFaceTracker srcTracker;
	ofImage src;
	vector<ofVec2f> srcPoints;
	Clone clone;
	ofFbo srcFbo, maskFbo;
	ofDirectory faces;
	int currentFace;
    
    // delay
    ofxSlitScan slitScan;
    
    // blend
    ofShader lighten;
    
    // motion amplification
    MotionAmplifier motionAmplifier;
    ofFbo amplifiedMotion;
};
