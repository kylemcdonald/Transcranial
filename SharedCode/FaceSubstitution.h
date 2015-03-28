#pragma once

#include "ofMain.h"
#include "Clone.h"
#include "ofxFaceTracker.h"

class FaceSubstitution {
public:
	ofxFaceTracker tracker;
	ofFbo srcFbo, maskFbo;
	Clone clone;
    
    void setup(int width, int height) {
        clone.setup(width, height);
        
        ofFbo::Settings settings;
        settings.width = width;
        settings.height = height;
        settings.useDepth = false;
        settings.useStencil = false;
        maskFbo.allocate(settings);
        srcFbo.allocate(settings);
        
        tracker.setup();
        tracker.setIterations(30);
        tracker.setAttempts(4);
        
        maskFbo.begin();
        ofClear(0, 255);
        maskFbo.end();
        
        srcFbo.begin();
        ofClear(0, 255);
        srcFbo.end();
    }
    template <class T>
    vector<ofVec2f> getSrcPoints(T& img) {
        tracker.reset();
        tracker.update(ofxCv::toCv(img));
        return tracker.getImagePoints();
    }
    void update(ofxFaceTracker& camTracker, ofBaseHasTexture& cam, vector<ofVec2f>& srcPoints, ofImage& src) {
        ofPushStyle();
        ofDisableDepthTest();
        
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
        
        clone.update(srcFbo.getTexture(), cam.getTexture(), maskFbo.getTexture());
        ofPopStyle();
    }
};