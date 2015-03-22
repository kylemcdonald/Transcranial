#include "testApp.h"

using namespace ofxCv;

void testApp::setupGui() {
    gui = new ofxUICanvas();
    gui->addFPS();
    gui->addToggle("Debug", &(debug=false));
    gui->addSlider("Offset", 0, 600, &(offset=0));
    gui->addSlider("Tracker rescale", .1, 1, &(trackerRescale=.5));
    gui->addSlider("Substitution strength", 0, 64, &(substitutionStrength=0));
    gui->addSlider("Motion max", 0, 100, &(motionMax=12));
    gui->addSlider("Motion strength", -100, 100, &motionAmplifier.strength);
    gui->addSlider("Motion learning rate", 0, 1, &motionAmplifier.learningRate);
    gui->addSlider("Motion blur amount", 0, 15, &motionAmplifier.blurAmount);
    gui->addSlider("Motion window size", 1, 64, &motionAmplifier.windowSize);
    gui->autoSizeToFitWidgets();
    keyPressed('\t');
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
    loadNextPair();
	
    faceOsc.osc.setup("localhost", 8338);
    oscInput.setup(7401);
    
    ofImage distortionMap;
    distortionMap.loadImage("images/white.png");
    slitScan.setup(cam.getWidth(), cam.getHeight(), 100);
    slitScan.setDelayMap(distortionMap);
    slitScan.setBlending(false);
    slitScan.setTimeDelayAndWidth(60, 0);
    
    lighten.load("shaders/Lighten");
    
    motionAmplifier.setup(cam.getWidth(), cam.getHeight(), 1, .25);
    amplifiedMotionOriginal.allocate(cam.getWidth(), cam.getHeight());
    amplifiedMotionDelay.allocate(cam.getWidth(), cam.getHeight());
    
    setupGui();
}

void testApp::exit() {
    camTracker.stopThread();
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
        
        // step 2: face sub with two different images if possible
        if(camTracker.getFound()) {
            faceSubstitution.update(camTracker, cam, srcDelayPoints, srcDelay);
            faceSubstitution.clone.getTexture().readToPixels(substitutionDelay);
            substitutionDelay.setImageType(OF_IMAGE_COLOR);
            slitScan.addImage(substitutionDelay);
            faceSubstitution.update(camTracker, cam, srcOriginalPoints, srcOriginal);
        } else {
            slitScan.addImage(cam);
        }
        
        // step 3: motion amplification
        if(motionAmplifier.strength != 0) {
            amplifiedMotionOriginal.begin();
            if(camTracker.getFound()) {
                motionAmplifier.draw(faceSubstitution.clone.getTexture());
            } else {
                motionAmplifier.draw(cam);
            }
            amplifiedMotionOriginal.end();
            
            amplifiedMotionDelay.begin();
            motionAmplifier.draw(slitScan.getOutputImage());
            amplifiedMotionDelay.end();
        }
	}
}

void testApp::draw() {
    ofBackground(0);
	ofSetColor(255);
    
    float scale = ofGetWidth() / (float) cam.getWidth();
    ofPushMatrix();
    ofScale(scale, scale);
	
    ofTexture* left;
    ofTexture* right;
    
    if(motionAmplifier.strength != 0) {
        left = &amplifiedMotionOriginal.getTextureReference();
        right = &amplifiedMotionDelay.getTextureReference();
    } else {
        if(camTracker.getFound()) {
            left = &faceSubstitution.clone.getTexture();
        } else {
            left = &cam.getTextureReference();
        }
        right = &slitScan.getOutputImage().getTextureReference();
    }
    
    if(offset != 0) {
        float w = cam.getWidth();
        float h = cam.getHeight();
        lighten.begin();
        lighten.setUniform2f("resolution", w, h);
        lighten.setUniformTexture("a", *left, 1);
        lighten.setUniformTexture("b", *right, 2);
        lighten.setUniform2f("offset", offset, 0);
        cam.draw(0, 0);
        lighten.end();
    } else {
        ofPushMatrix();
        ofTranslate(left->getWidth(), 0);
        ofScale(-1, 1);
        left->draw(0, 0);
        ofPopMatrix();
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

void testApp::loadNextPair() {
    loadFace(faces.getPath(currentFace), srcOriginal, srcOriginalPoints);
    currentFace = (currentFace + 1) % faces.size();
    loadFace(faces.getPath(currentFace), srcDelay, srcDelayPoints);
    currentFace = (currentFace + 1) % faces.size();
}

void testApp::loadFace(string face, ofImage& src, vector<ofVec2f>& srcPoints){
	src.loadImage(face);
	if(src.getWidth() > 0) {
        srcPoints = faceSubstitution.getSrcPoints(src);
	}
}

void testApp::dragEvent(ofDragInfo dragInfo) {
	loadFace(dragInfo.files[0], srcOriginal, srcOriginalPoints);
}

int startX, startY;
float startOffset, startStrength;
void testApp::mousePressed(int x, int y, int button) {
    startX = x;
    startY = y;
    startOffset = offset;
    startStrength = substitutionStrength;
}

void testApp::mouseDragged(int x, int y, int button) {
    int xDiff = x - startX;
    int yDiff = y - startY;
    if(ofGetKeyPressed(OF_KEY_SHIFT)) {
        offset = startOffset + xDiff;
    } else {
        substitutionStrength = startStrength + xDiff / 10;
        substitutionStrength = ofClamp(substitutionStrength, 0, 64);
    }
}

void testApp::keyPressed(int key){
    if(key == '0') {
        offset = 0;
    }
    if(key == 'f') {
        ofToggleFullscreen();
    }
    if(key == '\t') {
        gui->toggleVisible();
        if(gui->isVisible()) {
            ofShowCursor();
        } else {
            ofHideCursor();
        }
    }
}

void testApp::keyReleased(int key) {
    if(key == ' ') {
        loadNextPair();
    }
}
