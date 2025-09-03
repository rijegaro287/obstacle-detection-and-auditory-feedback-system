#include <iostream>

#include "obstacle_detection_module.h"
#include "image_capture_module.h"

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

        auto [depth, img] = capturemod.preprocessDepth(); // imagen preprocesada y de profundidad
        if (!img.empty()) {
            // Visualizar resultado del preprocesamiento
            detmod.previewDepth(img);
            
            // Iniciar deteccion 
            ObstacleDetectionModule::Obstacle obs;
            obs = detmod.startDetection(img, depth);  
            
            // Visualizar deteccion
            detmod.viewDetection(obs);
        }

        int key = cv::waitKey(1);
        if (key == 27 || key == 'q') break;
    }

    return 0;
}
