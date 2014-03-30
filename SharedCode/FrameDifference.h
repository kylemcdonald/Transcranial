#pragma once

#include "ofxCv.h"
#include "ofMain.h"

class FrameDifference {
private:
    cv::Mat a, b, difference;
    double meanVal, minVal, maxVal;
public:
    FrameDifference()
    :meanVal(0)
    ,minVal(0)
    ,maxVal(0) {
    }
    template <class F>
    void update(F& frame) {
        cv::Mat frameMat = ofxCv::toCv(frame);
        update(frameMat);
    }
    void update(cv::Mat& frame) {
        ofxCv::copyGray(frame, a);
        if(a.size() == b.size()) {
            absdiff(a, b, difference);
            meanVal = cv::mean(difference)[0] / 255.;
            cv::minMaxIdx(difference, &minVal, &maxVal);
            minVal /= 255;
            maxVal /= 255;
        }
        swap(a, b);
    }
    cv::Mat& getDifference() {
        return difference;
    }
    float getMean() {
        return meanVal;
    }
    float getMax() {
        return maxVal;
    }
};
