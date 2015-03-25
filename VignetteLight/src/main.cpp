#include "ofMain.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp {
public:
    ofImage vignette;
    ofParameter<float>
        radius = .5,
        bottom = .5,
        shape = .5,
        brightness = 1;
    bool hideGui = false;
    ofxPanel gui;
    void setup() {
        ofSetBackgroundAuto(false);
//        ofSetFrameRate(10);
        ofSetVerticalSync(true);
        windowResized(ofGetWidth(), ofGetHeight());
        gui.setup();
        gui.add(radius.set("Radius", radius, 0, .5));
        gui.add(bottom.set("Bottom", bottom, 0, .5));
        gui.add(shape.set("Shape", shape, 0, 1));
        gui.add(brightness.set("Brightness", brightness, 0, 1));
    }
    void update() {
        updateVignette(ofGetWidth(), ofGetHeight());
    }
    void keyPressed(int key) {
        if(key == 'f') {
            ofToggleFullscreen();
        }
        if(key == '\t') {
            hideGui = !hideGui;
        }
    }
    void draw() {
        vignette.draw(0, 0);
        if(!hideGui) {
            gui.draw();
        }
    }
    void windowResized(int w, int h) {
        updateVignette(w, h);
    }
    void updateVignette(int w, int h) {
        vignette.allocate(w, h, OF_IMAGE_GRAYSCALE);
        float r = radius * MIN(w, h);
        for(int y = 0; y < h; y++) {
            for(int x = 0; x < w; x++) {
                ofVec2f point(x, y);
                ofVec2f safe = point;
                safe.x = ofClamp(x, r, w - r);
                safe.y = ofClamp(y, r, h - r * bottom);
                float b = 1 - (safe.distance(point) / r);
                b = ofClamp(b, 0, 1);
                float smooth = b*b*(3 - 2*b);
                b = ofLerp(b, smooth, shape);
                b += ofRandom(-.01, +.01);
                b = ofClamp(b, 0, 1);
                vignette.setColor(x, y, ofFloatColor(brightness * b));
            }
        }
        vignette.update();
    }
};
int main() {
    ofSetupOpenGL(1280, 720, OF_WINDOW);
    ofRunApp(new ofApp());
}