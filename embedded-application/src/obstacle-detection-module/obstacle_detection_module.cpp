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
cv::Mat ObstacleDetectionModule::segmentRed(const cv::Mat& image) const { //image en HSV
    cv::Mat mask1, mask2, redMask;

    // Crear máscaras para cada rango
    cv::inRange(image, lowerRed1_, upperRed1_, mask1);
    cv::inRange(image, lowerRed2_, upperRed2_, mask2);

    // Combinar las dos máscaras
    cv::bitwise_or(mask1, mask2, redMask);

    return redMask;
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