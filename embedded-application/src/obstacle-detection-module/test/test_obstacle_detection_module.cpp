#include <iostream>

#include "obstacle_detection_module.hpp"
#include "image_capture_module.hpp"
#include "obstacle_detection_iface.hpp"

int main() {
    ImageCaptureModule& capturemod = ImageCaptureModule::get_instance();
    ObstacleDetectionModule& detmod = ObstacleDetectionModule::get_instance();

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

        auto [depth, img] = capturemod.preprocessDepth(); // imagen preprocesada y de profundidad
        if (!img.empty()) {
            // Visualizar resultado del preprocesamiento
            //detmod.previewDepth(img);
            IObstacleDetection::previewDepth(img);
            
            // Iniciar deteccion 
            Obstacle obs;
            //obs = detmod.detect(img, depth);  
            obs = IObstacleDetection::detect(img, depth);
            
            // Visualizar deteccion
            //detmod.viewDetection(obs);
            IObstacleDetection::viewDetection(obs);
        }

        int key = cv::waitKey(1);
        if (key == 27 || key == 'q') break;
    }

    return 0;
}
