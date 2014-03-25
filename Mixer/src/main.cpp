#include "ofAppGLFWWindow.h"
#include "ofxCv.h"
#include "ofMain.h"
#include "ofxEdsdk.h"
#include "ofxUI.h"
#include "ofxOsc.h"

#define USE_VIDEO

using namespace ofxCv;
using namespace cv;

class FrameDifference {
private:
    Mat a, b, difference;
    float mean;
public:
    template <class F>
    void update(F& frame) {
        cv::Mat frameMat = toCv(frame);
        update(frameMat);
    }
    float update(Mat& frame) {
        cv::cvtColor(frame, a, CV_RGB2GRAY);
        absdiff(a, b, difference);
        copy(a, b);
        mean = cv::mean(difference)[0] / 255.;
        return mean;
    }
    Mat& getDifference() {
        return difference;
    }
    float getMean() {
        return mean;
    }
};

class ofApp : public ofBaseApp {
public:
    ofxCv::ContourFinder contours;
#ifdef USE_VIDEO
    ofVideoPlayer video;
#else
	ofxEdsdk::Camera video;
#endif
    ofFbo buffer;
    ofxUICanvas* gui;
    
    FrameDifference motion;
    
    ofxOscSender osc;
    
    bool debug = false;
    bool showVideo = false;
    float minAreaRadius = 10;
    float stability = 1.;//.6; //1.
    float repetitionSteps = 1;//10;
    float threshold = 60;
    float rotationCorrection = 0;//.4;//.66;
    float rectRatio = .2;
    float rotationAmplitude = 180;//20;
    float rotationRate = 10;//1;
    float rotationNoise = .1;//.5;
    float scaleAmplitude = 1;//.1;
    float scaleRate =  10;//1;
    float scaleNoise = .1;//.5;
    float motionMin = .02 / 5; // .01
    float motionMax = .02 / 3; //.005;
    
    float motionValue = 0;
    float smoothedMotionValue = 0;
    float motionSmoothingUp = .99;
    float motionSmoothingDown = 0;
    ofxUISlider* motionSlider;
    ofxUISlider* smoothedMotionSlider;
    
    void setupGui() {
        gui = new ofxUICanvas();
        gui->addLabel("Settings");
        gui->addToggle("Debug", &debug);
        gui->addToggle("Show video", &showVideo);
        gui->addSpacer();
        gui->addSlider("Min area radius", 0, 50, &minAreaRadius);
        gui->addSlider("Stability", 0, 1, &stability);
        gui->addSlider("Repetition steps", 0, 40, &repetitionSteps);
        gui->addSlider("Threshold", 0, 255, &threshold);
        gui->addSlider("Rotation correction", 0, 1, &rotationCorrection);
        gui->addSlider("Rect ratio", 0, 1, &rectRatio);
        gui->addSlider("Rotation amplitude", 0, 360, &rotationAmplitude);
        gui->addSlider("Rotation rate", 0, 10, &rotationRate);
        gui->addSlider("Rotation noise", 0, 1, &rotationNoise);
        gui->addSlider("Scale amplitude", 0, 2, &scaleAmplitude);
        gui->addSlider("Scale rate", 0, 10, &scaleRate);
        gui->addSlider("Scale noise", 0, 1, &scaleNoise);
        motionSlider = gui->addSlider("Motion", 0, .02, &motionValue);
        smoothedMotionSlider = gui->addSlider("Smoothed motion", 0, .02, &smoothedMotionValue);
        gui->addSlider("Motion Smoothing+", .9, 1, &motionSmoothingUp);
        gui->addSlider("Motion Smoothing-", 0, 1, &motionSmoothingDown);
        gui->addSlider("Motion min", 0, .02, &motionMin);
        gui->addSlider("Motion max", 0, .02, &motionMax);
        gui->autoSizeToFitWidgets();
    }
    
    void setup() {
        ofSetDataPathRoot("../../../../../SharedData/");
        ofSetVerticalSync(true);
        ofSetLogLevel(OF_LOG_VERBOSE);
        
#ifdef USE_VIDEO
        video.loadMovie("MVI_0252.MOV");
        video.play();
#else
        video.setup();
#endif
        
        ofFbo::Settings settings;
        settings.width = video.getWidth();
        settings.height = video.getHeight();
        settings.useDepth = false;
        buffer.allocate(settings);
        ofSetBackgroundAuto(false);
        contours.getTracker().setPersistence(100);
        contours.getTracker().setMaximumDistance(100);
        setupGui();
        
        osc.setup("Klaus-Obermaiers-Macbook-Pro-Retina.local", 7400);
    }
    
