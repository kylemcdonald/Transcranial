#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxKeyValueOSC.h"
#include "ofxFaceTracker.h"
class FacePoly{
public:
    ofPolyline poly;
    vector<int>ids;

    float area;
    float minArea, maxArea, normArea, baseArea, diffNormArea;
    bool bCalibration;
    bool bDebug;
    void setup(){
        bDebug = false;
        reset();
        bCalibration = false;
    }
    void reset(){
        minArea = 1000000;//FLT_MAX;
        maxArea = -1000000;//FLT_MIN;
        normArea = 0.01;
        area = 0.01;
        baseArea = 0.01;
        diffNormArea = 0.01;
        bCalibration = false;
    }
    void setBase(){
        baseArea = area;
    }
    void update(const ofMesh & mesh){
        poly.clear();
        for (int i=0; i<ids.size(); i++) {
            poly.addVertex(mesh.getVertices()[ids[i]] );
        }
        poly.close();
        area = fabs(poly.getArea());
        if (bCalibration) {
            minArea = MIN(minArea, area);
            maxArea = MAX(maxArea, area);
        }
        normArea = ofMap(area, minArea, maxArea, 0., 1., true);
//        float far = (maxArea - baseArea) >(baseArea - minArea) ? maxArea : minArea;
        diffNormArea = fabs(baseArea - area) / fabs(maxArea - minArea);
        // limitter
        diffNormArea = ofClamp(diffNormArea, 0, 1);
        // optmise
        diffNormArea = ofMap(diffNormArea, 0., 0.45, 0.0, 1.0, true);
        //change curve
        diffNormArea = sin(diffNormArea * PI/2.0);
    }
    void draw(){
        ofSetColor(ofColor::fromHsb(180 * (1 - diffNormArea), 255, 255, 100) );
        ofFill();
        ofSetPolyMode(OF_POLY_WINDING_ODD);
        ofBeginShape();
        ofVertices(poly.getVertices());
        ofEndShape();

        if (bDebug) {
            poly.draw();
            ofCircle(poly.getCentroid2D(), 50 * diffNormArea);
            stringstream ss;
            ss << minArea << endl;
            ss << maxArea << endl;
            ss << diffNormArea << endl;
            ofSetColor(255);
            ofDrawBitmapString(ss.str(), poly.getCentroid2D());
            ofNoFill();
        }

        if(bCalibration){
            ofSetColor(0, 255, 0);
            ofRect(poly.getCentroid2D(), 5,5);
        }

    }
    void toggleCalibration(){
        bCalibration = !bCalibration;
    }
    void setCalibration(bool b){
        bCalibration = b;
    }

    void pushid(int pid){
        ids.push_back(pid);
    }
    float getDiffNormArea()const {
        return diffNormArea;
    }
    float getMinArea() const{
        return minArea;
    }
    float getBaseArea() const{
        return baseArea;
    }
    float getMaxArea() const{
        return maxArea;
    }
    float setMinArea(float s){
        minArea = s;
    }
    float setMaxArea(float s){
        maxArea = s;
    }
    float setBaseArea(float s){
        baseArea = s;
    }
};

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
    void keyPressed(int key);
    void load(string filename);
    void save(string filename);
    void drawClassifierInfo();
    void setupFacePolygons();

	ofVideoPlayer video;
	ofxFaceTracker tracker;
    vector<ofMesh> trackedImagePoints;
    vector<ofMesh> trackedObjectPoints;
    vector<vector<float> > trackedGestures;
    
    vector<ofMesh> recordedImagePoints;
    vector<ofMesh> recordedObjectPoints;
    vector<vector<float> > recordedGestures;
	ExpressionClassifier classifier;
    vector<FacePoly>polygons;
    ofxKeyValueOSC keyvalue;
};
