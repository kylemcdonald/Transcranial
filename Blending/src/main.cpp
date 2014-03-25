#include "ofMain.h"

class ofApp : public ofBaseApp {
public:
    ofShader blend;
    ofVideoPlayer video;
    
	void setup() {
        blend.load("blend");
        ofSetDataPathRoot("../../../../../SharedData/");
        video.loadMovie("milos-extreme.mov");
        video.play();
	}
	void update() {
        video.update();
	}
	void draw() {
        blend.begin();
        blend.setUniformTexture("bg", video.getTextureReference(), 0);
        blend.setUniformTexture("fg", video.getTextureReference(), 1);
        blend.setUniform2f("offset", mouseX, mouseY);
        ofSetTextureWrap(GL_CLAMP, GL_CLAMP);
        video.draw(0, 0);
        blend.end();
	}
};

int main() {
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}