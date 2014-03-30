#pragma once

//#define USE_VIDEO

#include "ofMain.h"
#include "ofxSlitScan.h"
#include "ofxUI.h"
#include "ofxCv.h"
#include "RateTimer.h"
#include "ofxFaceTrackerThreaded.h"

#include "FaceOsc.h"
#include "Clone.h"
#include "MotionAmplifier.h"
#include "FrameDifference.h"

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
    float offset = 280;
    bool debug = true;
    bool enableFaceSubstitution = false;
    bool enableSlitScan = false;
    bool enableMotionAmplifier = false;
    bool enableBinaryPatterns = false;
    float motionThreshold = 125;
    
    ofxOscReceiver osc;
    FaceOsc faceOsc;
	ofxFaceTrackerThreaded camTracker;
    
#ifdef USE_VIDEO
    ofVideoPlayer cam;
#else
	ofVideoGrabber cam;
#endif
    
    RateTimer camTimer;
    
    FrameDifference motion;
	
	ofxFaceTracker srcTracker;
	ofImage src;
	vector<ofVec2f> srcPoints;
	
	bool cloneReady;
	Clone clone;
	ofFbo srcFbo, maskFbo;

	ofDirectory faces;
	int currentFace;
    
    ofShader binaryEffects;
    ofFbo binary;
    ofShader displacement;
    
    ofxSlitScan slitScan;
    ofVideoGrabber grabber;
    
    ofShader lighten;
    
    MotionAmplifier motionAmplifier;
    ofFbo amplifiedMotion;
};
