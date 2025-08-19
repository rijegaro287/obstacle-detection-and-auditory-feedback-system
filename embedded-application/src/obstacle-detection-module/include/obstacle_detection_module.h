#pragma once

#ifndef OBSTACLE_DETECTION_MODULE_HPP
#define OBSTACLE_DETECTION_MODULE_HPP

#include <opencv2/opencv.hpp>

class ObstacleDetectionModule {
public:
    ObstacleDetectionModule();

    struct Components {
        cv::Mat labels;     // labels de los componentes
        cv::Mat stats;      // stats
        cv::Mat centroids;  // centroides
        cv::Mat image;      // imagen mascara coloreada por componentes
    };

    struct Obstacle {
        int id;             // etiqueta del componente
        double area;        // área en píxeles
        double meanDepth;   // promedio de profundidad
        double score;       // criterio de selección
        cv::Point centroid; // centroide
        cv::Mat image;      // imagen del obstaculo
    };

    cv::Mat segmentRed(const cv::Mat& image) const;
    cv::Mat filterByDepth(const cv::Mat& mask, const cv::Mat& depthImage, float maxDepthThreshold = 2000.0f) const; // AJUSTAR!!!
    cv::Mat filterByColorDensity(const cv::Mat& mask, double minArea = 700.0, double minDensity = 0.95) const;
    Components divideComponents(const cv::Mat& mask) const;
    Obstacle selectObstacle(const cv::Mat& labels, const cv::Mat& stats, const cv::Mat& centroids, const cv::Mat& depthMap, double areaWeight = 0.01) const;

private:
    // Rangos HSV para rojo (dos rangos para cubrir todo el rojo)
    cv::Scalar lowerRed1_;
    cv::Scalar upperRed1_;
    cv::Scalar lowerRed2_;
    cv::Scalar upperRed2_;
    cv::Scalar lowerOrange_;
    cv::Scalar upperOrange_;

    //cv::Mat filterByColorDensity(const cv::Mat& mask, double minArea = 700.0, double minDensity = 0.85) const;
    //cv::Mat filterByDepth(const cv::Mat& mask, const cv::Mat& depthImage, float maxDepthThreshold = 1000.0f) const; // AJUSTAR!!!

};

#endif // OBSTACLE_DETECTION_MODULE_HPP
