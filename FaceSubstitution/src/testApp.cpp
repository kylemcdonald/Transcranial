#include "testApp.h"

using namespace ofxCv;

void testApp::setupGui() {
    gui = new ofxUICanvas();
    gui->addFPS();
    gui->addToggle("Debug", &debug);
    gui->addSlider("Tracker rescale", .1, 1, &trackerRescale);
    gui->addSlider("Substitution strength", 1, 64, &substitutionStrength);
    gui->addSlider("Offset", 0, 600, &offset);
    gui->addSlider("Motion max", 0, 100, &motionMax);
    gui->addSlider("Motion strength", -100, 100, &motionAmplifier.strength);
    gui->addSlider("Motion learning rate", 0, 1, &motionAmplifier.learningRate);
    gui->addSlider("Motion blur amount", 0, 15, &motionAmplifier.blurAmount);
    gui->addSlider("Motion window size", 1, 64, &motionAmplifier.windowSize);
    gui->autoSizeToFitWidgets();
}

void testApp::setup() {
    ofSetDataPathRoot("../../../../../SharedData/");
	ofSetVerticalSync(true);
    
#ifdef USE_VIDEO
    cam.loadMovie("videos/milos-extreme.mov");
    cam.play();
#else
    cam.setDeviceID(0);
    cam.initGrabber(1280, 720);
    #ifdef USE_EDSDK
        cam.setDeviceType(EDSDK_MKII);
    #endif
#endif
    camTimer.setSmoothing(.5);
    
	camTracker.setup();
    camTracker.setRescale(trackerRescale);
    camTracker.setHaarMinSize(cam.getHeight() / 4);
    
    faceSubstitution.setup(cam.getWidth(), cam.getHeight());
    
	faces.allowExt("jpg");
	faces.listDir("faces");
	currentFace = 0;
	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}
    
    faceOsc.osc.setup("192.168.0.255", 8338);
    oscInput.setup(7401);
    
    ofImage distortionMap;
    distortionMap.loadImage("images/white.png");
    slitScan.setup(cam.getWidth(), cam.getHeight(), 100);
    slitScan.setDelayMap(distortionMap);
    slitScan.setBlending(false);
    slitScan.setTimeDelayAndWidth(60, 0);
    
    lighten.load("shaders/Lighten");
    
    motionAmplifier.setup(cam.getWidth(), cam.getHeight(), 1, .25);
    amplifiedMotion.allocate(cam.getWidth(), cam.getHeight());
    
    setupGui();
}

void testApp::exit() {
//    camTracker.stopThread();
}

void testApp::update() {
    while(oscInput.hasWaitingMessages()) {
        ofxOscMessage msg;
        oscInput.getNextMessage(&msg);
        if(msg.getAddress() == "/delay") {
            float delaySeconds = msg.getArgAsFloat(0) / 1000.;
            int delayFrames = delaySeconds * camTimer.getFrameRate();
            delayFrames = MIN(delayFrames, slitScan.getCapacity());
            ofLog() << delaySeconds << " " << delayFrames;
            slitScan.setTimeDelayAndWidth(delayFrames, 0);
        }
    }
    
    float normalizedMotion = ofGetKeyPressed(' ') ? 1 : 0;
    motionAmplifier.strength = normalizedMotion * motionMax;
    
    camTracker.setRescale(trackerRescale);
    faceSubstitution.clone.setStrength(substitutionStrength);
    
	cam.update();
	if(cam.isFrameNew()) {
        camTimer.tick();
        
        // step 1: face tracking and optical flow
        
        // face tracking
        camTracker.update(toCv(cam));
        faceOsc.sendFaceOsc(camTracker);
        
        // optical flow
        motionAmplifier.update(slitScan.getOutputImage());
        
        // step 2: face sub with two different images
        if(camTracker.getFound()) {
            faceSubstitution.update(camTracker, cam, srcPoints, src);
        }
        
        amplifiedMotion.begin();
        motionAmplifier.draw(slitScan.getOutputImage());
        amplifiedMotion.end();
        
        slitScan.addImage(cam);
	}
}

void testApp::draw() {
    ofBackground(0);
	ofSetColor(255);
    
    float scale = ofGetWidth() / (float) cam.getWidth();
    ofPushMatrix();
    ofScale(scale, scale);
	
    if(offset > 0) {
        float w = cam.getWidth();
        float h = cam.getHeight();
        lighten.begin();
        lighten.setUniform2f("resolution", w, h);
        lighten.setUniformTexture("a", faceSubstitution.clone.getTexture(), 1);
        lighten.setUniformTexture("b", amplifiedMotion, 2);
        lighten.setUniform2f("offset", offset, 0);
        cam.draw(0, 0);
        lighten.end();
    } else {
        amplifiedMotion.draw(0, 0);
    }
    
    if(debug) {
        camTracker.draw();
        ofScale(.2, .2);
        faceSubstitution.maskFbo.draw(0, 0);
        ofTranslate(0, cam.getHeight());
        faceSubstitution.srcFbo.draw(0, 0);
        ofTranslate(0, cam.getHeight());
        ofScale(1./motionAmplifier.getRescale(), 1./motionAmplifier.getRescale());
        motionAmplifier.getFlowTexture().draw(0, 0);
    }
    
    ofPopMatrix();
}

void testApp::loadFace(string face){
	src.loadImage(face);
	if(src.getWidth() > 0) {
        srcPoints = faceSubstitution.getSrcPoints(src);
	}
}

void testApp::dragEvent(ofDragInfo dragInfo) {
	loadFace(dragInfo.files[0]);
}

void testApp::keyPressed(int key){
    if(key == 'f') {
        ofToggleFullscreen();
    }
    if(key == '\t') {
        gui->toggleVisible();
    }
	switch(key){
        case OF_KEY_UP:
            currentFace++;
            break;
        case OF_KEY_DOWN:
            currentFace--;
            break;
	}
    if(key == OF_KEY_UP || key == OF_KEY_DOWN) {
        currentFace = ofClamp(currentFace,0,faces.size()-1);
        if(faces.size()!=0){
            loadFace(faces.getPath(currentFace));
        }
    }
}
