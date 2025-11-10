#include <gtest/gtest.h>
#include <array>
#include <chrono>
#include <cmath>
#include <mutex>
#include <numeric>
#include <thread>
#include <vector>

#include <opencv2/core.hpp>

#define private public
#define protected public
#include "control_module.hpp"
#include "feedback_module.hpp"
#include "image_capture_module.hpp"
#include "obstacle_detection_module.hpp"
#include "transmission_module.hpp"
#include "performance_monitor.hpp"
#undef private
#undef protected

using namespace std;

namespace {

std::once_flag unlock_once;

void ensure_control_module_ready() {
  std::call_once(unlock_once, []() {
    ControlModule::get_instance().unlock_mutexes();
  });
}

void reset_control_buffers(ControlModule& control) {
  Frame empty_frame;
  control.set_frame(empty_frame);

  Obstacle empty_obstacle;
  empty_obstacle.image = cv::Mat();
  control.set_obstacle(empty_obstacle);

  Audio empty_audio;
  control.set_audio_data(empty_audio);
}

void fill_processing_times(PerformanceMonitor& monitor,
                           uint64_t capture_ms,
                           uint64_t detection_ms,
                           uint64_t feedback_ms,
                           uint64_t detection_offset_ms,
                           uint64_t feedback_offset_ms) {
  TimePoint base = chrono::steady_clock::now();
  for (uint64_t i = 0; i < N_SAMPLES; ++i) {
    ProcessingTimes& times = monitor.processing_times[i];
    times.capture_start_time = base;
    times.capture_end_time = base + chrono::milliseconds(capture_ms);

    times.detection_start_time = base + chrono::milliseconds(detection_offset_ms);
    times.detection_end_time = times.detection_start_time + chrono::milliseconds(detection_ms);

    times.feedback_start_time = base + chrono::milliseconds(feedback_offset_ms);
    times.feedback_end_time = times.feedback_start_time + chrono::milliseconds(feedback_ms);
  }
}

} // namespace

class ControlModuleTest : public ::testing::Test {
protected:
  void SetUp() override {
    auto& control = ControlModule::get_instance();
    ensure_control_module_ready();
    reset_control_buffers(control);
    FeedbackModule::get_instance().stop_feedback();
    ImageCaptureModule::get_instance().running = false;
    ObstacleDetectionModule::get_instance().running = false;
    TransmissionModule::get_instance().running = false;
  }
};

TEST_F(ControlModuleTest, FrameRoundTripClearsStoredFrame) {
  auto& control = ControlModule::get_instance();

  Frame frame;
  frame.depthMap = cv::Mat::ones(2, 2, CV_8U) * 3;
  frame.image = cv::Mat::eye(2, 2, CV_8U);

  control.set_frame(frame);

  Frame retrieved = control.get_frame();
  EXPECT_EQ(cv::norm(retrieved.depthMap, frame.depthMap, cv::NORM_INF), 0);
  EXPECT_EQ(cv::norm(retrieved.image, frame.image, cv::NORM_INF), 0);

  Frame cleared = control.get_frame();
  EXPECT_TRUE(cleared.depthMap.empty());
  EXPECT_TRUE(cleared.image.empty());
}

TEST_F(ControlModuleTest, ObstacleRoundTripCopiesData) {
  auto& control = ControlModule::get_instance();

  Obstacle obstacle = {};
  obstacle.label = 7;
  obstacle.area = 42.0;
  obstacle.meanDepth = 1.2;
  obstacle.score = 11.0;
  obstacle.azimuth = 25.0;
  obstacle.elevation = -5.0;
  obstacle.centroid = cv::Point(4, 9);
  obstacle.image = cv::Mat::ones(3, 3, CV_8U) * 200;

  control.set_obstacle(obstacle);
  Obstacle retrieved = control.get_obstacle();

  // Only the fields explicitly copied by ControlModule::set_obstacle should persist.
  EXPECT_EQ(retrieved.label, 0);
  EXPECT_DOUBLE_EQ(retrieved.area, 0.0);
  EXPECT_DOUBLE_EQ(retrieved.meanDepth, obstacle.meanDepth);
  EXPECT_DOUBLE_EQ(retrieved.score, 0.0);
  EXPECT_DOUBLE_EQ(retrieved.azimuth, obstacle.azimuth);
  EXPECT_DOUBLE_EQ(retrieved.elevation, obstacle.elevation);
  EXPECT_EQ(retrieved.centroid, cv::Point());
  EXPECT_EQ(cv::norm(retrieved.image, obstacle.image, cv::NORM_INF), 0);

  Obstacle cleared = control.get_obstacle();
  EXPECT_TRUE(cleared.image.empty());
  EXPECT_DOUBLE_EQ(cleared.meanDepth, 0.0);
}

