#include <gtest/gtest.h>
#include <cmath>
#include <cstdlib>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#define private public
#define protected public
#include "obstacle_detection_module.hpp"
#undef private
#undef protected

using namespace std;

class ObstacleDetectionModuleTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    setenv("QT_QPA_PLATFORM", "offscreen", 1);
  }

  void SetUp() override {
    module = &ObstacleDetectionModule::get_instance();
    module->running = false;
  }

  ObstacleDetectionModule* module;
};

TEST_F(ObstacleDetectionModuleTest, SegmentRedDetectsTargetRanges) {
  cv::Mat hsv(1, 3, CV_8UC3);
  hsv.at<cv::Vec3b>(0, 0) = cv::Vec3b(0, 200, 200);    // should match red range
  hsv.at<cv::Vec3b>(0, 1) = cv::Vec3b(50, 200, 200);   // outside range
  hsv.at<cv::Vec3b>(0, 2) = cv::Vec3b(170, 200, 200);  // upper red range

  cv::Mat mask = module->segmentRed(hsv);
  EXPECT_EQ(mask.at<uchar>(0, 0), 255);
  EXPECT_EQ(mask.at<uchar>(0, 1), 0);
  EXPECT_EQ(mask.at<uchar>(0, 2), 255);
}

TEST_F(ObstacleDetectionModuleTest, FilterByDepthKeepsOnlyClosePixels) {
  cv::Mat mask = (cv::Mat_<uchar>(1, 3) << 255, 0, 255);
  cv::Mat depth(1, 3, CV_32F);
  depth.at<float>(0, 0) = 1500.0f;
  depth.at<float>(0, 1) = 0.0f;
  depth.at<float>(0, 2) = 3500.0f;

  cv::Mat filtered = module->filterByDepth(mask, depth, 2000.0f);
  EXPECT_EQ(filtered.at<uchar>(0, 0), 255);
  EXPECT_EQ(filtered.at<uchar>(0, 2), 0);
}

TEST_F(ObstacleDetectionModuleTest, FilterByColorDensityPreservesDenseRegions) {
  cv::Mat mask = cv::Mat::zeros(20, 20, CV_8UC1);
  cv::rectangle(mask, cv::Rect(2, 2, 10, 10), cv::Scalar(255), cv::FILLED);

  cv::Mat solid = module->filterByColorDensity(mask, 20.0, 0.5);
  EXPECT_GT(cv::countNonZero(solid), 0);
}

TEST_F(ObstacleDetectionModuleTest, DivideComponentsSeparatesBlobs) {
  cv::Mat mask = cv::Mat::zeros(30, 30, CV_8UC1);
  cv::rectangle(mask, cv::Rect(2, 2, 5, 5), cv::Scalar(255), cv::FILLED);
  cv::rectangle(mask, cv::Rect(20, 20, 5, 5), cv::Scalar(255), cv::FILLED);

  auto components = module->divideComponents(mask);
  ASSERT_GE(components.stats.rows, 3); // background + two components

  int label_a = components.labels.at<int>(3, 3);
  int label_b = components.labels.at<int>(22, 22);
  EXPECT_NE(label_a, 0);
  EXPECT_NE(label_b, 0);
  EXPECT_NE(label_a, label_b);
}

TEST_F(ObstacleDetectionModuleTest, SelectObstaclePrefersCloserObject) {
  cv::Mat mask = cv::Mat::zeros(40, 40, CV_8UC1);
  cv::Rect close_rect(5, 5, 8, 8);
  cv::Rect far_rect(25, 25, 8, 8);
  cv::rectangle(mask, close_rect, cv::Scalar(255), cv::FILLED);
  cv::rectangle(mask, far_rect, cv::Scalar(255), cv::FILLED);

  auto components = module->divideComponents(mask);

  cv::Mat depth = cv::Mat::ones(40, 40, CV_32F) * 3500.0f;
  depth(close_rect).setTo(500.0f);
  depth(far_rect).setTo(2500.0f);

  Obstacle obstacle = module->selectObstacle(components, depth);
  EXPECT_GT(obstacle.score, 0.0);
  EXPECT_GT(obstacle.meanDepth, 0.0);
  EXPECT_LT(obstacle.centroid.x, far_rect.x); // centroid should belong to close_rect
}

TEST_F(ObstacleDetectionModuleTest, CalculateAnglesForCenteredObstacle) {
  Obstacle obstacle = {};
  obstacle.centroid = cv::Point(120, 90);
  Obstacle result = module->calculateAngles(obstacle);
  EXPECT_NEAR(result.azimuth, 0.0, 1e-6);
  EXPECT_NEAR(result.elevation, 0.0, 1e-6);
}

TEST_F(ObstacleDetectionModuleTest, MapAzimuthWrapsAsExpected) {
  EXPECT_DOUBLE_EQ(module->mapAzimuth(-15.0), 15.0);
  EXPECT_DOUBLE_EQ(module->mapAzimuth(20.0), 340.0);
}

TEST_F(ObstacleDetectionModuleTest, DetectPipelineProducesObstacle) {
  cv::Mat image = cv::Mat::zeros(120, 160, CV_8UC3);
  cv::rectangle(image, cv::Rect(50, 40, 30, 30), cv::Scalar(0, 0, 255), cv::FILLED);

  cv::Mat depth = cv::Mat::ones(120, 160, CV_32F) * 1000.0f;

  Obstacle obstacle = module->detect(image, depth);
  EXPECT_GT(obstacle.meanDepth, 0.0);
  EXPECT_FALSE(obstacle.image.empty());
}

TEST_F(ObstacleDetectionModuleTest, StartStopDetectionTogglesRunningState) {
  EXPECT_FALSE(module->running);
  module->start_detection();
  EXPECT_TRUE(module->running);
  module->stop_detection();
  EXPECT_FALSE(module->running);
}
