/**
 * @file image_capture_module.cpp
 * @brief Implements acquisition and pre-processing routines for the depth
 * camera module.
 */

#include <exception>
#include <iostream>

#include "image_capture_module.hpp"

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <thread>
#include <chrono>

using namespace Arducam;

#define MAX_DISTANCE 4000
int max_range = 0;

ImageCaptureModule& ImageCaptureModule::get_instance() {
    static ImageCaptureModule instance;
    return instance;
}

/**
 * @brief Construct the capture module with a dormant camera handle.
 */
ImageCaptureModule::ImageCaptureModule() {
    this->running = false;
    this->camera_initialized = false;
    frame_ = nullptr;
    depth_frame_ = cv::Mat();
    result_frame_ = cv::Mat();
}

/**
 * @brief Ensure the camera is stopped and closed during destruction.
 */
ImageCaptureModule::~ImageCaptureModule() {
    if (!camera_initialized) {
        return;
    }

    try {
        tof_.stop();
    }
    catch (const std::exception& err) {
        std::cerr << "ImageCaptureModule stop failed: " << err.what() << '\n';
        camera_initialized = false;
        return;
    }
    catch (...) {
        std::cerr << "ImageCaptureModule stop failed with unknown error\n";
        camera_initialized = false;
        return;
    }

    try {
        tof_.close();
    }
    catch (const std::exception& err) {
        std::cerr << "ImageCaptureModule close failed: " << err.what() << '\n';
    }
    catch (...) {
        std::cerr << "ImageCaptureModule close failed with unknown error\n";
    }

    camera_initialized = false;
}

// Metodo initialize: inicializar camara ToF
bool ImageCaptureModule::initialize() {
    camera_initialized = false;
    if (tof_.open(Connection::CSI, 0)) {
        std::cerr << "Failed to open camera" << std::endl;
        return false;
    }
    if (tof_.start(FrameType::DEPTH_FRAME)) {
        std::cerr << "Failed to start camera" << std::endl;
        try {
            tof_.close();
        }
        catch (const std::exception& err) {
            std::cerr << "ImageCaptureModule close failed: " << err.what() << '\n';
        }
        catch (...) {
            std::cerr << "ImageCaptureModule close failed with unknown error\n";
        }
        return false;
    }
    tof_.setControl(Control::RANGE, MAX_DISTANCE);
    tof_.getControl(Control::RANGE, &max_range);

    camera_initialized = true;
    return true;
}

// Metodo captureFrame: captura imagenes
bool ImageCaptureModule::captureFrame() {
    if (!camera_initialized) {
        return false;
    }

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

        depth_frame_.convertTo(depth_vis, CV_8U, 255.0 / 4000);

        for(int i = 0; i < result_frame_.rows; i++){
            for(int j = 0; j < result_frame_.cols; j++){
                uint8_t val = result_frame_.at<uint8_t>(i,j);
                if(val < 20){
                    result_frame_.at<uint8_t>(i, j) = 240;
                }
            }
        }

        cv::applyColorMap(depth_vis, result_frame_, cv::COLORMAP_HOT);
        //cv::imshow("Original Depth Frame", result_frame_);
    }

    tof_.releaseFrame(frame_); // liberar frame_
    return !depth_frame_.empty(); // vacio = false, no vacio = true (success)
}

// Metodo preprocessDepth: preprocesamiento de la imagen
Frame ImageCaptureModule::preprocessDepth() {
    if (depth_frame_.empty()) {
        std::cerr << "[WARNING] La imagen de profundidad está vacía, no se puede preprocesar" << std::endl;
        return {cv::Mat(), cv::Mat()};
    }

    // Clonar y limitar valores mayores a MAX_DISTANCE para reducir ruido en zonas lejanas
    cv::Mat depth_clipped = depth_frame_.clone();
    depth_clipped.setTo(MAX_DISTANCE, depth_clipped > MAX_DISTANCE);

    // Normalización usando el máximo fijo
    cv::Mat depth_normalized;
    depth_clipped.convertTo(depth_normalized, CV_32F);
    depth_normalized /= MAX_DISTANCE;   // Escala 0.0 a 1.0
    depth_normalized = cv::min(depth_normalized, 1.0f);

    // Convertir a 8 bits para visualización
    cv::Mat depth_8u;
    depth_normalized.convertTo(depth_8u, CV_8U, 255);

    // Aplicar mapa de colores
    cv::applyColorMap(depth_8u, result_frame_, cv::COLORMAP_HOT);

    // Convertir a HSV para filtrar brillo (canal V)
    cv::Mat hsv_image;
    cv::cvtColor(result_frame_, hsv_image, cv::COLOR_BGR2HSV);

    std::vector<cv::Mat> hsv_channels;
    cv::split(hsv_image, hsv_channels);

    // Extraer canal V
    cv::Mat& v_channel = hsv_channels[2];

    // Si el tipo no es CV_8U, convertirlo
    if (v_channel.type() != CV_8U) {
        v_channel.convertTo(v_channel, CV_8U, 255.0);  // escala si es flotante normalizado
    }

    // Aplicar filtro bilateral
    cv::Mat v_filtered;
    cv::bilateralFilter(v_channel, v_filtered, 9, 75, 75);

     // Combinar resultados
    cv::merge(hsv_channels, hsv_image);
    cv::cvtColor(hsv_image, result_frame_, cv::COLOR_HSV2BGR);

    return {depth_frame_, result_frame_};
}

void ImageCaptureModule::start_capture() {
    this->running = true;
}

void ImageCaptureModule::stop_capture() {
    this->running = false;
}

void ImageCaptureModule::start() {
    // Inicializar ToF camera
    if (!initialize()) {
        return;
    }

    // Capturar frames (loop)
    while (true) {
        if (!this->running) {
            // printf("Image Capture module is paused...\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(PAUSED_SLEEP_MS));
            continue;
        }

        IControl::add_capture_sample_start();

        // si la captura NO fue exitosa vuelve a intentarlo en la siguiente iteracion/captura
        if (!captureFrame()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_SLEEP_MS));
            continue;
        }

        Frame frame = preprocessDepth(); // imagen preprocesada
        if (!frame.image.empty()) {
            IControl::set_frame(frame);
            //cv::imshow("Preprocessed Depth Preview", frame.image);

            IControl::add_capture_sample_end();
        }

        // int key = cv::waitKey(1);
        // if (key == 27 || key == 'q') break;
        
        // std::this_thread::sleep_for(std::chrono::milliseconds(1/TARGET_FPS)); // ajustar frecuencia de captura
        std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_SLEEP_MS));
    }
}
