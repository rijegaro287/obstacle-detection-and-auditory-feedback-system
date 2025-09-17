#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include "image_capture_module.hpp"
#include "image_capture_iface.hpp"

int main() {
    // Prueba con la interfaz
    if (!IImageCapture::initialize()) {
        std::cerr << "Error al inicializar la cámara" << std::endl;
        return 1;
    }
    while(true){
        if (!IImageCapture::captureFrame()) {
            std::cerr << "Frame no disponible" << std::endl;
            continue;
        }

        auto [depth, img] = IImageCapture::preprocessDepth();
        if (!img.empty()) {
            cv::imshow("Preprocessed Depth Preview", img);
        }

        int key = cv::waitKey(1);
        if (key == 27 || key == 'q') break;
    }


     // Prueba con el modulo 
    /**ImageCaptureModule capturemod;
    capturemod.start();**/

    return 0;
}

