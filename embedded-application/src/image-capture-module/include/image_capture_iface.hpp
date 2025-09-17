#pragma once
#include <opencv2/core.hpp>
#include "control_iface.hpp"

class IImageCapture {
public:
    static void start_capture();
    static void stop_capture();
    
    static bool initialize();
    static bool captureFrame();
    static Frame preprocessDepth();
};
