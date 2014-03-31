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

class FaceSubstitution {
public:
	ofxFaceTracker tracker;
	Clone clone;
	ofFbo srcFbo, maskFbo;
    
    void setup(int width, int height) {
        clone.setup(width, height);
        
        ofFbo::Settings settings;
        settings.width = width;
        settings.height = height;
        maskFbo.allocate(settings);
        srcFbo.allocate(settings);
        
        tracker.setup();
        tracker.setIterations(30);
        tracker.setAttempts(4);
    }
    template <class T>
    vector<ofVec2f> getSrcPoints(T& img) {
        tracker.update(ofxCv::toCv(img));
        return tracker.getImagePoints();
    }
    void update(ofxFaceTracker& camTracker, ofBaseHasTexture& cam, vector<ofVec2f>& srcPoints, ofImage& src) {
        ofMesh camMesh = camTracker.getImageMesh();
        camMesh.clearTexCoords();
        camMesh.addTexCoords(srcPoints);
        
        maskFbo.begin();
        ofClear(0, 255);
        camMesh.draw();
        maskFbo.end();
        
        srcFbo.begin();
        ofClear(0, 255);
        src.bind();
        camMesh.draw();
        src.unbind();
        srcFbo.end();
        
        clone.update(srcFbo.getTextureReference(), cam.getTextureReference(), maskFbo.getTextureReference());
    }
};

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
    float trackerRescale = .5;
    float substitutionStrength = 16;
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
    FaceSubstitution faceSubstitution;
    
	ofDirectory faces;
	int currentFace;
	ofImage src;
	vector<ofVec2f> srcPoints;
    
    // delay
    ofxSlitScan slitScan;
    
    // blend
    ofShader lighten;
    
    // motion amplification
    MotionAmplifier motionAmplifier;
    ofFbo amplifiedMotion;
};
