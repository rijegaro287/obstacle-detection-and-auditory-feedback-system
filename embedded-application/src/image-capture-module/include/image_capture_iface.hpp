#pragma once

/**
 * @file image_capture_iface.hpp
 * @brief Public façade for the image capture module.
 */

#include <opencv2/core.hpp>
#include "control_iface.hpp"

/**
 * @class IImageCapture
 * @ingroup image_capture_module
 * @brief Static wrapper delegating functionality to the
 * @ref ImageCaptureModule singleton.
 */
class IImageCapture {
public:
    /** See @ref ImageCaptureModule::start_capture. */
    static void start_capture();

    /** See @ref ImageCaptureModule::stop_capture. */
    static void stop_capture();
    
    /** See @ref ImageCaptureModule::initialize. */
    static bool initialize();

    /** See @ref ImageCaptureModule::captureFrame. */
    static bool captureFrame();

    /** See @ref ImageCaptureModule::preprocessDepth. */
    static Frame preprocessDepth();
};