TEST_F(ControlModuleTest, AudioRoundTripClearsAfterRead) {
  auto& control = ControlModule::get_instance();

  Audio audio;
  audio.left_signal = {0.1f, -0.2f, 0.3f};
  audio.right_signal = {0.4f, -0.5f, 0.6f};
  audio.sample_rate = 48000;
  audio.gain = 0.75f;

  control.set_audio_data(audio);

  Audio retrieved = control.get_audio_data();
  EXPECT_EQ(retrieved.left_signal, audio.left_signal);
  EXPECT_EQ(retrieved.right_signal, audio.right_signal);
  EXPECT_EQ(retrieved.sample_rate, audio.sample_rate);
  EXPECT_FLOAT_EQ(retrieved.gain, audio.gain);

  Audio cleared = control.get_audio_data();
  EXPECT_TRUE(cleared.left_signal.empty());
  EXPECT_EQ(cleared.sample_rate, 0u);
}

TEST_F(ControlModuleTest, SetVolumePropagatesToFeedbackModule) {
  auto& control = ControlModule::get_instance();
  auto& feedback = FeedbackModule::get_instance();
  float original_volume = feedback.volume;

  control.set_volume(150);
  EXPECT_FLOAT_EQ(feedback.volume, 1.0f);

  control.set_volume(25);
  EXPECT_NEAR(feedback.volume, 0.25f, 1e-6f);

  feedback.volume = original_volume;
}

TEST_F(ControlModuleTest, SetFeedbackModePropagatesToFeedbackModule) {
  auto& control = ControlModule::get_instance();
  auto& feedback = FeedbackModule::get_instance();
  FEEDBACK_MODES original_mode = feedback.feedback_mode;

  control.set_feedback_mode(VERBAL_MODE);
  EXPECT_EQ(feedback.feedback_mode, VERBAL_MODE);

  control.set_feedback_mode(NON_VERBAL_MODE);
  EXPECT_EQ(feedback.feedback_mode, NON_VERBAL_MODE);

  feedback.feedback_mode = original_mode;
}

TEST_F(ControlModuleTest, StopFeedbackClearsState) {
  auto& control = ControlModule::get_instance();

  Frame frame;
  frame.depthMap = cv::Mat::ones(1, 1, CV_8U);
  control.set_frame(frame);

  Obstacle obstacle = {};
  obstacle.meanDepth = 0.9;
  obstacle.image = cv::Mat::ones(1, 1, CV_8U);
  control.set_obstacle(obstacle);

  Audio audio;
  audio.left_signal = {1.0f};
  audio.right_signal = {1.0f};
  audio.sample_rate = 1;
  audio.gain = 1.0f;
  control.set_audio_data(audio);

  control.stop_feedback();

  EXPECT_TRUE(control.get_frame().depthMap.empty());
  EXPECT_TRUE(control.get_obstacle().image.empty());
  EXPECT_TRUE(control.get_audio_data().left_signal.empty());
}

TEST_F(ControlModuleTest, StartFeedbackEnablesSubsystems) {
  auto& control = ControlModule::get_instance();
  auto& image_module = ImageCaptureModule::get_instance();
  auto& detection_module = ObstacleDetectionModule::get_instance();
  auto& feedback_module = FeedbackModule::get_instance();
  auto& transmission_module = TransmissionModule::get_instance();

  bool original_image_running = image_module.running;
  bool original_detection_running = detection_module.running;
  bool original_feedback_running = feedback_module.running;
  bool original_transmission_running = transmission_module.running;

  image_module.running = false;
  detection_module.running = false;
  feedback_module.running = false;
  transmission_module.running = false;

  control.start_feedback();

  EXPECT_TRUE(image_module.running);
  EXPECT_TRUE(detection_module.running);
  EXPECT_TRUE(feedback_module.running);
  EXPECT_TRUE(transmission_module.running);

  control.stop_feedback();

  EXPECT_FALSE(image_module.running);
  EXPECT_FALSE(detection_module.running);
  EXPECT_FALSE(feedback_module.running);
  EXPECT_FALSE(transmission_module.running);

  image_module.running = original_image_running;
  detection_module.running = original_detection_running;
  feedback_module.running = original_feedback_running;
  transmission_module.running = original_transmission_running;
}

TEST_F(ControlModuleTest, PerformanceMonitorIgnoresSamplesWhenDisabled) {
  auto& monitor = PerformanceMonitor::get_instance();
  monitor.performance_monitoring_enabled = false;
  monitor.sample_indices.capture_idx = 0;

  monitor.add_capture_sample_start();
  monitor.add_capture_sample_end();

  EXPECT_EQ(monitor.sample_indices.capture_idx, 0u);
}

