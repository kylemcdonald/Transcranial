#include "ofAppGLFWWindow.h"
#include "ofxCv.h"
#include "ofMain.h"
#include "ofxEdsdkCam.h"
#include "ofxUI.h"
#include "ofxOsc.h"
#include "ofxTiming.h"
#include "FrameDifference.h"

//#define USE_VIDEO
#define USE_EDSDK

using namespace ofxCv;
using namespace cv;

void drawIndicator() {
    if(ofGetKeyPressed('`')) {
        ofPushMatrix();
        ofPushStyle();
        ofFill();
        ofSetColor(255, 0, 0);
        ofDrawCircle(ofGetWidth() - 10, ofGetHeight() - 18, 10);
        ofPopStyle();
        ofPopMatrix();
    }
}

float smoothestStep(float t) {
    float t2 = t * t;
    return (-20*t2*t+70*t2-84*t+35)*t2*t2;
}

class ofApp : public ofBaseApp {
public:
#ifdef USE_VIDEO
    ofVideoPlayer video;
#else
	ofxEdsdkCam video;
#endif
    
    Mat gray, graySmall, thresholded, dilated;
    ofxCv::ContourFinder contours;
    
    ofFbo buffer;
    ofxUICanvas* gui;
    
    FadeTimer stabilityFade;
    FrameDifference motion;
    RunningBackground motionRunning;
    Mat thresholdedRunning;
    
    ofxOscSender osc;
    
    bool debug = false;
    bool bggray = false;
    float rescale = .25;
    float minAreaRadius = 10;
    float thresholdValue = 55; //72;
    float dilationAmount = 2;
    float verticalOffset = 30;//-8;
    float horizontalOffset = 0;
    float scaling = .91;//1.02;
    float bodyCenterSmoothing = .5;
    
    float tintRed = 255, tintGreen = 225, tintBlue = 255;
    
    float stability = 0.;//.6; //1.
    float spreadAmplitude = .5;
    float repetitionSteps = 1;//10;
    float rotationAmplitude = 180;//20;
    float rotationRate = 10;//1;
    float rotationNoise = .1;//.5;
    float scaleAmplitude = 1;//.1;
    float scaleRate =  10;//1;
    float scaleNoise = .1;//.5;
    float motionMin = .02 / 5; // .01
    float motionMax = .02 / 3; //.005;
    
    const float motionRange = .02;
    const float motionLearningTime = .250;
    float motionValue = 0;
    float smoothedMotionValue = 0;
    float motionSmoothingUp = .99;
    float motionSmoothingDown = .33;
    ofxUISlider* motionSlider;
    ofxUISlider* smoothedMotionSlider;
    
    ofVec2f bodyCenter;
    
    void loadScene1() {
        stabilityFade.start();
//        stability = 0;
        spreadAmplitude = .5;
        repetitionSteps = 1;
        rotationAmplitude = 180;
        rotationRate = 10;
        rotationNoise = .1;
        scaleAmplitude = .87;
        scaleRate = 10;
        scaleNoise = .1;
        motionSmoothingUp = .99;
        motionSmoothingDown = .33;
        motionMin = .006;
        motionMax = .012;
    }
    
    void loadScene2() {
        stabilityFade.start();
//        stability = 0;
        spreadAmplitude = 2;
        repetitionSteps = 1;
        rotationAmplitude = 0;
        rotationRate = 10;
        rotationNoise = .1;
        scaleAmplitude = .87;
        scaleRate = 10;
        scaleNoise = .1;
        motionSmoothingUp = .99;
        motionSmoothingDown = .92;
        motionMin = .006;
        motionMax = .016;
    }
    
