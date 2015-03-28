#pragma once

#include "ofMain.h"
#include "ofxCv.h"

#include "ofxFaceTracker.h"
#include "ofxTiming.h"

class testApp : public ofBaseApp {
public:
    void setup();
    void update();
	void draw();
    void loadImage();
    void saveFace();
    bool loadFace();
    void keyPressed(int key);
    
    ofDirectory dir;
    bool done;
    int index;
	ofxFaceTracker tracker;
    int iterations;
    ofImage img;
    ofMesh prevFace;
    RateTimer timer;
};
