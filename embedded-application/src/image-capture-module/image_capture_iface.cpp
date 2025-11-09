/**
 * @file image_capture_iface.cpp
 * @brief Implements the @ref IImageCapture façade.
 */

#include "image_capture_iface.hpp"
#include "image_capture_module.hpp"

/** See @ref IImageCapture::start_capture. */
void IImageCapture::start_capture(){
    ImageCaptureModule::get_instance().start_capture();
}

/** See @ref IImageCapture::stop_capture. */
void IImageCapture::stop_capture(){
    ImageCaptureModule::get_instance().stop_capture();
}

/** See @ref IImageCapture::initialize. */
bool IImageCapture::initialize() {
    return ImageCaptureModule::get_instance().initialize();
}

/** See @ref IImageCapture::captureFrame. */
bool IImageCapture::captureFrame() {
    return ImageCaptureModule::get_instance().captureFrame();
}

/** See @ref IImageCapture::preprocessDepth. */
Frame IImageCapture::preprocessDepth() {
    return ImageCaptureModule::get_instance().preprocessDepth();
}
