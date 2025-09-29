#pragma once

#include "control_iface.hpp"

#include "ArducamTOFCamera.hpp"
#include <opencv2/core.hpp>

class ImageCaptureModule {
public:
    ImageCaptureModule(const ImageCaptureModule&) = delete;
    ImageCaptureModule& operator=(const ImageCaptureModule&) = delete;
    ImageCaptureModule(ImageCaptureModule&&) = delete;
    ImageCaptureModule& operator=(ImageCaptureModule&&) = delete;

    static ImageCaptureModule& get_instance();

    bool initialize();
    bool captureFrame();
    Frame preprocessDepth();

    void start_capture();
    void stop_capture();

    void start();

private:
    bool running;
    Arducam::ArducamTOFCamera tof_;
    Arducam::ArducamFrameBuffer* frame_; // imagen
    cv::Mat depth_frame_; // imagen mapa de profundidad
    cv::Mat result_frame_; // imagen final del preprocesamiento

    ImageCaptureModule();
    ~ImageCaptureModule();
};
