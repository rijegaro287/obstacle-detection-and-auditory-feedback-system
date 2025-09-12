#pragma once

#ifndef IOBSTACLE_DETECTION_HPP
#define IOBSTACLE_DETECTION_HPP

#include <opencv2/opencv.hpp>
#include "control_iface.hpp"

class IObstacleDetection {
public:
    static void previewDepth(cv::Mat& depthImage);
    static void viewDetection(Obstacle& obstacle);
    static Obstacle detect(cv::Mat& image, cv::Mat& depthMap);
};

#endif // IOBSTACLE_DETECTION_HPP
