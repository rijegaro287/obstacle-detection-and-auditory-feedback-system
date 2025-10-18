#include "image_capture_module.hpp"

#include <gtest/gtest.h>

using namespace std;

TEST(ImageCaptureModuleTest, BasicFunctionality) {
  ImageCaptureModule& icm = ImageCaptureModule::get_instance();
}
