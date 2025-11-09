/**
 * @file obstacle_detection_iface.cpp
 * @brief Implements the @ref IObstacleDetection façade.
 */

#include "obstacle_detection_iface.hpp"
#include "obstacle_detection_module.hpp"

/** See @ref IObstacleDetection::previewDepth. */
void IObstacleDetection::previewDepth(cv::Mat& depthImage) {
    ObstacleDetectionModule::get_instance().previewDepth(depthImage);
}

/** See @ref IObstacleDetection::viewDetection. */
void IObstacleDetection::viewDetection(Obstacle& obstacle) {
    ObstacleDetectionModule::get_instance().viewDetection(obstacle);
}

/** See @ref IObstacleDetection::detect. */
Obstacle IObstacleDetection::detect(cv::Mat& image, cv::Mat& depthMap) {
    return ObstacleDetectionModule::get_instance().detect(image, depthMap);
}

/** See @ref IObstacleDetection::start_detection. */
void IObstacleDetection::start_detection() {
    ObstacleDetectionModule::get_instance().start_detection();
}

/** See @ref IObstacleDetection::stop_detection. */
void IObstacleDetection::stop_detection() {
    ObstacleDetectionModule::get_instance().stop_detection();
}