TEST_F(ControlModuleTest, PerformanceMonitorRecordsCaptureSamplesWhenEnabled) {
  auto& monitor = PerformanceMonitor::get_instance();
  monitor.performance_monitoring_enabled = true;
  monitor.sample_indices.capture_idx = 0;

  monitor.add_capture_sample_start();
  this_thread::sleep_for(chrono::milliseconds(1));
  monitor.add_capture_sample_end();

  EXPECT_EQ(monitor.sample_indices.capture_idx, 1u);

  monitor.performance_monitoring_enabled = false;
  monitor.sample_indices.capture_idx = 0;
}

TEST_F(ControlModuleTest, PerformanceMonitorRecordsDetectionAndFeedbackSamplesWhenEnabled) {
  auto& monitor = PerformanceMonitor::get_instance();
  monitor.performance_monitoring_enabled = true;
  monitor.sample_indices.detection_idx = 0;
  monitor.sample_indices.feedback_idx = 0;

  monitor.add_detection_sample_start();
  this_thread::sleep_for(chrono::milliseconds(1));
  monitor.add_detection_sample_end();
  EXPECT_EQ(monitor.sample_indices.detection_idx, 1u);

  monitor.add_feedback_sample_start();
  this_thread::sleep_for(chrono::milliseconds(1));
  monitor.add_feedback_sample_end();
  EXPECT_EQ(monitor.sample_indices.feedback_idx, 1u);

  monitor.performance_monitoring_enabled = false;
  monitor.sample_indices.detection_idx = 0;
  monitor.sample_indices.feedback_idx = 0;
}

TEST_F(ControlModuleTest, PerformanceMonitorComputeStddevMatchesManualCalculation) {
  auto& monitor = PerformanceMonitor::get_instance();
  array<uint64_t, N_SAMPLES> values;
  for (uint64_t i = 0; i < N_SAMPLES; ++i) {
    values[i] = i % 7;
  }

  double mean = accumulate(values.begin(), values.end(), 0.0) / static_cast<double>(N_SAMPLES);
  double expected_variance = 0.0;
  for (uint64_t value : values) {
    double diff = static_cast<double>(value) - mean;
    expected_variance += diff * diff;
  }
  expected_variance /= static_cast<double>(N_SAMPLES);
  double expected_stddev = sqrt(expected_variance);

  double computed = monitor.compute_stddev(values.data(), mean);
  EXPECT_NEAR(computed, expected_stddev, 1e-6);
}

TEST_F(ControlModuleTest, PerformanceMonitorMapTotalTimesProducesExpectedDurations) {
  auto& monitor = PerformanceMonitor::get_instance();
  fill_processing_times(monitor, 2, 3, 3, 2, 5);

  TotalTimes totals = monitor.map_total_times();

  for (uint64_t idx : {0, N_SAMPLES / 2, N_SAMPLES - 1}) {
    EXPECT_EQ(totals.capture_times_ms[idx], 2u);
    EXPECT_EQ(totals.detection_times_ms[idx], 3u);
    EXPECT_EQ(totals.feedback_times_ms[idx], 3u);
    EXPECT_EQ(totals.total_time_ms[idx], 8u);
  }
}

TEST_F(ControlModuleTest, PerformanceMonitorRecordPerformanceStatsRollsOverAfterRepeats) {
  auto& monitor = PerformanceMonitor::get_instance();
  fill_processing_times(monitor, 5, 5, 5, 5, 10);

  monitor.sample_indices.capture_idx = N_SAMPLES;
  monitor.sample_indices.detection_idx = N_SAMPLES;
  monitor.sample_indices.feedback_idx = N_SAMPLES;
  monitor.repetition_count = N_REPEATS - 1;

  bool finished = monitor.record_performance_stats();
  EXPECT_TRUE(finished);
  EXPECT_EQ(monitor.repetition_count, N_REPEATS);
  EXPECT_EQ(monitor.sample_indices.capture_idx, 0u);
  EXPECT_EQ(monitor.sample_indices.detection_idx, 0u);
  EXPECT_EQ(monitor.sample_indices.feedback_idx, 0u);

  const PerformanceStats& stats = monitor.performance_stats[N_REPEATS - 1];
  EXPECT_GT(stats.total_avg_ms, 0.0);

  monitor.repetition_count = 0;
  monitor.sample_indices = SampleIndices();
}

TEST_F(ControlModuleTest, PerformanceMonitorSkipsSamplesExceedingDurationThreshold) {
  auto& monitor = PerformanceMonitor::get_instance();
  monitor.performance_monitoring_enabled = true;
  monitor.sample_indices.capture_idx = 0;
  monitor.processing_times[0].capture_start_time = chrono::steady_clock::now() - chrono::milliseconds(150);
  monitor.add_capture_sample_end();
  EXPECT_EQ(monitor.sample_indices.capture_idx, 0u);

  monitor.sample_indices.detection_idx = 0;
  monitor.processing_times[0].detection_start_time = chrono::steady_clock::now() - chrono::milliseconds(150);
  monitor.add_detection_sample_end();
  EXPECT_EQ(monitor.sample_indices.detection_idx, 0u);

  monitor.sample_indices.feedback_idx = 0;
  monitor.processing_times[0].feedback_start_time = chrono::steady_clock::now() - chrono::milliseconds(150);
  monitor.add_feedback_sample_end();
  EXPECT_EQ(monitor.sample_indices.feedback_idx, 0u);

  monitor.performance_monitoring_enabled = false;
  monitor.sample_indices = SampleIndices();
}

