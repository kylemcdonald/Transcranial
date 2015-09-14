#include "ofMain.h"
#include "ofxGui.h"

#include "FadeTimer.h"

class ofApp : public ofBaseApp {
public:
    FadeTimer fade;
    bool fadeDirection = true;
    ofImage vignette;
    ofParameter<float>
    radius = .5,//.18,
    bottom = 0,//.2,
    shape = 1.,//.75,
    brightness = 1;
    bool hideGui = true;
    ofxPanel gui;
    void setup() {
        ofSetBackgroundColor(0);
        ofSetVerticalSync(true);
        windowResized(ofGetWidth(), ofGetHeight());
        gui.setup();
        gui.add(radius.set("Radius", radius, 0, .5));
        gui.add(bottom.set("Bottom", bottom, 0, .5));
        gui.add(shape.set("Shape", shape, 0, 1));
        gui.add(brightness.set("Brightness", brightness, 0, 1));
        updateVignette();
        fade.setLength(3);
    }
    void update() {
        if(!hideGui) {
            updateVignette();
        }
    }
    void keyPressed(int key) {
        if(key == 'f') {
            ofToggleFullscreen();
        }
        if(key == '\t') {
            hideGui = !hideGui;
        }
        if(key == ' ') {
            if(fadeDirection) {
                fade.start();
                fadeDirection = false;
            } else {
                fade.stop();
                fadeDirection = true;
            }
        }
    }
    void draw() {
        ofPushStyle();
        ofSetColor(255, fade.get() * 255);
        vignette.draw(0, 0);
        ofPopStyle();
        if(!hideGui) {
            gui.draw();
        }
    }
    void windowResized(int w, int h) {
        updateVignette();
    }
    void updateVignette() {
        int w = ofGetWidth();
        int h = ofGetHeight();
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
    ofSetupOpenGL(1024, 768, OF_FULLSCREEN);
    ofRunApp(new ofApp());
}