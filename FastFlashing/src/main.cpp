#include "ofMain.h"
#include "ofAppGlutWindow.h"

#define MAX_PHASE_OFFSET 120

class ofApp : public ofBaseApp {
public:
    int w = 1024, h = 768;
    int n = 2;
    int phase = 0;
    bool fullscreen = false;
    bool halves = false;
    void setup() {
        ofSetVerticalSync(true);
//        ofSetBackgroundAuto(false);
    }
    void update() {
    }
    void draw() {
        int mod = (MAX_PHASE_OFFSET + phase + ofGetFrameNum()) % n;
        bool on = halves ? (mod*2 >= n) : (mod == 0);
        ofBackground(on ? 255 : 0);
//        ofSetColor(on ? 255 : 0);
//        ofDrawRectangle(0, 0, w, h);
//        ofTranslate(w, 0);
//        ofSetColor(on ? 0 : 255);
//        ofDrawRectangle(0, 0, w, h);
        ofSetColor(0);
        ofDrawBitmapString(ofToString((int) ofGetFrameRate()) + " " + ofToString(n), 10, 20);
    }
    void keyPressed(int key) {
        if(key == 'f') {
            ofToggleFullscreen();
//            fullscreen = !fullscreen;
//            if(fullscreen) {
//                ofSetWindowPosition(1200, 0);
//                ofSetWindowShape(w * 2, h);
//            } else {
//                ofSetWindowPosition(0, 0);
//                ofSetWindowShape(w * 2, h);
//            }
        }
        if(key == '\t') {
            halves = !halves;
        }
        if(key == OF_KEY_UP) {
            n++;
        }
        if(key == OF_KEY_DOWN) {
            n--;
        }
        if(key == OF_KEY_RIGHT) {
            phase++;
        }
        if(key == OF_KEY_LEFT) {
            phase++;
        }
        n = ofClamp(n, 2, 120);
        phase = ofClamp(phase, -MAX_PHASE_OFFSET, +MAX_PHASE_OFFSET);
    }
};
int main() {
//    ofAppGlutWindow window;
//    ofSetupOpenGL(&window, 1280, 720, OF_FULLSCREEN);
    ofSetupOpenGL(1280, 720, OF_FULLSCREEN);
    ofRunApp(new ofApp());
}