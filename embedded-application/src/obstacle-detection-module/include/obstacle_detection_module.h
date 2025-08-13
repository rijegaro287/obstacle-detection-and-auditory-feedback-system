#pragma once

#ifndef OBSTACLE_DETECTION_MODULE_HPP
#define OBSTACLE_DETECTION_MODULE_HPP

#include <opencv2/opencv.hpp>

class ObstacleDetectionModule {
public:
    ObstacleDetectionModule();

    // Segmenta el color rojo usando los rangos privados
    cv::Mat segmentRed(const cv::Mat& image) const;

private:
    // Rangos HSV para rojo (dos rangos para cubrir todo el rojo)
    cv::Scalar lowerRed1_;
    cv::Scalar upperRed1_;
    cv::Scalar lowerRed2_;
    cv::Scalar upperRed2_;

    //cv::Mat filterByColorDensity(const cv::Mat& mask, double minArea = 700.0, double minDensity = 0.85) const;
    cv::Mat filterByDepth(const cv::Mat& mask, const cv::Mat& depthImage, float maxDepthThreshold = 1000.0f) const; // AJUSTAR!!!

};

#endif // OBSTACLE_DETECTION_MODULE_HPP
