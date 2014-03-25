#include "ofAppGlutWindow.h"
#include "ofxCv.h"
#include "ofMain.h"
#include "ofxEdsdk.h"

using namespace ofxCv;
using namespace cv;

class ofApp : public ofBaseApp {
public:
	ofxEdsdk::Camera camera;
    ofxCv::ContourFinder contours;
    
    void setup() {
        ofSetVerticalSync(true);
        ofSetLogLevel(OF_LOG_VERBOSE);
        camera.setup();
    }
    
    void update() {
        camera.update();
        if(camera.isFrameNew()) {
            contours.setThreshold(mouseX);
            contours.setMinAreaRadius(10);
            contours.findContours(camera.getLivePixels());
        }
    }
    
    void draw() {
        float scaleFactor = ofGetWidth() / 1024.;
        ofScale(scaleFactor, scaleFactor);
        
        camera.draw(0, 0);
        // camera.drawPhoto(0, 0, 432, 288);
        ofSetLineWidth(3);
        contours.draw();
        
        if(camera.isLiveReady()) {
            stringstream status;
            status << camera.getWidth() << "x" << camera.getHeight() << " @ " <<
            (int) ofGetFrameRate() << " app-fps " << " / " <<
            (int) camera.getFrameRate() << " cam-fps";
            ofDrawBitmapString(status.str(), 10, 20);
        }
    }
    
    void keyPressed(int key) {
        if(key == 'f') {
            ofToggleFullscreen();
        }
    }
};

int main() {
	ofAppGlutWindow window;
    ofSetupOpenGL(&window, 1920, 1200, OF_FULLSCREEN);
//	ofSetupOpenGL(&window, 1024, 680, OF_WINDOW); // mkii
//	ofSetupOpenGL(&window, 1056, 704, OF_WINDOW); // t2i
	ofRunApp(new ofApp());
}
