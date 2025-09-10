#include "image_capture_iface.hpp"
#include "image_capture_module.h"

// Instancia global interna 
static ImageCaptureModule g_imageCapture;

bool IImageCapture::initialize() {
    return g_imageCapture.initialize();
}

bool IImageCapture::captureFrame() {
    return g_imageCapture.captureFrame();
}

std::pair<cv::Mat, cv::Mat> IImageCapture::preprocessDepth() {
    return g_imageCapture.preprocessDepth();
}
