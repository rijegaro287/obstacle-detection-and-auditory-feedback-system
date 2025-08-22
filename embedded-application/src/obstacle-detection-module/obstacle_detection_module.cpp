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
      upperRed2_(180, 255, 255),
      // Rango 3 - Naranja
      lowerOrange_(11, 0, 0),
      upperOrange_(13, 255, 255)
{}

// Método segmentRed: segmenta los rangos de rojo en las imagenes usando una mascara de color
cv::Mat ObstacleDetectionModule::segmentRed(const cv::Mat& image) const { //imagen en HSV
    cv::Mat mask1, mask2, mask3, redOrangeMask;

    // Crear máscaras para cada rango
    cv::inRange(image, lowerRed1_, upperRed1_, mask1);
    cv::inRange(image, lowerRed2_, upperRed2_, mask2);

    // Naranja
    cv::inRange(image, lowerOrange_, upperOrange_, mask3);

    // Combinar todo
    cv::bitwise_or(mask1, mask2, redOrangeMask);
    cv::bitwise_or(redOrangeMask, mask3, redOrangeMask);

    return redOrangeMask;
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

// Método divideComponents: Separar y visualizar componentes conectados
ObstacleDetectionModule::Components ObstacleDetectionModule::divideComponents(const cv::Mat& mask) const {
    Components comp;

    if (mask.empty()) {
        std::cerr << "[ERROR] La máscara está vacía" << std::endl;
        return comp;
    }

    // Aplicar morfología para separar componentes cercanos
    cv::Mat maskProcessed;
    int kernelSize = 3; // Ajusta según qué tan cerca estén los objetos
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(kernelSize, kernelSize));
    cv::morphologyEx(mask, maskProcessed, cv::MORPH_OPEN, kernel);

    // Detectar componentes conectados
    int numComponents = cv::connectedComponentsWithStats(maskProcessed, comp.labels, comp.stats, comp.centroids, 4); // 4-connectivity más selectiva

    // Crear imagen de salida en color
    comp.image = cv::Mat(mask.size(), CV_8UC3, cv::Scalar(0, 0, 0));

    // Colores aleatorios para cada componente
    cv::RNG rng(12345);
    std::vector<cv::Vec3b> colors(numComponents);
    colors[0] = cv::Vec3b(0, 0, 0); // Fondo negro
    for (int i = 1; i < numComponents; i++) {
        colors[i] = cv::Vec3b(rng.uniform(0, 255),
                              rng.uniform(0, 255),
                              rng.uniform(0, 255));
    }

    // Asignar colores según etiqueta
    for (int y = 0; y < comp.labels.rows; y++) {
        for (int x = 0; x < comp.labels.cols; x++) {
            int label = comp.labels.at<int>(y, x);
            comp.image.at<cv::Vec3b>(y, x) = colors[label];
        }
    }

    return comp;
}


