#include "ofMain.h"

class ofApp : public ofBaseApp {
public:
    ofShader blend;
    ofFbo bg, fg, blended;
    ofVideoPlayer video;
    
	void setup() {
        ofSetDataPathRoot("../../../../../SharedData/");
        blend.load("shaders/Lighten");
        video.loadMovie("videos/milos-extreme.mov");
        video.play();
	}
	void update() {
        video.update();
	}
	void draw() {
        blend.begin();
        blend.setUniformTexture("a", video, 0);
        blend.setUniformTexture("b", video, 1);
        blend.setUniform2f("resolution", video.getWidth(), video.getHeight());
        blend.setUniform2f("offset", mouseX, 0);
        video.draw(0, 0);
        blend.end();
	}
};

int main() {
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}