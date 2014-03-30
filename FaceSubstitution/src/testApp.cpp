#include "testApp.h"

using namespace ofxCv;

void testApp::setupGui() {
    gui = new ofxUICanvas();
    gui->addFPS();
    gui->addToggle("Debug", &debug);
    gui->addToggle("Enable face sub", &enableFaceSubstitution);
    gui->addToggle("Enable slit scan", &enableSlitScan);
    gui->addToggle("Enable motion amp", &enableMotionAmplifier);
    gui->addSlider("Offset", 0, 600, &offset);
    gui->addSlider("Motion strength", -100, 100, &motionAmplifier.strength);
    gui->addSlider("Motion learning rate", 0, 1, &motionAmplifier.learningRate);
    gui->addSlider("Motion blur amount", 0, 15, &motionAmplifier.blurAmount);
    gui->addSlider("Motion window size", 1, 64, &motionAmplifier.windowSize);
    gui->autoSizeToFitWidgets();
}

void testApp::setup() {
    ofSetDataPathRoot("../../../../../SharedData/");
	ofSetVerticalSync(true);
	cloneReady = false;
    
#ifdef USE_VIDEO
    cam.loadMovie("videos/milos-talking.mov");
    cam.play();
#else
    cam.setDeviceID(0);
	cam.initGrabber(1280, 720);
#endif
    
	clone.setup(cam.getWidth(), cam.getHeight());
	ofFbo::Settings settings;
	settings.width = cam.getWidth();
	settings.height = cam.getHeight();
	maskFbo.allocate(settings);
	srcFbo.allocate(settings);
	camTracker.setup();
	srcTracker.setup();
	srcTracker.setIterations(25);
	srcTracker.setAttempts(4);
    
	faces.allowExt("jpg");
	faces.allowExt("png");
	faces.listDir("faces");
	currentFace = 0;
	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}
    
    binaryEffects.load("", "shaders/BinaryEffects.frag");
    binary.allocate(cam.getWidth(), cam.getHeight());
    displacement.load("shaders/Displacement");
    
    faceOsc.osc.setup("192.168.0.255", 8338);
    
    ofImage distortionMap;
    distortionMap.loadImage("images/white.png");
    slitScan.setup(cam.getWidth(), cam.getHeight(), 60, OF_IMAGE_COLOR);
    slitScan.setDelayMap(distortionMap);
    slitScan.setBlending(true);
    slitScan.setTimeDelayAndWidth(60, 0);
    
    lighten.load("shaders/Lighten");
    
    motionAmplifier.setup(cam.getWidth(), cam.getHeight(), 1, .1);
    amplifiedMotion.allocate(cam.getWidth(), cam.getHeight());
    
    setupGui();
}

void testApp::exit() {
    camTracker.stopThread();
    cam.close();
}

void testApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
        if(enableSlitScan) {
            slitScan.addImage(cam);
        }
        
        if(enableMotionAmplifier) {
            amplifiedMotion.begin();
            if(enableSlitScan) {
                motionAmplifier.update(slitScan.getOutputImage());
                motionAmplifier.draw(slitScan.getOutputImage());
            } else {
                motionAmplifier.update(cam);
                motionAmplifier.draw(cam);
            }
            amplifiedMotion.end();
        }
        
        if(enableFaceSubstitution) {
            camTracker.update(toCv(cam));
            faceOsc.sendFaceOsc(camTracker);
            cloneReady = camTracker.getFound();
            if(cloneReady) {
                binary.begin();
                binaryEffects.begin();
                binaryEffects.setUniform3f("iResolution", cam.getWidth(), cam.getHeight(), 0);
                binaryEffects.setUniform1f("iGlobalTime", ofGetElapsedTimef());
                cam.draw(0, 0);
                binaryEffects.end();
                binary.end();
                
                ofMesh camMesh = camTracker.getImageMesh();
                camMesh.clearTexCoords();
                camMesh.addTexCoords(srcPoints);
                
                maskFbo.begin();
                ofClear(0, 255);
                camMesh.draw();
                ofPushStyle();
                ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
                binary.draw(0, 0);
                ofPopStyle();
                maskFbo.end();
                
                srcFbo.begin();
                ofClear(0, 255);
                src.bind();
                camMesh.draw();
                src.unbind();
                srcFbo.end();
                
                clone.setStrength(16);
                clone.update(srcFbo.getTextureReference(), cam.getTextureReference(), maskFbo.getTextureReference());
            }
        }
	}
}

void testApp::draw() {
    ofBackground(0);
	ofSetColor(255);
    
    float scale = ofGetWidth() / (float) cam.getWidth();
    ofPushMatrix();
    ofScale(scale, scale);
	
    float w = cam.getWidth();
    float h = cam.getHeight();
    lighten.begin();
    lighten.setUniform2f("resolution", w, h);
    if(enableFaceSubstitution) {
        lighten.setUniformTexture("a", clone.getTexture(), 1);
    } else {
        lighten.setUniformTexture("a", cam, 1);
    }
    if(enableMotionAmplifier) {
        lighten.setUniformTexture("b", amplifiedMotion, 2);
    } else {
        if(enableSlitScan) {
            lighten.setUniformTexture("b", slitScan.getOutputImage(), 2);
        } else {
            lighten.setUniformTexture("b", cam, 2);
        }
    }
    lighten.setUniform2f("offset", offset, 0);
    cam.draw(0, 0);
    lighten.end();
    
    if(debug) {
        camTracker.draw();
        ofScale(.2, .2);
        maskFbo.draw(0, 0);
        ofTranslate(0, cam.getHeight());
        srcFbo.draw(0, 0);
        ofTranslate(0, cam.getHeight());
        ofScale(1./motionAmplifier.getRescale(), 1./motionAmplifier.getRescale());
        motionAmplifier.getFlowTexture().draw(0, 0);
    }
    
    ofPopMatrix();
}

void testApp::loadFace(string face){
	src.loadImage(face);
	if(src.getWidth() > 0) {
		srcTracker.update(toCv(src));
		srcPoints = srcTracker.getImagePoints();
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
	currentFace = ofClamp(currentFace,0,faces.size()-1);
	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}
}
