#include "ofMain.h"

class ofApp : public ofBaseApp {
public:
    ofShader blend;
    ofFbo bg, fg, blended;
    ofVideoPlayer video;
    
	void setup() {
        blend.load("blend");
        ofSetDataPathRoot("../../../../../SharedData/");
        video.loadMovie("milos-extreme.mov");
        video.play();
        bg.allocate(video.getWidth(), video.getHeight());
        fg.allocate(video.getWidth(), video.getHeight());
        blended.allocate(video.getWidth(), video.getHeight());
	}
	void update() {
        video.update();
        if(video.isFrameNew()) {
            bg.begin();
            ofClear(0, 0);
            video.draw(0, 0);
            bg.end();
            
            fg.begin();
            ofClear(0, 0);
            video.draw(mouseX, mouseY);
            fg.end();
            
            blended.begin();
            blend.begin();
            blend.setUniformTexture("bg", bg.getTextureReference(), 0);
            blend.setUniformTexture("fg", fg.getTextureReference(), 1);
            video.draw(0, 0);
            blend.end();
            blended.end();
        }
	}
	void draw() {
        blended.draw(0, 0);
	}
};

int main() {
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}