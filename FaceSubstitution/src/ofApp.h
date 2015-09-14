#pragma once

//#define USE_VIDEO
#define USE_EDSDK

#include "ofMain.h"
#include "ofxSlitScan.h"
#include "ofxUI.h"
#include "ofxCv.h"
#include "ofxFaceTrackerThreaded.h"
#include "ofxEdsdkCam.h"
#include "ofxTiming.h"

#include "MotionAmplifier.h"
#include "FaceSubstitution.h"

class ofApp : public ofBaseApp {
public:
    void setupGui();
	void setup();
    void exit();
	void update();
	void draw();
	void dragEvent(ofDragInfo dragInfo);
    void loadNextPair();
	void loadFace(ofFile face, ofImage& src, vector<ofVec2f>& srcPoints);
	
    void mousePressed(int x, int y, int button);
    void mouseDragged(int x, int y, int button);
	void keyPressed(int key);
    
    ofxUICanvas* gui;
    float maxOffset;
    float motionMax;
    float trackerRescale;
    float substitutionStrength;
    bool debug;
    
#ifdef USE_VIDEO
    ofVideoPlayer cam;
#else
    #ifdef USE_EDSDK
        ofxEdsdkCam cam;
    #else
        ofVideoGrabber cam;
    #endif
#endif
    ofImage prevCam;
    
    ofxEdsdk::RateTimer camTimer;
    
    // face tracking, face substitution
	ofxFaceTrackerThreaded camTracker;
    FaceSubstitution faceSubstitution;
    ofPixels substitutionDelay;
    FadeTimer substitutionTimer;
    
	ofDirectory faceMeshes;
	int currentFace;
	ofImage srcOriginal, srcDelay;
	vector<ofVec2f> srcOriginalPoints;
	vector<ofVec2f> srcDelayPoints;
    
    // delay
    ofxSlitScan slitScan;
    DelayTimer delaySync;
    float delaySeconds;
    
    bool offsetDirection;
    FadeTimer offsetTimer;
    
    Hysteresis learningRateHysteresis;
    FadeTimer learningRateTimer;
    Hysteresis normalizedMotionHysteresis;
    FadeTimer normalizedMotionTimer;
    
    // blend
    ofShader lighten;
    
    // motion amplification
    MotionAmplifier motionAmplifier;
    ofFbo amplifiedMotionOriginal;
    ofFbo amplifiedMotionDelay;
};
