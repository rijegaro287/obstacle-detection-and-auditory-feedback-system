#include <iostream>
#include "image_capture_module.h"
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

// Metodo filterByColorDensity: descarta las areas muy pequeños o con poca densidad de color (control FP)
// mask: máscara binaria
// minArea: tamaño minimo del area de un obstaculo
// minDensity: densidad minima de color en el area de un obstaculo 

cv::Mat ObstacleDetectionModule::filterByColorDensity(const cv::Mat& mask, double minArea, double minDensity) const {
    // Extracción de contornos de los obstáculos en la máscara para analizar la densidad por región
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    cv::Mat solidMask = cv::Mat::zeros(mask.size(), CV_8UC1); //inicializacion del resultado

    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        if (area < minArea) {
            continue; // Ignorar objetos muy pequeños
        }

        // extraer la region para analisis de densidad
        cv::Mat contourMask = cv::Mat::zeros(mask.size(), CV_8UC1);
        cv::drawContours(contourMask, std::vector<std::vector<cv::Point>>{contour}, -1, cv::Scalar(255), cv::FILLED);

        double colorPixels = cv::countNonZero(mask & contourMask); // separar area de interes de la mascara, deja solo los pixeles de interes (AND)
        double totalPixels = cv::countNonZero(contourMask); // total de pixeles que comprenden la region de interes
        double colorDensity = colorPixels / totalPixels; //densidad de color de la region de interes

        if (colorDensity < minDensity) {
            continue;
        }
        solidMask |= contourMask;
    }

    // Mejorar la segmentación con morfología: reduce el ruido y rellena huecos
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::dilate(solidMask, solidMask, kernel, cv::Point(-1,-1), 1);
    cv::erode(solidMask, solidMask, kernel, cv::Point(-1,-1), 1);

    return solidMask;
}

int main() {
    ImageCaptureModule capturemod;
    ObstacleDetectionModule detmod;

    // Inicializar ToF camera
    if (!capturemod.initialize()) {
        return -1;
    }

    // Capturar frames
    while (true) {
        // si la captura NO fue exitosa vuelve a intentarlo en la siguiente iteracion/captura
        if (!capturemod.captureFrame()) {
            continue;
        }

        auto [depth_og, img] = capturemod.preprocessDepth(); // imagen preprocesada y de profundidad
        if (!img.empty()) {
            cv::Mat img_bgr;
            cv::cvtColor(img, img_bgr, cv::COLOR_HSV2BGR);
            cv::imshow("Preprocessed Depth Preview", img_bgr);
            
            // Aplicar segmentar rojo
            cv::Mat seg = detmod.segmentRed(img);
            cv::Mat seg_bgr; 
            cv::cvtColor(seg, seg_bgr, cv::COLOR_GRAY2BGR);
            //cv::imshow("Segmented red mask", seg_bgr);

            // Aplicar filtrado por profundidad
            cv::Mat depth = detmod.filterByDepth(seg, depth_og);
            cv::Mat depth_bgr; 
            cv::cvtColor(depth, depth_bgr, cv::COLOR_GRAY2BGR);
            cv::imshow("Filtered by depth", depth_bgr);

            cv::Mat solid = detmod.filterByColorDensity(depth);
            cv::Mat solid_bgr; 
            cv::cvtColor(solid, solid_bgr, cv::COLOR_GRAY2BGR);
            cv::imshow("Filtered by density", solid_bgr);

        }

        int key = cv::waitKey(1);
        if (key == 27 || key == 'q') break;
    }

    return 0;
}