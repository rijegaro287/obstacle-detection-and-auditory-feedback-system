#pragma once

/**
 * @file obstacle_detection_iface.hpp
 * @brief Public façade for the obstacle detection module.
 */

#include <opencv2/opencv.hpp>
#include "control_iface.hpp"

/**
 * @class IObstacleDetection
 * @ingroup obstacle_detection_module
 * @brief Static wrapper exposing @ref ObstacleDetectionModule functionality.
 */
class IObstacleDetection {
public:
    /** See @ref ObstacleDetectionModule::previewDepth. */
    static void previewDepth(cv::Mat& depthImage);

    /** See @ref ObstacleDetectionModule::viewDetection. */
    static void viewDetection(Obstacle& obstacle);

    /** See @ref ObstacleDetectionModule::detect. */
    static Obstacle detect(cv::Mat& image, cv::Mat& depthMap);

    /** See @ref ObstacleDetectionModule::start_detection. */
    static void start_detection();

    /** See @ref ObstacleDetectionModule::stop_detection. */
    static void stop_detection();
};
