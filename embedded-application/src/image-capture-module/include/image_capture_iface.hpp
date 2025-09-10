#pragma once
#include <opencv2/core.hpp>

class IImageCapture {
public:
    static bool initialize();
    static bool captureFrame();
    static std::pair<cv::Mat, cv::Mat> preprocessDepth();
};
