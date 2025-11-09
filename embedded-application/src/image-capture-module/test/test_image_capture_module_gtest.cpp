#include <gtest/gtest.h>
#include <cstdlib>
#include <vector>

#include <opencv2/core.hpp>

#define private public
#define protected public
#include "image_capture_module.hpp"
#undef private
#undef protected

using namespace std;

class ImageCaptureModuleTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    setenv("QT_QPA_PLATFORM", "offscreen", 1);
  }

  void SetUp() override {
    module = &ImageCaptureModule::get_instance();
    module->running = false;
    module->camera_initialized = false;
    module->depth_frame_ = cv::Mat();
    module->result_frame_ = cv::Mat();
  }

  ImageCaptureModule* module;
};

TEST_F(ImageCaptureModuleTest, StartStopCaptureTogglesRunningState) {
  EXPECT_FALSE(module->running);
  module->start_capture();
  EXPECT_TRUE(module->running);
  module->stop_capture();
  EXPECT_FALSE(module->running);
}

TEST_F(ImageCaptureModuleTest, PreprocessDepthReturnsEmptyWhenFrameMissing) {
  module->depth_frame_ = cv::Mat();
  Frame frame = module->preprocessDepth();
  EXPECT_TRUE(frame.depthMap.empty());
  EXPECT_TRUE(frame.image.empty());
}

TEST_F(ImageCaptureModuleTest, PreprocessDepthGeneratesColorizedOutput) {
  cv::Mat depth(4, 4, CV_32F);
  for (int r = 0; r < depth.rows; ++r) {
    for (int c = 0; c < depth.cols; ++c) {
      depth.at<float>(r, c) = static_cast<float>((r + 1) * (c + 1) * 100);
    }
  }

  module->depth_frame_ = depth.clone();
  module->result_frame_ = cv::Mat();

  Frame frame = module->preprocessDepth();

  EXPECT_FALSE(frame.depthMap.empty());
  EXPECT_EQ(frame.depthMap.rows, depth.rows);
  EXPECT_EQ(frame.depthMap.cols, depth.cols);
  EXPECT_FALSE(frame.image.empty());
  EXPECT_EQ(frame.image.type(), CV_8UC3);
  EXPECT_FALSE(module->result_frame_.empty());
}