    void loadScene3() {
        stabilityFade.start();
//        stability = 0;
        spreadAmplitude = 2;
        repetitionSteps = 40;
        rotationAmplitude = 360;
        rotationRate = 5;
        rotationNoise = .2;
        scaleAmplitude = 2;
        scaleRate = 2.46;
        scaleNoise = .1;
        motionSmoothingUp = .96;
        motionSmoothingDown = .07;
        motionMin = .006;
        motionMax = .024;
    }
    
    
    void setupGui() {
        gui = new ofxUICanvas();
        gui->addLabel("Settings");
        gui->addFPS();
        gui->addToggle("Debug", &debug);
        gui->addToggle("Gray Background", &bggray);
        gui->addSlider("Tint red", 220, 255, &tintRed);
        gui->addSlider("Tint green", 220, 255, &tintGreen);
        gui->addSlider("Tint blue", 220, 255, &tintBlue);
        gui->addSlider("Scaling", .5, 1.5, &scaling);
        gui->addSlider("Rescale", .1, 1, &rescale);
        gui->addSlider("Threshold", 0, 255, &thresholdValue);
        gui->addSlider("Dilation", 0, 6, &dilationAmount);
        gui->addSlider("Vertical offset", -100, 100, &verticalOffset);
        gui->addSlider("Horizontal offset", -300, 300, &horizontalOffset);
        gui->addSlider("Body center smoothing", 0, 1, &bodyCenterSmoothing);
        gui->addSpacer();
        gui->addSlider("Min area radius", 0, 50, &minAreaRadius);
        gui->addSlider("Stability", 0, 1, &stability);
        gui->addSlider("Spread amplitude", 0, 2, &spreadAmplitude);
        gui->addSlider("Repetition steps", 0, 40, &repetitionSteps);
        gui->addSlider("Rotation amplitude", 0, 360, &rotationAmplitude);
        gui->addSlider("Rotation rate", 0, 10, &rotationRate);
        gui->addSlider("Rotation noise", 0, 1, &rotationNoise);
        gui->addSlider("Scale amplitude", 0, 2, &scaleAmplitude);
        gui->addSlider("Scale rate", 0, 10, &scaleRate);
        gui->addSlider("Scale noise", 0, 1, &scaleNoise);
        motionSlider = gui->addSlider("Motion", 0, motionRange, &motionValue);
        smoothedMotionSlider = gui->addSlider("Smoothed motion", 0, motionRange, &smoothedMotionValue);
        gui->addSlider("Motion Smoothing+", .9, 1, &motionSmoothingUp);
        gui->addSlider("Motion Smoothing-", 0, 1, &motionSmoothingDown);
        gui->addSlider("Motion min", 0, motionRange, &motionMin);
        gui->addSlider("Motion max", 0, motionRange, &motionMax);
        gui->autoSizeToFitWidgets();
        keyPressed('\t');
    }
    
    void setup() {
        ofSetDataPathRoot("../../../../../SharedData/");
        ofSetVerticalSync(true);
//        ofSetLogLevel(OF_LOG_VERBOSE);
        
#ifdef USE_VIDEO
        video.loadMovie("videos/melica.mp4");
        video.play();
#else
        video.setup();
        #ifdef USE_EDSDK
            video.setDeviceType(EDSDK_MKII);
        #endif
#endif
        stabilityFade.setLength(30, .5);
        
        ofFbo::Settings settings;
        settings.width = video.getWidth();
        settings.height = video.getHeight();
        settings.useDepth = false;
        buffer.allocate(settings);
        ofSetBackgroundAuto(false);
        contours.getTracker().setPersistence(100);
        contours.getTracker().setMaximumDistance(100);
        setupGui();
        
        osc.setup("klaus.local", 7400);
    }
    
    void exit() {
#ifdef USE_EDSDK
        video.close();
#endif
    }
    
    void update() {
        video.update();
        if(video.isFrameNew()) {
            copyGray(video, gray);
            resize(video, graySmall, rescale, rescale);
            threshold(gray, thresholded, thresholdValue);
            dilate(thresholded, dilated, dilationAmount);
            updateMotion();
            updateContours();
        }
    }
    
