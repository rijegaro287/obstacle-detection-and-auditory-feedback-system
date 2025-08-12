#pragma once

#ifndef IMAGE_CAPTURE_MODULE_HPP
#define IMAGE_CAPTURE_MODULE_HPP

#include "ArducamTOFCamera.hpp"
#include <opencv2/core.hpp>

class ImageCaptureModule {
public:
    ImageCaptureModule();
    ~ImageCaptureModule();

    bool initialize();
    bool captureFrame();
    cv::Mat preprocessDepth();

private:
    Arducam::ArducamTOFCamera tof_;
    Arducam::ArducamFrameBuffer* frame_; // imagen
    cv::Mat depth_frame_; // imagen mapa de profundidad
    cv::Mat result_frame_; // imagen final del preprocesamiento
};

#endif // IMAGE_CAPTURE_MODULE_HPP
