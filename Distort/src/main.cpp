#include "ofAppGLFWWindow.h"
#include "ofxCv.h"
#include "ofMain.h"
#include "ofxEdsdk.h"
#include "ofxUI.h"

using namespace ofxCv;
using namespace cv;

class ofApp : public ofBaseApp {
public:
    //	ofxEdsdk::Camera camera;
    
    ofVideoPlayer video;
    ofxUICanvas* gui;
    
    ofPixels resized;
    
	ofxCv::FlowFarneback flow;
	ofMesh mesh;
	int stepSize, xSteps, ySteps;
    
    bool debug = false;
    
    void setupGui() {
        gui = new ofxUICanvas();
        gui->addLabel("Settings");
        gui->addSpacer();
        gui->addFPS();
        gui->addToggle("Debug", &debug);
        gui->addSpacer();
        gui->autoSizeToFitWidgets();
    }
    
    void setup() {
        ofSetDataPathRoot("../../../../../SharedData/");
        ofSetVerticalSync(true);
        ofSetLogLevel(OF_LOG_VERBOSE);
        video.loadMovie("MVI_0248.MOV");
        video.play();
        
        setupGui();
    
        mesh.setMode(OF_PRIMITIVE_TRIANGLES);
        stepSize = 32;
        ySteps = video.getHeight() / stepSize;
        xSteps = video.getWidth() / stepSize;
        for(int y = 0; y < ySteps; y++) {
            for(int x = 0; x < xSteps; x++) {
                mesh.addVertex(ofVec2f(x * stepSize, y * stepSize));
                mesh.addTexCoord(ofVec2f(x * stepSize, y * stepSize));
            }
        }
        for(int y = 0; y + 1 < ySteps; y++) {
            for(int x = 0; x + 1 < xSteps; x++) {
                int nw = y * xSteps + x;
                int ne = nw + 1;
                int sw = nw + xSteps;
                int se = sw + 1;
                mesh.addIndex(nw);
                mesh.addIndex(ne);
                mesh.addIndex(se);
                mesh.addIndex(nw);
                mesh.addIndex(se);
                mesh.addIndex(sw);
            }
        }
    }
    
    void update() {
        video.update();
        if(video.isFrameNew()) {
            flow.setWindowSize(16);
            float rescale = .1;
            resize(video, resized, rescale, rescale);
            flow.calcOpticalFlow(resized);
            int i = 0;
            float distortionStrength = 6;
            for(int y = 1; y + 1 < ySteps; y++) {
                for(int x = 1; x + 1 < xSteps; x++) {
                    int i = y * xSteps + x;
                    ofVec2f position(x * stepSize * rescale, y * stepSize * rescale);
                    ofRectangle area(position - rescale * ofVec2f(stepSize, stepSize) / 2, stepSize * rescale, stepSize * rescale);
                    ofVec2f offset = flow.getAverageFlowInRegion(area);
                    mesh.setVertex(i, position / rescale + distortionStrength * offset / rescale);
                    i++;
                }
            }
        }
    }
    
    void draw() {
        ofBackground(0);
        video.getTextureReference().bind();
        mesh.draw();
        video.getTextureReference().unbind();
        if(debug) {
            mesh.drawWireframe();
        }
    }
    
    void keyPressed(int key) {
        if(key == 'f') {
            ofToggleFullscreen();
        }
    }
};

int main() {
	ofAppGLFWWindow window;
    ofSetupOpenGL(&window, 1920, 1200, OF_FULLSCREEN);
    //	ofSetupOpenGL(&window, 1024, 680, OF_WINDOW); // mkii
    //	ofSetupOpenGL(&window, 1056, 704, OF_WINDOW); // t2i
	ofRunApp(new ofApp());
}