    void update() {
        video.update();
        if(video.isFrameNew()) {
#ifdef USE_VIDEO
            motion.update(video);
#else
            motion.update(video.getLivePixels());
#endif
            motionValue = motion.getMean();
            motionSlider->setValue(motionValue);
            ofxOscMessage msg;
            msg.setAddress("/motion");
            msg.addFloatArg(motionValue);
            osc.sendMessage(msg);
            
            if(motionValue > smoothedMotionValue) {
                smoothedMotionValue = ofLerp(motionValue, smoothedMotionValue, motionSmoothingUp);
            } else {
                smoothedMotionValue = ofLerp(motionValue, smoothedMotionValue, motionSmoothingDown);
            }
            smoothedMotionSlider->setValue(smoothedMotionValue);
            
            contours.setThreshold(threshold);
            contours.setMinAreaRadius(minAreaRadius);
            contours.setSortBySize(true);
#ifdef USE_VIDEO
            contours.findContours(video);
#else
            contours.findContours(video.getLivePixels());
#endif
        }
    }
    
    void draw() {
        ofBackground(0);
        
        ofPushMatrix();
        ofPushStyle();
        
#ifdef USE_VIDEO
        float scaleFactor = ofGetWidth() / video.getWidth();
        ofScale(scaleFactor, scaleFactor);
#else
        if(video.isLiveReady()) {
            float scaleFactor = ofGetWidth() / (float) video.getWidth();
            ofScale(scaleFactor, scaleFactor);
        }
#endif
        
        float totalStability = ofClamp(ofMap(smoothedMotionValue, motionMin, motionMax, 0, stability), 0, 1);
        
        int n = contours.size();
        
        if(debug) {
            ofPushStyle();
            ofSetLineWidth(3);
            for(int i = 0; i < n; i++) {
                ofSetColor(255);
                contours.getPolyline(i).draw();
                RotatedRect minRect = contours.getMinAreaRect(i);
                float ratio = minRect.size.width / minRect.size.height;
                if(ratio > 1+rectRatio || ratio < 1-rectRatio) {
                    if(minRect.size.width > minRect.size.height) {
                        ofSetColor(magentaPrint);
                    } else {
                        ofSetColor(yellowPrint);
                    }
                }
                toOf(minRect).draw();
            }
            ofPopStyle();
        }
        
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
#ifdef USE_VIDEO
            video.getTextureReference().drawSubsection(sx, sy, w, h, sx, sy);
#else
            video.getLiveTexture().drawSubsection(sx, sy, w, h, sx, sy);
#endif
            buffer.end();
            
            ofEnableBlendMode(OF_BLENDMODE_ALPHA);
            ofPushMatrix();
            ofTranslate(sx + w / 2, sy + h / 2);
            float baseRotation = rotationRate * ofGetElapsedTimef() + (contours.getLabel(i) % 3);
            float rotation = ofLerp(sin(baseRotation), ofSignedNoise(baseRotation), rotationNoise);
            float periodicRotation = rotation * rotationAmplitude;
            float correctedRotation = 0;
            RotatedRect fit = contours.getMinAreaRect(i);
            float ratio = fit.size.width / fit.size.height;
            if(ratio > 1+rectRatio || ratio < 1-rectRatio) {
                if(fit.size.width > fit.size.height) {
                    correctedRotation = -90 - fit.angle;
                } else {
                    correctedRotation = 0 - fit.angle;
                }
            }
            float finalRotation = ofLerp(periodicRotation, correctedRotation, rotationCorrection) * totalStability;
            
            float baseScale = scaleRate * ofGetElapsedTimef() + contours.getLabel(i);
            float scale = 1 + scaleAmplitude * ofLerp(sin(baseScale), ofSignedNoise(baseScale), scaleNoise) * totalStability;
            
            for(int j = 0; j < repetitionSteps; j++) {
                ofPushMatrix();
                ofRotate(ofMap(j, -1, repetitionSteps, 0, finalRotation));
                float curScale = ofMap(j, -1, repetitionSteps, 1, scale);
                ofScale(curScale, curScale, curScale);
                buffer.getTextureReference().drawSubsection(-w / 2, -h / 2, 0, w, h, sx, sy);
                ofPopMatrix();
            }
            
            if(debug) {
                ofDrawBitmapStringHighlight(ofToString(contours.getLabel(i)), 0, 0);
            }
            ofPopMatrix();
        }
            
        ofPopStyle();
        ofPopMatrix();
        
        ofEnableAlphaBlending();
        
#ifndef USE_VIDEO
        if(video.isLiveReady()) {
            stringstream status;
            status << video.getWidth() << "x" << video.getHeight() << " @ " <<
            (int) ofGetFrameRate() << " app-fps " << " / " <<
            (int) video.getFrameRate() << " cam-fps";
            ofDrawBitmapString(status.str(), 10, ofGetHeight() - 40);
        }
#endif
        
        if(showVideo) {
            ofNoFill();
            ofSetColor(255);
            ofSetLineWidth(4);
            video.draw(0, 0);
            ofRectangle(0, 0, video.getWidth(), video.getHeight());
            ofPushMatrix();
            ofScale(.5, .5);
            Mat& diff = motion.getDifference();
            drawMat(diff, 0, 0);
            ofRectangle(0, 0, diff.cols, diff.rows);
            ofPopMatrix();
        }
    }
    
    void keyPressed(int key) {
        if(key == 'f') {
            ofToggleFullscreen();
        }
        if(key == '\t') {
            gui->toggleVisible();
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