TEST_F(ControlModuleTest, PerformanceMonitorRecordPerformanceStatsWaitsForAllSamples) {
  auto& monitor = PerformanceMonitor::get_instance();
  monitor.sample_indices.capture_idx = N_SAMPLES;
  monitor.sample_indices.detection_idx = N_SAMPLES - 1;
  monitor.sample_indices.feedback_idx = N_SAMPLES;
  monitor.repetition_count = 0;

  EXPECT_FALSE(monitor.record_performance_stats());
  monitor.sample_indices = SampleIndices();
}

TEST_F(ControlModuleTest, BasicFunctionality) {
  ControlModule& cm = ControlModule::get_instance();
  EXPECT_NO_THROW({ ensure_control_module_ready(); });
  (void)cm;
}

TEST_F(ControlModuleTest, IControlFacadeForFrameObstacleAndAudio) {
  ensure_control_module_ready();
  // Frame
  Frame f;
  f.depthMap = cv::Mat::ones(2,2,CV_8U);
  IControl::set_frame(f);
  Frame rf = IControl::get_frame();
  EXPECT_FALSE(rf.depthMap.empty());

  // Obstacle
  Obstacle o = {};
  o.meanDepth = 123.4;
  o.image = cv::Mat::ones(1,1,CV_8U);
  IControl::set_obstacle(o);
  Obstacle ro = IControl::get_obstacle();
  EXPECT_DOUBLE_EQ(ro.meanDepth, o.meanDepth);

  // Audio
  Audio a;
  a.left_signal = {0.1f};
  a.right_signal = {0.2f};
  a.sample_rate = 22050;
  IControl::set_audio_data(a);
  Audio ra = IControl::get_audio_data();
  EXPECT_EQ(ra.left_signal.size(), 1u);
  EXPECT_EQ(ra.sample_rate, 22050u);
}

TEST_F(ControlModuleTest, IControlPerformanceMonitorSamplesViaFacade) {
  auto& monitor = PerformanceMonitor::get_instance();
  // enable monitoring
  monitor.set_performance_monitoring(true);

  monitor.sample_indices.capture_idx = 0;
  IControl::add_capture_sample_start();
  this_thread::sleep_for(chrono::milliseconds(1));
  IControl::add_capture_sample_end();
  EXPECT_EQ(monitor.sample_indices.capture_idx, 1u);

  monitor.sample_indices.detection_idx = 0;
  IControl::add_detection_sample_start();
  this_thread::sleep_for(chrono::milliseconds(1));
  IControl::add_detection_sample_end();
  EXPECT_EQ(monitor.sample_indices.detection_idx, 1u);

  monitor.sample_indices.feedback_idx = 0;
  IControl::add_feedback_sample_start();
  this_thread::sleep_for(chrono::milliseconds(1));
  IControl::add_feedback_sample_end();
  EXPECT_EQ(monitor.sample_indices.feedback_idx, 1u);

  // restore
  monitor.set_performance_monitoring(false);
  monitor.sample_indices = SampleIndices();
}

TEST_F(ControlModuleTest, IControlStartStopFeedbackViaFacade) {
  auto& image_module = ImageCaptureModule::get_instance();
  auto& detection_module = ObstacleDetectionModule::get_instance();
  auto& feedback_module = FeedbackModule::get_instance();
  auto& transmission_module = TransmissionModule::get_instance();

  bool orig_image = image_module.running;
  bool orig_detection = detection_module.running;
  bool orig_feedback = feedback_module.running;
  bool orig_transmission = transmission_module.running;

  image_module.running = false;
  detection_module.running = false;
  feedback_module.running = false;
  transmission_module.running = false;

  IControl::start_feedback();
  EXPECT_TRUE(image_module.running);
  EXPECT_TRUE(detection_module.running);
  EXPECT_TRUE(feedback_module.running);
  EXPECT_TRUE(transmission_module.running);

  IControl::stop_feedback();
  EXPECT_FALSE(image_module.running);
  EXPECT_FALSE(detection_module.running);
  EXPECT_FALSE(feedback_module.running);
  EXPECT_FALSE(transmission_module.running);

  image_module.running = orig_image;
  detection_module.running = orig_detection;
  feedback_module.running = orig_feedback;
  transmission_module.running = orig_transmission;
}
