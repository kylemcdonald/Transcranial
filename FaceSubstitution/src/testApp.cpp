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
    ofLog() << "Loading video.";
    cam.loadMovie("videos/milos-extreme.mov");
    cam.play();
#else
    ofLog() << "Setting up camera.";
    cam.setDeviceID(0);
    cam.initGrabber(1280, 720);
    #ifdef USE_EDSDK
    ofLog() << "Using EDSDK.";
    cam.setDeviceType(EDSDK_T2I);
    #else
    ofLog() << "Using webcam.";
    #endif
#endif
    camTimer.setSmoothing(.99);
    
	camTracker.setup();
    camTracker.setRescale(trackerRescale);
    camTracker.setHaarMinSize(cam.getHeight() / 4);
    
    faceSubstitution.setup(cam.getWidth(), cam.getHeight());
    
	faceMeshes.allowExt("ply");
	faceMeshes.listDir("meshes");
	currentFace = 0;
    loadNextPair();

    slitScan.setup(cam.getWidth(), cam.getHeight(), 100);
    slitScan.setBlending(false);
    slitScan.setTimeDelayAndWidth(0, 0);
    delaySeconds = 3;
    delaySync.setPeriod(1);
    
    lighten.load("shaders/Lighten");
    
    motionAmplifier.setup(cam.getWidth(), cam.getHeight(), 1, .25);
    amplifiedMotionOriginal.allocate(cam.getWidth(), cam.getHeight());
    amplifiedMotionDelay.allocate(cam.getWidth(), cam.getHeight());
    
    setupGui();
}

void testApp::exit() {
    camTracker.stopThread();
#ifdef USE_EDSDK
    cam.close();
#endif
}

void testApp::update() {    
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
        
        // optical flow
        if(prevCam.getWidth()) {
            motionAmplifier.update(slitScan.getOutputImage());
        }
        
        // step 2: face sub with two different images if possible
        if(camTracker.getFound()) {
            faceSubstitution.update(camTracker, prevCam, srcDelayPoints, srcDelay);
            faceSubstitution.clone.getTexture().readToPixels(substitutionDelay);
            substitutionDelay.setImageType(OF_IMAGE_COLOR);
            slitScan.addImage(substitutionDelay);
            faceSubstitution.update(camTracker, prevCam, srcOriginalPoints, srcOriginal);
        } else {
            slitScan.addImage(prevCam);
        }
        
        // step 3: motion amplification
        if(motionAmplifier.strength != 0) {
            amplifiedMotionOriginal.begin();
            if(camTracker.getFound()) {
                motionAmplifier.draw(faceSubstitution.clone.getTexture());
            } else {
                motionAmplifier.draw(prevCam);
            }
            amplifiedMotionOriginal.end();
            
            amplifiedMotionDelay.begin();
            motionAmplifier.draw(slitScan.getOutputImage());
            amplifiedMotionDelay.end();
        }
        
        copy(cam, prevCam);
        prevCam.update();
        
        if(delaySync.tick()) {
            float delayFrames = MIN(delaySeconds * camTimer.getFrameRate(), slitScan.getCapacity());
            slitScan.setTimeDelayAndWidth(delayFrames, 0);
        }
	}
}

void testApp::draw() {
    ofBackground(0);
	ofSetColor(255);
    
    float scale = ofGetHeight() / (float) cam.getHeight();
    ofPushMatrix();
    ofTranslate(ofGetWidth() / 2, 0);
    ofScale(scale, scale);
    ofTranslate(-cam.getWidth() / 2, 0);
	
    ofTexture* left;
    ofTexture* right;
    
    if(motionAmplifier.strength != 0) {
        left = &amplifiedMotionOriginal.getTexture();
        right = &amplifiedMotionDelay.getTexture();
    } else {
        if(camTracker.getFound()) {
            left = &faceSubstitution.clone.getTexture();
        } else {
            left = &cam.getTexture();
        }
        right = &slitScan.getOutputImage().getTexture();
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
    loadFace(faceMeshes.getFile(currentFace), srcOriginal, srcOriginalPoints);
    currentFace = (currentFace + 1) % faceMeshes.size();
    loadFace(faceMeshes.getFile(currentFace), srcDelay, srcDelayPoints);
    currentFace = (currentFace + 1) % faceMeshes.size();
}

void testApp::loadFace(ofFile faceMesh, ofImage& src, vector<ofVec2f>& srcPoints){
    ofMesh mesh;
    mesh.load(faceMesh.path());
    srcPoints.clear();
    for(auto vertex : mesh.getVertices()) {
        srcPoints.push_back(vertex);
    }
    string faceImage = "faces/" + faceMesh.getBaseName() + ".jpg";
    src.loadImage(faceImage);
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
    if(key == '.') {
        loadNextPair();
    }
    // if key == ' ' then wait half a sec and change the face
    // if key == ' ' the first time then fade in the face sub
    // if key == OF_KEY_SHIFT slowly move face to sides
    // if keey == '0' need to fade back to single face
}

