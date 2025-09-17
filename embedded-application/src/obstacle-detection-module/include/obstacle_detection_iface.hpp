#pragma once

#include <opencv2/opencv.hpp>
#include "control_iface.hpp"

class IObstacleDetection {
public:
    static void previewDepth(cv::Mat& depthImage);
    static void viewDetection(Obstacle& obstacle);
    static Obstacle detect(cv::Mat& image, cv::Mat& depthMap);
    static void start_detection();
    static void stop_detection();
};
