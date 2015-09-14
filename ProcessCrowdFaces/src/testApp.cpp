#include "testApp.h"

using namespace ofxCv;

void testApp::setup() {
    ofSetDataPathRoot("../../../../../SharedData/");
    dir.allowExt("png");
    dir.allowExt("jpg");
    dir.allowExt("tiff");
    dir.listDir("faces");
    index = 0;
	tracker.setup();
    tracker.setRescale(.5);
    tracker.setIterations(1);
    tracker.setClamp(4);
    tracker.setTolerance(.01);
    tracker.setAttempts(4);
    iterations = 30;
    done = false;
    glPointSize(2);
    ofSetLineWidth(2);
}

void testApp::loadImage() {
    img.load(dir.getPath(index));
    if(!loadFace()) {
        tracker.reset();
        float startTime = ofGetElapsedTimef();
        bool good = true;
        for(int i = 0; i < iterations; i++) {
            tracker.update(toCv(img));
            float curTime = ofGetElapsedTimef();
            if(curTime - startTime > 2) { // max 2 seconds per face
                good = false;
                break;
            }
        }
        if(good) {
            saveFace();
        } else {
            ofFile(dir.getPath(index)).remove();
        }
        timer.tick();
    }
}

void testApp::saveFace() {
    ofMesh mesh = tracker.getImageMesh();
    string baseName = dir.getFile(index).getBaseName();
    string meshName = "meshes/" + baseName + ".ply";
    mesh.save(meshName);
}

bool testApp::loadFace() {
    string baseName = dir.getFile(index).getBaseName();
    string meshName = "meshes/" + baseName + ".ply";
    if(ofFile(meshName).exists()) {
        prevFace.load(meshName);
        return true;
    }
    return false;
}

void testApp::keyPressed(int key) {
    if(key == OF_KEY_LEFT) {
        index = ofClamp(index - 1, 0, dir.size() - 1);
        loadImage();
    }
    if(key == OF_KEY_RIGHT) {
        index = ofClamp(index + 1, 0, dir.size() - 1);
        loadImage();
    }
}

void testApp::update() {
    if(!done && index < dir.size()) {
        loadImage();
        index++;
    } else {
        done = true;
    }
}

void testApp::draw() {
    ofSetColor(255);
    if(img.isAllocated()) {
        img.draw(0, 0);
    }
    
    ofSetColor(ofColor::yellow);
    prevFace.drawVertices();
    ofSetColor(ofColor::yellow, 16);
    prevFace.drawWireframe();
    
    ofSetColor(ofColor::white);
    tracker.getImageMesh().drawVertices();
    ofSetColor(ofColor::white, 16);
    tracker.getImageMesh().drawWireframe();
    

    int n = dir.size();
    float timeEstimate = timer.getPeriod() * (n - index);
    ofDrawBitmapStringHighlight(ofToString(index) + "/" + ofToString(n) + " " + ofToString((int) timeEstimate) + "s", 10, 20);
}