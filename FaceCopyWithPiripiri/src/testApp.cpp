#include "testApp.h"
#include "ofxJSONElement.h"
#include "ofxSyphonClient.h"

using namespace cv;
using namespace ofxCv;

class ofxOscMultiSender
{
public:

    void setup(std::string hostname, int port)
    {
        ofxOscSender *sender = new ofxOscSender();
        sender->setup(hostname, port);
        senders.push_back(SenderPtr(sender));
    }

    void sendMessage( ofxOscMessage& message )
    {
        for (int i = 0; i < senders.size(); i++)
            senders[i]->sendMessage(message);
    }

    void sendBundle( ofxOscBundle& bundle )
    {
        for (int i = 0; i < senders.size(); i++)
            senders[i]->sendBundle(bundle);
    }

protected:
    
    typedef ofPtr<ofxOscSender> SenderPtr;
    vector<SenderPtr> senders;
};



ofxOscMultiSender sender;
ofxSyphonServer syphoneServer;

ofxFaceTracker::Gesture gestureIds[] = {
    ofxFaceTracker::MOUTH_WIDTH,
    ofxFaceTracker::MOUTH_HEIGHT,
    ofxFaceTracker::LEFT_EYEBROW_HEIGHT,
    ofxFaceTracker::RIGHT_EYEBROW_HEIGHT,
    ofxFaceTracker::LEFT_EYE_OPENNESS,
    ofxFaceTracker::RIGHT_EYE_OPENNESS,
    ofxFaceTracker::JAW_OPENNESS,
    ofxFaceTracker::NOSTRIL_FLARE
};

string gestureNames[] = {
    "mouthWidth",
    "mouthHeight",
    "leftEyebrowHeight",
    "rightEyebrowHeight",
    "leftEyeOpenness",
    "rightEyeOpenness",
    "jawOpenness",
    "nostrilFlare"
};

int gestureCount = 8;
bool bExport = true;
void testApp::save(string filename){
    ofMesh imageMesh, objectMesh;
    vector<float> curGesture;    // video
    int numFrames = video.getTotalNumFrames();
    for (int i=0; i<numFrames; i++) {
        video.setFrame(i);
        video.update();
        tracker.update(toCv(video));
        if (tracker.getFound()) {
            imageMesh = tracker.getImageMesh();
            objectMesh = tracker.getObjectMesh();
            
            trackedImagePoints.push_back(imageMesh);
            trackedObjectPoints.push_back(objectMesh);
            curGesture.clear();
            for(int i = 0; i < gestureCount; i++) {
                curGesture.push_back(tracker.getGesture(gestureIds[i]));
            }
            trackedGestures.push_back(curGesture);
            cout << "push_back tracker data : " << i << " / " << numFrames << endl;
        }
        else{
            trackedImagePoints.push_back(imageMesh);
            trackedObjectPoints.push_back(objectMesh);
            for(int i = 0; i < gestureCount; i++) {
                curGesture.push_back(tracker.getGesture(gestureIds[i]));
            }
            trackedGestures.push_back(curGesture);

            cout << "---------------cant track" << endl;
        }
    }
    
    // json
    ofFile out(filename, ofFile::WriteOnly);
    out << "{\"results\": [";
    
    for(int i = 0; i < numFrames; i++) {
        out << "{";
        //gestures
        for(int j = 0; j < gestureCount; j++) {
            out << "\""+gestureNames[j]+"\"" << ":" << trackedGestures[i][j] << "," << endl;
        }
        
        //imagepoints
        out << "\"imagepoints\": [";
        for(int j = 0; j < trackedImagePoints[i].getNumVertices(); j++) {
            if (j == trackedImagePoints[i].getNumVertices() - 1) {
                out << "[" << ofVec2f(trackedImagePoints[i].getVertex(j)) << "]" << endl;
            }else{
                out << "[" << ofVec2f(trackedImagePoints[i].getVertex(j)) << "]," << endl;
            }
        }
        out << "],";
        
        //objectpoints
        out <<"\"objectpoints\": [";
        for(int j = 0; j < trackedObjectPoints[i].getNumVertices(); j++) {
            if (j == trackedObjectPoints[i].getNumVertices() - 1) {
                out << "[" << ofVec2f(trackedObjectPoints[i].getVertex(j)) << "]" << endl;
            }else{
                out << "[" << ofVec2f(trackedObjectPoints[i].getVertex(j)) << "]," << endl;
            }
        }
        out << "]"<<endl;
        
        if (i == numFrames - 1) {
            out << "}"<<endl;
            
        }else{
            out << "},"<<endl;
        }
    }
    out << "]}";
    
}
void testApp::load(string filename){
    ofxJSONElement element;
    element.open(filename);
    for (int i=0; i<element["results"].size(); i++) {
        vector<float>val_list;
        for (int j=0; j<gestureCount; j++) {
            float val = element["results"][i][gestureNames[j]].asFloat();
            val_list.push_back(val);
        }
        recordedGestures.push_back(val_list);

        ofMesh imagemesh;
        for (int j=0; j<element["results"][i]["imagepoints"].size(); j++) {
            float px = element["results"][i]["imagepoints"][j][0].asFloat();
            float py = element["results"][i]["imagepoints"][j][1].asFloat();
            imagemesh.addVertex(ofVec3f(px, py, 0));
        }
        recordedImagePoints.push_back(imagemesh);
        
        ofMesh objectmesh;
        for (int j=0; j<element["results"][i]["objectpoints"].size(); j++) {
            float px = element["results"][i]["objectpoints"][j][0].asFloat();
            float py = element["results"][i]["objectpoints"][j][1].asFloat();
            objectmesh.addVertex(ofVec3f(px, py, 0));
        }
        recordedObjectPoints.push_back(objectmesh);
    }
    element.clear();
}