    void updateMotion() {
        // get overall motion
//        motion.update(graySmall);
//        motionValue = motion.getMean();
        
        motionRunning.setLearningTime(motionLearningTime);
        motionRunning.update(graySmall, thresholdedRunning);
        motionValue = motionRunning.getPresence();
        
        ofxOscMessage msg;
        msg.setAddress("/motion");
        float t = ofGetElapsedTimef();
        msg.addFloatArg(motionValue);
        osc.sendMessage(msg);
        if(motionValue > smoothedMotionValue) {
            smoothedMotionValue = ofLerp(motionValue, smoothedMotionValue, motionSmoothingUp);
        } else {
            smoothedMotionValue = ofLerp(motionValue, smoothedMotionValue, motionSmoothingDown);
        }
    }
    
    void updateContours() {
        contours.setMinAreaRadius(minAreaRadius);
        contours.setSortBySize(true);
        contours.findContours(dilated);
        int n = contours.size();
        if(n > 0) {
            cv::Rect all = contours.getBoundingRect(0);
            for(int i = 1; i < n; i++) {
                all |= contours.getBoundingRect(i);
            }
            bodyCenter.interpolate(toOf(all).getCenter(), bodyCenterSmoothing);
        }
    }
    
    void draw() {
        if(!ofGetKeyPressed('b')) {
            ofBackground(bggray ? 128 : 0);
        }
        if(ofGetKeyPressed('l')) {
            ofEnableBlendMode(OF_BLENDMODE_SCREEN);
        } else {
            ofEnableAlphaBlending();
        }
        
        ofPushMatrix();
        ofPushStyle();
        
        ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
        float scaleFactor = ofGetHeight() / (float) MAX(1, video.getHeight());
        ofScale(scaleFactor, scaleFactor);
        ofScale(scaling, scaling);
        ofTranslate(-video.getWidth() / 2, -video.getHeight() / 2);
        ofTranslate(horizontalOffset, verticalOffset);
        
        stability = smoothestStep(stabilityFade.get());
        float totalStability = ofClamp(ofMap(smoothedMotionValue, motionMin, motionMax, 0, stability), 0, 1);
        
        int n = contours.size();
        
        for(int i = 0; i < n; i++) {
            cv::Rect cur = contours.getBoundingRect(i);
            float w = cur.width, h = cur.height;
            float sx = cur.x, sy = cur.y;
            
            buffer.begin();
            ofDisableBlendMode();
            
            // clear buffer area
            ofClear(0, 0);
            
            // draw filled shape (could blur here)
            ofPushMatrix();
            ofSetColor(255);
            ofFill();
            ofBeginShape();
            vector<cv::Point>& vertices = contours.getContour(i);
            for(int j = 0; j < vertices.size(); j++) {
                ofVertex(vertices[j].x, vertices[j].y);
            }
            ofEndShape();
            ofPopMatrix();
            
            // draw body image
            ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
            ofSetColor(255);
            video.getTexture().drawSubsection(sx, sy, w, h, sx, sy);
            buffer.end();
            
            ofEnableBlendMode(OF_BLENDMODE_ALPHA);
            ofPushMatrix();
            
            ofVec2f center = toOf(contours.getCenter(i));
            ofVec2f offset = center - bodyCenter;
            float orientation = atan2f(offset.y, offset.x);
            float spread = totalStability * spreadAmplitude;
            ofVec2f position = bodyCenter + offset + ofVec2f(offset.x, 0) * spread;
            
            float id = orientation; //contours.getLabel(i) % 3;
            
            float baseRotation = rotationRate * ofGetElapsedTimef() + id;
            float rotation = ofLerp(sin(baseRotation), ofSignedNoise(baseRotation), rotationNoise);
            rotation *= rotationAmplitude * totalStability;
            
            float baseScale = scaleRate * ofGetElapsedTimef() + id;
            float scale = 1 + scaleAmplitude * ofLerp(sin(baseScale), ofSignedNoise(baseScale), scaleNoise) * totalStability;
            
            ofPushStyle();
            ofSetColor(tintRed, tintGreen, tintBlue);
            ofTranslate(position);
            for(int j = 0; j < repetitionSteps; j++) {
                ofPushMatrix();
                float rotationAmount = ofMap(j, -1, repetitionSteps, 0, rotation);
                ofRotate(rotationAmount);
//                ofVec3f axis(0, 0, 1);
//                ofRotate(rotationAmount, axis.x, axis.y, axis.z);
                float curScale = ofMap(j, -1, repetitionSteps, 1, scale);
                ofScale(curScale, curScale, curScale);
                buffer.getTexture().drawSubsection(-w / 2, -h / 2, 0, w, h, sx, sy);
                ofPopMatrix();
            }
            ofPopStyle();
            
            if(debug) {
                ofDrawBitmapStringHighlight(ofToString(contours.getLabel(i)), 0, 0);
            }
            ofPopMatrix();
        }
        
        if(debug) {
            ofNoFill();
            ofSetColor(255);
            ofRectangle(0, 0, video.getWidth(), video.getHeight());
        }

        ofPopStyle();
        ofPopMatrix();
        
        ofEnableAlphaBlending();
        
        if(debug) {
            ofPushStyle();
            ofSetColor(255);
            ofNoFill();
            ofSetLineWidth(2);
            ofDrawRectangle(0, 0, video.getWidth(), video.getHeight());
            video.draw(0, 0);
            ofPopStyle();
            
            ofPushStyle();
            ofEnableBlendMode(OF_BLENDMODE_ADD);
            
            ofPushMatrix();
            ofScale(1 / rescale, 1 / rescale);
            drawMat(thresholdedRunning, 0, 0);
            ofPopMatrix();
            
            ofSetColor(magentaPrint, 10);
            drawMat(thresholded, 0, 0);
            
            ofSetLineWidth(3);
            for(int i = 0; i < n; i++) {
                ofSetColor(255);
                contours.getPolyline(i).draw();
            }
            ofNoFill();
            ofSetColor(cyanPrint);
            ofDrawCircle(bodyCenter, 10);
            ofPopStyle();
            
#ifndef USE_VIDEO
            if(video.isLiveDataReady()) {
                stringstream status;
                status << video.getWidth() << "x" << video.getHeight() << " @ " <<
                (int) ofGetFrameRate() << " app-fps " << " / " <<
                (int) video.getFrameRate() << " cam-fps";
                ofDrawBitmapString(status.str(), 10, ofGetHeight() - 40);
            }
#endif
        }
        
        drawIndicator();
    }
    
    int startX, startY;
    float startStability;
    void mousePressed(int x, int y, int button) {
        startX = x, startY = y;
        startStability = stability;
    }
    
    void mouseDragged(int x, int y, int button) {
        int diffX = x - startX, diffY = y - startY;
        stability = startStability + diffX / 500.;
        stability = ofClamp(stability, 0, 1);
    }
    
    void keyPressed(int key) {
        if(key == 'f') {
            ofToggleFullscreen();
        }
        if(key == 'g') {
            ofSetFullscreen(false);
            ofSetWindowPosition(ofGetScreenWidth() + 100, 100);
            ofSetFullscreen(true);
        }
        if(key == '\t') {
            gui->toggleVisible();
            if(gui->isVisible()) {
                debug = true;
                ofShowCursor();
            } else {
                debug = false;
                ofHideCursor();
            }
        }
        if(key == OF_KEY_BACKSPACE) {
            stabilityFade.stop();
        }
        switch(key) {
            case '1': loadScene1(); break;
            case '2': loadScene2(); break;
            case '3': loadScene3(); break;
        }
    }
};

int main() {
    ofSetupOpenGL(1920, 1080, OF_FULLSCREEN);
	ofRunApp(new ofApp());
}