// Metodo selectObstacle: selecciona el obstaculo mas importante bajo un criterio matematico 
ObstacleDetectionModule::Obstacle ObstacleDetectionModule::selectObstacle(
        const cv::Mat& labels,
        const cv::Mat& stats,
        const cv::Mat& centroids,
        const cv::Mat& depthMap,
        double areaWeight) const 
{

    double distMax = std::sqrt(labels.cols*labels.cols/4.0 + labels.rows*labels.rows/4.0); //maximo valor de diatancia del centro
    double depthMin = 0.2;  // mínimo valor esperado del sensor en metros
    double depthMax = 5.0;  // máximo valor esperado del sensor en metros
    int areaMax = 20000;

    Obstacle mainObstacle{0, 0, 0, -1.0, cv::Point(-1, -1)};
    int numComponents = stats.rows;

    cv::Point imageCenter(labels.cols / 2, labels.rows / 2);
    double alpha = 5000.0; // peso para cercanía al centro
    double beta  = 2000.0; // peso para cercanía en profundidad

    for (int i = 1; i < numComponents; i++) { // 0 = fondo
        int area = stats.at<int>(i, cv::CC_STAT_AREA);
        if (area <= 0) continue;

        // bounding box
        int x = stats.at<int>(i, cv::CC_STAT_LEFT);
        int y = stats.at<int>(i, cv::CC_STAT_TOP);
        int w = stats.at<int>(i, cv::CC_STAT_WIDTH);
        int h = stats.at<int>(i, cv::CC_STAT_HEIGHT);

        // promedio de profundidad
        double sumDepth = 0.0;
        int count = 0;
        for (int yy = y; yy < y + h; yy++) {
            for (int xx = x; xx < x + w; xx++) {
                if (labels.at<int>(yy, xx) == i) {
                    float d = depthMap.at<float>(yy, xx);
                    if (d > 0) {
                        sumDepth += d;
                        count++;
                    }
                }
            }
        }
        if (count == 0) continue;
        
        double meanDepth = (sumDepth / count)/ 1000.0;
        
        // centroide
        cv::Point centroid(
            static_cast<int>(centroids.at<double>(i,0)),
            static_cast<int>(centroids.at<double>(i,1))
        );

        // distancia al centro
        double distToCenter = cv::norm(centroid - imageCenter);

        // Normalización 0..1
        double normArea   = std::min(1.0, area / static_cast<double>(areaMax));
        double normDist   = 1.0 - std::min(1.0, distToCenter / distMax);
        double normDepth  = 1.0 - std::min(1.0, (meanDepth - depthMin) / (depthMax - depthMin));

        // calcular score
        double score = 0.3*normArea + 0.3*normDist + 0.4*normDepth;

        if (score > mainObstacle.score) {
            mainObstacle.label = i;
            mainObstacle.area = area;
            mainObstacle.meanDepth = meanDepth;
            mainObstacle.score = score;
            mainObstacle.centroid = cv::Point(
                static_cast<int>(centroids.at<double>(i,0)),
                static_cast<int>(centroids.at<double>(i,1))
            );
        }
    }
    
    // dibujar mascara del obstaculo seleccionado como main
    if (mainObstacle.label > 0) {  // si se selecciona un obstaculo valido
        mainObstacle.image = cv::Mat::zeros(labels.size(), CV_8UC1); // mascara vacia
        for (int y = 0; y < labels.rows; y++) {
            for (int x = 0; x < labels.cols; x++) {
                if (labels.at<int>(y,x) == mainObstacle.label) {
                    mainObstacle.image.at<uchar>(y,x) = 255; // pixeles del obstaculo
                }
            }
        }
    }

    return mainObstacle;
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
            //cv::imshow("Filtered by depth", depth_bgr);

            cv::Mat solid = detmod.filterByColorDensity(depth);
            cv::Mat solid_bgr; 
            cv::cvtColor(solid, solid_bgr, cv::COLOR_GRAY2BGR);
            //cv::imshow("Filtered by density", solid_bgr);

            ObstacleDetectionModule::Components components = detmod.divideComponents(solid);
            cv::imshow("Componentes Detectados", components.image);
            //cv::waitKey(0);
            
            ObstacleDetectionModule::Obstacle obs = detmod.selectObstacle(components.labels, components.stats, components.centroids, depth_og);
            cv::Mat colorObs;
            cv::cvtColor(obs.image, colorObs, cv::COLOR_GRAY2BGR);

            // Dibujar la profundidad promedio 
            cv::putText(
                colorObs,
                std::to_string(obs.meanDepth) + " m", // texto
                cv::Point(10, 30),                    // posición
                cv::FONT_HERSHEY_SIMPLEX,             // fuente
                0.8,                                  // escala
                cv::Scalar(255, 0, 255),                // color (verde)
                2                                     // grosor
            );

            cv::imshow("Obstaculo seleccionado", colorObs);
            

        }

        int key = cv::waitKey(1);
        if (key == 27 || key == 'q') break;
    }

    return 0;
}
