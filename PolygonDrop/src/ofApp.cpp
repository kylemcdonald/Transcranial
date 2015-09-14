#include "ofApp.h"

static bool shouldRemove(shared_ptr<ofxBox2dBaseShape>shape) {
    return !ofRectangle(0, -400, ofGetWidth(), ofGetHeight()+400).inside(shape->getPosition());
}

//--------------------------------------------------------------
void ofApp::setup() {
    ofSetVerticalSync(true);
	ofBackgroundHex(0xfdefc2);
	ofSetLogLevel(OF_LOG_NOTICE);
	
    cam.initGrabber(640, 480);
    contourFinder.setMinAreaRadius(10);
    contourFinder.setMaxAreaRadius(200);
    
	// Box2d
	box2d.init();
	box2d.setGravity(0, 10);
	box2d.createGround();
	box2d.setFPS(30.0);
	    
	// load the shape we saved...
	vector <ofPoint> pts = loadPoints("shape.dat");
    shared_ptr<ofxBox2dPolygon> poly = shared_ptr<ofxBox2dPolygon>(new ofxBox2dPolygon);
    poly->addVertices(pts);
    poly->setPhysics(1.0, 0.3, 0.3);
    poly->triangulatePoly();
    poly->create(box2d.getWorld());
	polyShapes.push_back(poly);
	
}

//--------------------------------------------------------------
vector <ofPoint> ofApp::loadPoints(string file) {
    vector <ofPoint> pts;
    vector <string>  ptsStr = ofSplitString(ofBufferFromFile(file).getText(), ",");
    for (int i=0; i<ptsStr.size(); i+=2) {
        float x = ofToFloat(ptsStr[i]);
        float y = ofToFloat(ptsStr[i+1]);
        pts.push_back(ofPoint(x, y));
    }
	return pts;
}


//--------------------------------------------------------------
void ofApp::update() {
    // remove shapes offscreen
    ofRemove(polyShapes, shouldRemove);
	box2d.update();
    
    cam.update();
    if(cam.isFrameNew()) {
        contourFinder.setThreshold(ofMap(mouseX, 0, ofGetWidth(), 0, 255));
        contourFinder.findContours(cam);
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofSetColor(255);
    cam.draw(0, 0);
    contourFinder.draw();
    
	ofSetHexColor(0x444342);
	ofFill();
	shape.draw();
	
	ofSetHexColor(0x444342);
	ofNoFill();
	for (int i=0; i<polyShapes.size(); i++) {
		polyShapes[i]->draw();
        
        ofCircle(polyShapes[i]->getPosition(), 3);
	}	
    
	
	// some debug information
	string info = "Draw a shape with the mouse\n";
	info += "Press c to clear everything\n";
    info += "Total Bodies: "+ofToString(box2d.getBodyCount())+"\n";
	info += "Total Joints: "+ofToString(box2d.getJointCount())+"\n\n";
	info += "FPS: "+ofToString(ofGetFrameRate())+"\n";
    ofSetHexColor(0x444342);
	ofDrawBitmapString(info, 10, 15);
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if(key == 'c') {
		shape.clear();
        polyShapes.clear();
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ) {
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	shape.addVertex(x, y);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	shape.clear();
	shape.addVertex(x, y);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

    shared_ptr<ofxBox2dPolygon> poly = shared_ptr<ofxBox2dPolygon>(new ofxBox2dPolygon);
    poly->addVertices(shape.getVertices());
    poly->setPhysics(1.0, 0.3, 0.3);
    poly->triangulatePoly();
    poly->create(box2d.getWorld());
    polyShapes.push_back(poly);

    // done with shape clear it now
    shape.clear();
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
}

