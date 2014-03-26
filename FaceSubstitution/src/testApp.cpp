#include "testApp.h"

using namespace ofxCv;

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
    
    faceOsc.osc.setup("localhost", 8338);
}

void testApp::update() {
    binary.begin();
    binaryEffects.begin();
    binaryEffects.setUniform3f("iResolution", cam.getWidth(), cam.getHeight(), 0);
    binaryEffects.setUniform1f("iGlobalTime", ofGetElapsedTimef());
    cam.draw(0, 0);
    binaryEffects.end();
    binary.end();
    
	cam.update();
	if(cam.isFrameNew()) {
		camTracker.update(toCv(cam));
        faceOsc.sendFaceOsc(camTracker);
		
		cloneReady = camTracker.getFound();
		if(cloneReady) {
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

void testApp::draw() {
    ofBackground(0);
	ofSetColor(255);
    
    float scale = ofGetWidth() / (float) cam.getWidth();
    ofScale(scale, scale);
	
	if(src.getWidth() > 0 && cloneReady) {
        float w = clone.getTexture().getWidth();
        float h = clone.getTexture().getHeight();
        ofEnableBlendMode(OF_BLENDMODE_ADD);
		clone.getTexture().setAnchorPercent(0, 0);
        clone.getTexture().draw(0, 0);
		clone.getTexture().setAnchorPercent(0, 1);
        clone.getTexture().draw(0, h, w  / 2, h / 2);
		clone.getTexture().setAnchorPercent(1, 1);
        clone.getTexture().draw(w, h, w  / 2, h / 2);
		clone.getTexture().setAnchorPercent(0, 0);
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        camTracker.draw();
	} else {
		cam.draw(0, 0);
	}
	
	if(!camTracker.getFound()) {
		drawHighlightString("camera face not found", 10, 10);
	}
	if(src.getWidth() == 0) {
		drawHighlightString("drag an image here", 10, 30);
	} else if(!srcTracker.getFound()) {
		drawHighlightString("image face not found", 10, 30);
	}
    
    ofScale(.5, .5);
//    maskFbo.draw(0, 0);
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