void savePolygonParameters(const vector<FacePoly> & polys, string filename){

    // json
    ofFile out(filename, ofFile::WriteOnly);
    out << "{\"results\": ["<< endl;;
    
    for(int i = 0; i < polys.size(); i++) {
        out << "{";
        //gestures
        out << "\"baseArea\" :" << polys[i].getBaseArea() << ","<< endl;;
        out << "\"minArea\" :" << polys[i].getMinArea() << ","<< endl;;
        out << "\"maxArea\" :" << polys[i].getMaxArea() << endl;;
        if (i == polys.size() - 1) {
            out << "}"<< endl;;
            
        }else{
            out << "},"<< endl;;
        }
    }
    out << "]}"<<endl;
}

void loadPolygonParameters(vector<FacePoly> & polys, string filename){

    // json
    ofxJSONElement element;
    element.open(filename);

    for (int i=0; i<element["results"].size(); i++) {
        polys[i].setBaseArea(element["results"][i]["baseArea"].asFloat());
        polys[i].setMinArea(element["results"][i]["minArea"].asFloat());
        polys[i].setMaxArea(element["results"][i]["maxArea"].asFloat());
    }
    element.clear();

}

void testApp::setupFacePolygons(){
    polygons.resize(6);
    for (int i=0;i<polygons.size() ;i++ ){
        polygons[i].setup();
    }

    //lefteye
    polygons[0].pushid(17);
    polygons[0].pushid(18);
    polygons[0].pushid(19);
    polygons[0].pushid(20);
    polygons[0].pushid(21);
    polygons[0].pushid(39);
    polygons[0].pushid(38);
    polygons[0].pushid(37);
    polygons[0].pushid(36);

    //lefteye
    polygons[1].pushid(22);
    polygons[1].pushid(23);
    polygons[1].pushid(24);
    polygons[1].pushid(25);
    polygons[1].pushid(26);
    polygons[1].pushid(45);
    polygons[1].pushid(44);
    polygons[1].pushid(43);
    polygons[1].pushid(42);
    //left cheeks
    polygons[2].pushid(48);
    polygons[2].pushid(49);
    polygons[2].pushid(31);
    polygons[2].pushid(3);
    polygons[2].pushid(4);

    //right cheeks
    //    polygons[3].pushid(54);
    polygons[3].pushid(53);
    polygons[3].pushid(35);
    polygons[3].pushid(13);
    polygons[3].pushid(12);

    //left mouth
    //    polygons[4].pushid(48);
    polygons[4].pushid(59);
    polygons[4].pushid(58);
    polygons[4].pushid(7);
    polygons[4].pushid(6);
    polygons[4].pushid(5);

    //right mouth
    polygons[5].pushid(54);
    polygons[5].pushid(55);
    polygons[5].pushid(56);
    polygons[5].pushid(9);
    polygons[5].pushid(10);
    polygons[5].pushid(11);

}

