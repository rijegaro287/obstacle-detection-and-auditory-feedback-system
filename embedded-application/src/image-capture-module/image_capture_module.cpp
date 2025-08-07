#include <iostream>

#include "image_capture_module.h"
#include "image_capture_module.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

using namespace Arducam;

// Constructor: Inicializacion del modulo (frame=nullptr)
ImageCaptureModule::ImageCaptureModule() : frame_(nullptr) {}

// Destructor 
ImageCaptureModule::~ImageCaptureModule() {
    tof_.stop();
    tof_.close();
}

// Metodo initialize: inicializar camara ToF
bool ImageCaptureModule::initialize() {
    if (tof_.open(Connection::CSI, 0)) {
        std::cerr << "Failed to open camera" << std::endl;
        return false;
    }
    if (tof_.start(FrameType::DEPTH_FRAME)) {
        std::cerr << "Failed to start camera" << std::endl;
        return false;
    }
    return true;
}

// Metodo captureFrame: captura imagenes
bool ImageCaptureModule::captureFrame() {
    frame_ = tof_.requestFrame(200); // capturar frame
    if (!frame_) {
        return false;
    }

    FrameFormat format;
    frame_->getFormat(FrameType::DEPTH_FRAME, format);

    float* depth_ptr = (float*)frame_->getData(FrameType::DEPTH_FRAME);
    // verificar que se pudieron obtener los datos de profundidad
    if (!depth_ptr) {
        tof_.releaseFrame(frame_);
        return false;
    }

    depth_frame_ = cv::Mat(format.height, format.width, CV_32F, depth_ptr).clone(); // imagen de profundidad en blanco y negro

    // Ver la imagen de profundidad original
    if (!depth_frame_.empty()) {
        cv::Mat depth_vis;
        depth_frame_.convertTo(depth_vis, CV_8U, 255.0 / 7000); // normalizar para visualización
        cv::imshow("Original Depth Frame", depth_vis); // mostrar imagen original 
    }

    tof_.releaseFrame(frame_); // liberar frame_
    return !depth_frame_.empty(); // vacio = false, no vacio = true (success)
}

// Metodo preprocessDepth: preprocesamiento de la imagen
cv::Mat ImageCaptureModule::preprocessDepth() {
    //verificar si la imagen de profundidad esta vacia 
    if (depth_frame_.empty()){
        std::cerr << "[WARNING] La imagen de profundidad esta vacia, no se puede preprocesar" << std::endl;
        return cv::Mat(); // retorna vacio
    }
    // Normalizacion de la imagen de profundidad (a flotante 0.0 - 1.0)
    cv::Mat depth_normalized;
    cv::normalize(depth_frame_, depth_normalized, 0.0, 1.0, cv::NORM_MINMAX);

    // Conversion a 8 bits para visualización
    cv::Mat depth_8u;
    depth_normalized.convertTo(depth_8u, CV_8U, 255);

    // Mapa de colores (JET)
    cv::applyColorMap(depth_8u, result_frame_, cv::COLORMAP_JET);

    // Convertir de BGR a HSV
    cv::Mat hsv_image;
    cv::cvtColor(result_frame_, hsv_image, cv::COLOR_BGR2HSV);

    // Filtro de mediana al canal de brillo (V) para reducir ruido
    std::vector<cv::Mat> hsv_channels;
    cv::split(hsv_image, hsv_channels);
    cv::medianBlur(hsv_channels[2], hsv_channels[2], 5);  // filtro a canal V

    // Unir resultados
    cv::merge(hsv_channels, hsv_image);
    cv::cvtColor(hsv_image, result_frame_, cv::COLOR_HSV2BGR);

    return result_frame_;
}

int main() {
    ImageCaptureModule capturemod;

    // Inicializar ToF camera
    if (!capturemod.initialize()) {
        return -1;
    }

    cv::namedWindow("Depth Preview", cv::WINDOW_AUTOSIZE);

    // Capturar frames
    while (true) {
        // si la captura NO fue exitosa vuelve a intentarlo en la siguiente iteracion/captura
        if (!capturemod.captureFrame()) {
            continue;
        }

        cv::Mat img = capturemod.preprocessDepth(); // imagen preprocesada
        if (!img.empty()) {
            cv::imshow("Preprocessed Depth Preview", img);
        }

        int key = cv::waitKey(1);
        if (key == 27 || key == 'q') break;
    }

    return 0;
}



