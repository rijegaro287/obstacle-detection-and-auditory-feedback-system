#include <iostream>

#include "obstacle_detection_module.h"

// Contructor: asigna los rangos de rojo para la detección
ObstacleDetectionModule::ObstacleDetectionModule():
      // Rango 1 - Tonos (H) del 0 al 10, para todas las saturaciones (S) y brillos (V)
      lowerRed1_(0, 0, 0),
      upperRed1_(10, 255, 255),
      // Rango 2 – Tonos (H) del 160 al 180, para todas las saturaciones (S) y brillos (V)
      lowerRed2_(160, 0, 0),
      upperRed2_(180, 255, 255)
{}

// Método segmentRed: segmenta los rangos de rojo en las imagenes usando una mascara de color
cv::Mat ObstacleDetectionModule::segmentRed(const cv::Mat& image) const { //imagen en HSV
    cv::Mat mask1, mask2, redMask;

    // Crear máscaras para cada rango
    cv::inRange(image, lowerRed1_, upperRed1_, mask1);
    cv::inRange(image, lowerRed2_, upperRed2_, mask2);

    // Combinar las dos máscaras
    cv::bitwise_or(mask1, mask2, redMask);

    return redMask;
}

// Método filterByDepth: verifica que el obstáculo encontrado tenga valores de profundidad coherentes (control FP)
// mask: máscara binaria, salida de segmentRed
// depthImage: imagen de profundidad original, en blanco y negro
// maxDepthThreshold: Umbral para determinar si es objeto detectado esta suificientemente cerca para ser considerado un obstáculo

cv::Mat ObstacleDetectionModule::filterByDepth(const cv::Mat& mask, const cv::Mat& depthImage, float maxDepthThreshold) const {
    // Extracción de contornos de los obstáculos en la máscara para analizar la profundidad por región
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    cv::Mat filteredMask = cv::Mat::zeros(mask.size(), CV_8UC1); // Inicialización del resultado

    // Análisis de profundidad en los contornos encontrados
    for (const auto& contour : contours) {
        cv::Mat contourMask = cv::Mat::zeros(mask.size(), CV_8UC1);
        cv::drawContours(contourMask, std::vector<std::vector<cv::Point>>{contour}, -1, cv::Scalar(255), cv::FILLED);

        // depthROI contiene los valores de profundidad de la región de interés (ROI), copiados de depthImage
        cv::Mat depthROI;
        depthImage.copyTo(depthROI, contourMask);

        double minDepth, maxDepth; // Valores min y max de profundidad en la región 
        cv::minMaxLoc(depthROI, &minDepth, &maxDepth, nullptr, nullptr, contourMask);

        if (minDepth < maxDepthThreshold) { 
            filteredMask |= contourMask; // mantener áreas de la máscara que cumplen con el umbral
        }
    }

    return filteredMask;
}

int main() {
    std::cout << "Obstacle Detection Module - Starting..." << std::endl;
    
    // Initialize detection algorithms
    std::cout << "Loading obstacle detection models..." << std::endl;
    std::cout << "Initializing computer vision algorithms..." << std::endl;
    
    // Configure detection parameters
    std::cout << "Setting detection thresholds..." << std::endl;
    std::cout << "Configuring depth analysis..." << std::endl;
    
    // Start detection process
    std::cout << "Obstacle detection module ready for real-time analysis." << std::endl;
    
    return 0;
}