int currentFrame = 0;
float polygonsAlpha = 1;
float videoAlpha = 1.;

void testApp::setup() {
    ofBackground(0, 0, 0);
//    ofSetDataPathRoot("../../../../../SharedData/");
    ofSetDataPathRoot("../../../../../SharedData/");
    sender.setup("localhost", 8877);
    sender.setup("10.0.0.2", 8877);
    
    keyvalue.setup(8866);
    syphoneServer.setName("FacePiripiri");
    video.loadMovie("videos/transcranial_scopiton_1080.MOV");
    cout << " -- video.getTotalNumFrames() "<< endl;
    cout << video.getTotalNumFrames() << endl;
    ofSetWindowShape(video.getWidth(), video.getHeight());
	tracker.setup();
    tracker.setRescale(.25);
    tracker.setIterations(100);
    tracker.setClamp(10);
    tracker.setTolerance(.5);
    tracker.setAttempts(4);
    
    setupFacePolygons();
    load("jsons/data.json");
    loadPolygonParameters(polygons, "jsons/polygondata.json");

}

void testApp::update() {
//    ofSetWindowTitle(ofToString(currentFrame) + " / " + ofToString(video.getTotalNumFrames()));
    
    if (recordedImagePoints.size()>0) {
        keyvalue.get("/current_frame", currentFrame);
        currentFrame = ofClamp(currentFrame, 0, recordedImagePoints.size()-1);
        if ( ofInRange(currentFrame, 0, video.getTotalNumFrames()-1) ) {
            video.setFrame(currentFrame);
        }
        video.update();

        for (int i=0;i<polygons.size() ;i++ ){

            polygons[i].update(recordedImagePoints[currentFrame]);
            //osc
            ofxOscMessage m;
            m.setAddress("/faceparts");
            m.addIntArg(i + 1); //ch number starts from 1 in Max
            m.addFloatArg(polygons[i].getDiffNormArea() );
            sender.sendMessage(m);
        }
    }
}

void testApp::draw() {
    
    if (recordedImagePoints.size()>0) {
        ofPushMatrix();
        float scale = ofGetWidth() / video.getWidth();
        ofScale(scale, scale);
        keyvalue.get("/polygons_alpha", polygonsAlpha);
        keyvalue.get("/video_alpha", videoAlpha);
        ofSetColor(ofColor::white, videoAlpha * 255.);
        video.draw(0, 0);
        for (int i=0;i<polygons.size() ;i++ ){
            polygons[i].draw(polygonsAlpha);
        }
        ofPopMatrix();
    }

    syphoneServer.publishScreen();
}
void testApp::keyPressed(int key) {
    if(key == 'L'){
        load("data.json");
    }
    if(key == 'S') {
        save("data.json");
    }
    if(key == OF_KEY_LEFT){
        currentFrame--;
        if (currentFrame<0) {
            currentFrame = video.getTotalNumFrames() - 2;
        }
    }
    if(key == OF_KEY_RIGHT){
        currentFrame++;
        if (currentFrame>video.getTotalNumFrames() - 3) {
            currentFrame =  0;
        }

    }
    if(key == 'b'){
        for (int i=0; i<polygons.size(); i++) {
            polygons[i].setBase();
        }
    }
    if(key == 'p'){
        savePolygonParameters(polygons, "polygondata.json");
    }
    if(key == 'P'){
        loadPolygonParameters(polygons, "polygondata.json");
        for (int i=0; i<polygons.size(); i++) {
            polygons[i].setCalibration(false);
        }
    }

    if(key == 'c'){
        for (int i=0; i<polygons.size(); i++) {
            polygons[i].toggleCalibration();
        }
    }
}