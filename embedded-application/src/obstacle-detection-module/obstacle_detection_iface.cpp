#include "obstacle_detection_iface.hpp"
#include "obstacle_detection_module.hpp"

// Vista previa de la imagen de profundidad
void IObstacleDetection::previewDepth(cv::Mat& depthImage) {
    ObstacleDetectionModule::get_instance().previewDepth(depthImage);
}

// Mostrar un obstáculo detectado
void IObstacleDetection::viewDetection(Obstacle& obstacle) {
    ObstacleDetectionModule::get_instance().viewDetection(obstacle);
}

// Ejecutar la detección y devolver un obstáculo
Obstacle IObstacleDetection::detect(cv::Mat& image, cv::Mat& depthMap) {
    return ObstacleDetectionModule::get_instance().detect(image, depthMap);
}

void IObstacleDetection::start_detection() {
    ObstacleDetectionModule::get_instance().start_detection();
}

void IObstacleDetection::stop_detection() {
    ObstacleDetectionModule::get_instance().stop_detection();
}
