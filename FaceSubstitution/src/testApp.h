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
#include "MotionAmplifier.h"
#include "FaceSubstitution.h"

class testApp : public ofBaseApp {
public:
    void setupGui();
	void setup();
    void exit();
	void update();
	void draw();
	void dragEvent(ofDragInfo dragInfo);
    void loadNextPair();
	void loadFace(string face, ofImage& src, vector<ofVec2f>& srcPoints);
	
    void mousePressed(int x, int y, int button);
    void mouseDragged(int x, int y, int button);
	void keyPressed(int key);
    void keyReleased(int key);
    
    ofxUICanvas* gui;
    float offset;
    float motionMax;
    float trackerRescale;
    float substitutionStrength;
    bool debug;
    
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
	ofxFaceTrackerThreaded camTracker;
    FaceSubstitution faceSubstitution;
    ofPixels substitutionDelay;
    
	ofDirectory faces;
	int currentFace;
	ofImage srcOriginal, srcDelay;
	vector<ofVec2f> srcOriginalPoints;
	vector<ofVec2f> srcDelayPoints;
    
    // delay
    ofxSlitScan slitScan;
    
    // blend
    ofShader lighten;
    
    // motion amplification
    MotionAmplifier motionAmplifier;
    ofFbo amplifiedMotionOriginal;
    ofFbo amplifiedMotionDelay;
};
