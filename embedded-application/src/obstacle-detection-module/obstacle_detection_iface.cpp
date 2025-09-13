#include "obstacle_detection_iface.hpp"
#include "obstacle_detection_module.h"

// Vista previa de la imagen de profundidad
void IObstacleDetection::previewDepth(cv::Mat& depthImage) {
    ObstacleDetectionModule detector;
    detector.previewDepth(depthImage);
}

// Mostrar un obstáculo detectado
void IObstacleDetection::viewDetection(Obstacle& obstacle) {
    ObstacleDetectionModule detector;
    detector.viewDetection(obstacle);
}

// Ejecutar la detección y devolver un obstáculo
Obstacle IObstacleDetection::detect(cv::Mat& image, cv::Mat& depthMap) {
    ObstacleDetectionModule detector;
    return detector.detect(image, depthMap);
}
