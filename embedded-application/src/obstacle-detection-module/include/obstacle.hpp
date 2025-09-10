#pragma once
#include <opencv2/opencv.hpp>

struct Obstacle {
    int label;          // etiqueta del componente 
    double area;        // área en píxeles
    double meanDepth;   // promedio de profundidad
    double score;       // criterio de selección
    double azimuth;     // ángulo horizontal
    double elevation;   // ángulo vertical
    cv::Point centroid; // centroide
    cv::Mat image;      // imagen del obstáculo
};
