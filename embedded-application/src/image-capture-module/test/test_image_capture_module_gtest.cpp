#include <gtest/gtest.h>
#include <cstdlib>
#include <vector>

#include <opencv2/core.hpp>

#define private public
#define protected public
#include "image_capture_module.hpp"
#include "image_capture_iface.hpp"
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
    module->frame_ = nullptr;
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

TEST_F(ImageCaptureModuleTest, CaptureFrameFailsWhenCameraIsNotInitialized) {
  module->camera_initialized = false;
  module->frame_ = reinterpret_cast<Arducam::ArducamFrameBuffer*>(0x1);

  EXPECT_FALSE(module->captureFrame());
  EXPECT_EQ(module->frame_, reinterpret_cast<Arducam::ArducamFrameBuffer*>(0x1));
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

TEST_F(ImageCaptureModuleTest, PreprocessDepthClipsValuesAboveMaxDistance) {
  // Build a depth map with values above and below MAX_DISTANCE
  cv::Mat depth(3, 3, CV_32F);
  for (int r = 0; r < depth.rows; ++r) {
    for (int c = 0; c < depth.cols; ++c) {
      if (r == 1 && c == 1) depth.at<float>(r, c) = 5000.0f; // above MAX_DISTANCE
      else depth.at<float>(r, c) = 1000.0f;
    }
  }

  module->depth_frame_ = depth.clone();
  module->result_frame_ = cv::Mat();

  Frame frame = module->preprocessDepth();

  // depth map should be returned as-is
  ASSERT_FALSE(frame.depthMap.empty());
  EXPECT_EQ(frame.depthMap.rows, depth.rows);
  EXPECT_EQ(frame.depthMap.cols, depth.cols);

  // image should be produced and be 3-channel
  ASSERT_FALSE(frame.image.empty());
  EXPECT_EQ(frame.image.type(), CV_8UC3);

  // ensure that the pixel corresponding to the clipped value exists and is colored
  cv::Vec3b pix = frame.image.at<cv::Vec3b>(1,1);
  // at least one channel should be non-zero for the colored mapping
  EXPECT_TRUE(pix[0] != 0 || pix[1] != 0 || pix[2] != 0);
}

TEST_F(ImageCaptureModuleTest, PreprocessDepthHandlesZeroAndSmallSizes) {
  cv::Mat depth = cv::Mat::zeros(1, 1, CV_32F);
  module->depth_frame_ = depth;
  module->result_frame_ = cv::Mat();

  Frame frame = module->preprocessDepth();
  EXPECT_FALSE(frame.depthMap.empty());
  EXPECT_EQ(frame.depthMap.at<float>(0,0), 0.0f);
  EXPECT_FALSE(frame.image.empty());
  EXPECT_EQ(frame.image.rows, 1);
  EXPECT_EQ(frame.image.cols, 1);
}

TEST(ImageCaptureInterfaceTest, FacadeDelegatesBasicControls) {
  auto& module = ImageCaptureModule::get_instance();
  module.running = false;
  IImageCapture::start_capture();
  EXPECT_TRUE(module.running);
  IImageCapture::stop_capture();
  EXPECT_FALSE(module.running);

  module.camera_initialized = false;
  EXPECT_FALSE(IImageCapture::captureFrame());

  module.depth_frame_ = cv::Mat();
  module.result_frame_ = cv::Mat();
  Frame frame = IImageCapture::preprocessDepth();
  EXPECT_TRUE(frame.depthMap.empty());
  EXPECT_TRUE(frame.image.empty());
}
