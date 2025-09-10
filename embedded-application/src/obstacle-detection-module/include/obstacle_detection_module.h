#pragma once

#ifndef OBSTACLE_DETECTION_MODULE_HPP
#define OBSTACLE_DETECTION_MODULE_HPP

#include <opencv2/opencv.hpp>
#include "obstacle.hpp"

class ObstacleDetectionModule {
public:
    ObstacleDetectionModule();

    void previewDepth(cv::Mat& depthImage);
    void viewDetection(Obstacle& obstacle);
    Obstacle detect(cv::Mat& image, cv::Mat& depthMap);
    void start();

private:
    struct Components {
        cv::Mat labels;     // labels de los componentes
        cv::Mat stats;      // stats
        cv::Mat centroids;  // centroides
        cv::Mat image;      // imagen mascara coloreada por componentes
    };
    
    // Rangos HSV de colores para deteccion 
    cv::Scalar lowerRed1_;
    cv::Scalar upperRed1_;
    cv::Scalar lowerRed2_;
    cv::Scalar upperRed2_;
    cv::Scalar lowerOrange_;
    cv::Scalar upperOrange_;

    // Valores de FOV de la camara (field-of-view)
    static constexpr double FOV_X_DEG = 62.8; // Horizontal
    static constexpr double FOV_Y_DEG = 37.9; // Vertical

    cv::Mat segmentRed(const cv::Mat& image) const;
    cv::Mat filterByDepth(const cv::Mat& mask, const cv::Mat& depthImage, float maxDepthThreshold = 2000.0f) const; // AJUSTAR!!!
    cv::Mat filterByColorDensity(const cv::Mat& mask, double minArea = 700.0, double minDensity = 0.95) const;
    Components divideComponents(const cv::Mat& mask) const;
    Obstacle selectObstacle(Components& components, const cv::Mat& depthMap) const;
    Obstacle calculateAngles(Obstacle& obstacle);
    double mapAzimuth(double azimuth);

};

#endif // OBSTACLE_DETECTION_MODULE_HPP
