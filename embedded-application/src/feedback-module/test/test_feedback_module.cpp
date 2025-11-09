#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <vector>

#define private public
#define protected public
#include "feedback_module.hpp"
#undef private
#undef protected

using namespace std;

class FeedbackModuleTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    // Trigger dataset loading once for the entire suite.
    FeedbackModule::get_instance();
  }

  void SetUp() override {
    module = &FeedbackModule::get_instance();
    module->stop_feedback();
    module->running = false;
    module->set_volume(50);
    module->feedback_mode = NON_VERBAL_MODE;
  }

  FeedbackModule* module;
};

TEST_F(FeedbackModuleTest, StartStopFeedbackTogglesRunningState) {
  EXPECT_FALSE(module->running);
  module->start_feedback();
  EXPECT_TRUE(module->running);
  module->stop_feedback();
  EXPECT_FALSE(module->running);
}

TEST_F(FeedbackModuleTest, SetVolumeScalesLinearlyAndClamps) {
  module->set_volume(25);
  EXPECT_NEAR(module->volume, 0.25f, 1e-6f);

  module->set_volume(150);
  EXPECT_FLOAT_EQ(module->volume, 1.0f);
}

TEST_F(FeedbackModuleTest, SetFeedbackModeUpdatesState) {
  module->set_feedback_mode(VERBAL_MODE);
  EXPECT_EQ(module->feedback_mode, VERBAL_MODE);

  module->set_feedback_mode(NON_VERBAL_MODE);
  EXPECT_EQ(module->feedback_mode, NON_VERBAL_MODE);
}

TEST_F(FeedbackModuleTest, CalculateVerbalPositionProducesExpectedMasks) {
  Obstacle obstacle = {};

  obstacle.azimuth = 0.0;
  obstacle.elevation = 0.0;
  uint8_t center = module->calculate_verbal_position(obstacle);
  EXPECT_EQ(center, HORIZONTALLY_CENTERED_MASK | VERTICALLY_CENTERED_MASK);

  obstacle.azimuth = 15.0;
  obstacle.elevation = 0.0;
  uint8_t left = module->calculate_verbal_position(obstacle);
  EXPECT_TRUE(left & LEFT_MASK);
  EXPECT_TRUE(left & VERTICALLY_CENTERED_MASK);

  obstacle.azimuth = 340.0;
  obstacle.elevation = 15.0;
  uint8_t rightAbove = module->calculate_verbal_position(obstacle);
  EXPECT_TRUE(rightAbove & RIGHT_MASK);
  EXPECT_TRUE(rightAbove & ABOVE_MASK);
}

TEST_F(FeedbackModuleTest, GenerateVerbalFeedbackProducesStereoOutput) {
  module->set_volume(100);
  Obstacle obstacle = {};
  obstacle.azimuth = 0.0;
  obstacle.elevation = 0.0;
  obstacle.meanDepth = 500.0;

  Audio audio = module->generate_verbal_feedback(obstacle);
  ASSERT_FALSE(audio.left_signal.empty());
  EXPECT_EQ(audio.left_signal.size(), audio.right_signal.size());
  EXPECT_EQ(audio.sample_rate, VERBAL_SAMPLE_RATE);
  EXPECT_NEAR(audio.gain, 0.75f, 1e-6f);
  EXPECT_EQ(audio.left_signal.front(), audio.right_signal.front());
}

TEST_F(FeedbackModuleTest, GenerateNonVerbalFeedbackAppliesGainAndLength) {
  module->set_volume(100);
  Obstacle obstacle = {};
  obstacle.azimuth = 0.0;
  obstacle.elevation = 0.0;
  obstacle.meanDepth = 1000.0;

  Audio audio = module->generate_non_verbal_feedback(obstacle);
  ASSERT_EQ(audio.left_signal.size(), TAP_N_SAMPLES);
  ASSERT_EQ(audio.right_signal.size(), TAP_N_SAMPLES);
  EXPECT_EQ(audio.sample_rate, NON_VERBAL_SAMPLE_RATE);
  EXPECT_NEAR(audio.gain, 0.5f, 1e-6f);
  EXPECT_TRUE(any_of(audio.left_signal.begin(), audio.left_signal.end(), [](float v) { return fabs(v) > 1e-6f; }));
}

TEST_F(FeedbackModuleTest, GenerateFeedbackRespectsModeSelection) {
  module->set_volume(100);
  Obstacle obstacle = {};
  obstacle.azimuth = 0.0;
  obstacle.elevation = 0.0;
  obstacle.meanDepth = 750.0;

  module->set_feedback_mode(NON_VERBAL_MODE);
  Audio non_verbal = module->generate_feedback(obstacle);
  EXPECT_EQ(non_verbal.sample_rate, NON_VERBAL_SAMPLE_RATE);

  module->set_feedback_mode(VERBAL_MODE);
  Audio verbal = module->generate_feedback(obstacle);
  EXPECT_EQ(verbal.sample_rate, VERBAL_SAMPLE_RATE);
}

TEST(FeedbackModuleUtilityTest, KDTreeFindsNearestNeighbour) {
  kd_tree<3> tree;
  tree.insert(0, {0.0, 0.0, 0.0});
  tree.insert(1, {10.0, 0.0, 0.0});
  tree.insert(2, {0.0, 10.0, 0.0});

  EXPECT_EQ(tree.find_nearest({0.1, 0.1, 0.1}), 0u);
  EXPECT_EQ(tree.find_nearest({9.5, 0.1, 0.0}), 1u);
  EXPECT_EQ(tree.find_nearest({0.1, 9.5, 0.0}), 2u);
}
