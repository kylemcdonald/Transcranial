#include "testApp.h"

using namespace ofxCv;

void testApp::setup() {
#ifdef TARGET_OSX
	//ofSetDataPathRoot("../data/");
#endif
	ofSetVerticalSync(true);
	cloneReady = false;
    cam.setDeviceID(0);
	cam.initGrabber(1280, 720);
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
}

void testApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
		camTracker.update(toCv(cam));
		
		cloneReady = camTracker.getFound();
		if(cloneReady) {
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
			
			clone.setStrength(16);
			clone.update(srcFbo.getTextureReference(), cam.getTextureReference(), maskFbo.getTextureReference());
		}
	}
}

void testApp::draw() {
    ofBackground(0);
	ofSetColor(255);
	
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
	switch(key){
	case OF_KEY_UP:
		currentFace++;
		break;
	case OF_KEY_DOWN:
		currentFace--;
		break;
	}
	currentFace = ofClamp(currentFace,0,faces.size());
	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}
}
