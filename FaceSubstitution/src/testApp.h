#pragma once

#define USE_VIDEO

#include "ofMain.h"
#include "ofxCv.h"
#include "Clone.h"
#include "FaceOsc.h"
#include "ofxFaceTrackerThreaded.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void dragEvent(ofDragInfo dragInfo);
	void loadFace(string face);
	
	void keyPressed(int key);
    
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
};
