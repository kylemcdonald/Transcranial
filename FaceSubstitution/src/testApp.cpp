#include "testApp.h"

using namespace ofxCv;

float smoothestStep(float t) {
    float t2 = t * t;
    return (-20*t2*t+70*t2-84*t+35)*t2*t2;
}

void testApp::setupGui() {
    gui = new ofxUICanvas();
    gui->addFPS();
    gui->addToggle("Debug", &(debug=false));
    gui->addSlider("Max offset", 0, 600, &(maxOffset=250));
    gui->addSlider("Tracker rescale", .1, 1, &(trackerRescale=.5));
    gui->addSlider("Substitution strength", 0, 64, &(substitutionStrength=20));
    gui->addSlider("Motion max", 0, 100, &(motionMax=24));
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
    substitutionTimer.setLength(10, 0);
    
	faceMeshes.allowExt("ply");
	faceMeshes.listDir("meshes");
	currentFace = 0;
    loadNextPair();

    slitScan.setup(cam.getWidth(), cam.getHeight(), 100);
    slitScan.setBlending(false);
    slitScan.setTimeDelayAndWidth(0, 0);
    delaySeconds = 3;
    delaySync.setPeriod(1);
    
    offsetTimer.setLength(.5, 1);
    offsetDirection = false;
    
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
    float normalizedMotion = 0;
    if(ofGetKeyPressed('1')) normalizedMotion = .33;
    if(ofGetKeyPressed('2')) normalizedMotion = .66;
    if(ofGetKeyPressed('3') || ofGetKeyPressed(' ')) normalizedMotion = 1.0;
    motionAmplifier.strength = normalizedMotion * motionMax;
    
    if(ofGetKeyPressed('a')) motionAmplifier.learningRate = .1;
    else motionAmplifier.learningRate = .9;
    
    camTracker.setRescale(trackerRescale);
    faceSubstitution.clone.setStrength(smoothestStep(substitutionTimer.get()) *substitutionStrength);
    
	cam.update();
	if(cam.isFrameNew()) {
        camTimer.tick();
        
        // step 1: face tracking and optical flow on current image
        camTracker.update(toCv(cam));
        if(prevCam.getWidth()) {
            motionAmplifier.update(cam);
        }
        
        // step 2: face sub onto present and future if possible
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
        if(motionAmplifier.strength > 0) {
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
    
    if(offsetTimer.getActive()) {
        float offset = smoothestStep(offsetTimer.get()) * maxOffset;
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
    src.load(faceImage);
}

void testApp::dragEvent(ofDragInfo dragInfo) {
	loadFace(dragInfo.files[0], srcOriginal, srcOriginalPoints);
}

int startX, startY;
float startStrength;
void testApp::mousePressed(int x, int y, int button) {
    startX = x;
    startY = y;
    startStrength = substitutionStrength;
}

void testApp::mouseDragged(int x, int y, int button) {
    int xDiff = x - startX;
    int yDiff = y - startY;
    substitutionStrength = startStrength + xDiff / 10;
    substitutionStrength = ofClamp(substitutionStrength, 0, 64);
}

void testApp::keyPressed(int key){
    if(key == OF_KEY_SHIFT) {
        offsetDirection = !offsetDirection;
        if(offsetDirection) {
            offsetTimer.start();
        } else {
            offsetTimer.stop();
        }
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
        substitutionTimer.start();
    }
}

