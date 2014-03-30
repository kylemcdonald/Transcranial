#pragma once

#define USE_VIDEO

#include "ofMain.h"
#include "ofxSlitScan.h"
#include "ofxUI.h"
#include "ofxCv.h"
#include "Clone.h"
#include "FaceOsc.h"
#include "ofxFaceTrackerThreaded.h"
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
    float offset = 280;
    bool debug = true;
    bool enableFaceSubstitution = false;
    bool enableSlitScan = false;
    bool enableMotionAmplifier = false;
    
    FaceOsc faceOsc;
	ofxFaceTrackerThreaded camTracker;
    
#ifdef USE_VIDEO
    ofVideoPlayer cam;
#else
	ofVideoGrabber cam;
#endif
	
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
