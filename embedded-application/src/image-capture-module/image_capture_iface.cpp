#include "image_capture_iface.hpp"
#include "image_capture_module.hpp"

void IImageCapture::start_capture(){
    ImageCaptureModule::get_instance().start_capture();
}

void IImageCapture::stop_capture(){
    ImageCaptureModule::get_instance().stop_capture();
}

bool IImageCapture::initialize() {
    return ImageCaptureModule::get_instance().initialize();
}

bool IImageCapture::captureFrame() {
    return ImageCaptureModule::get_instance().captureFrame();
}

Frame IImageCapture::preprocessDepth() {
    return ImageCaptureModule::get_instance().preprocessDepth();
}
