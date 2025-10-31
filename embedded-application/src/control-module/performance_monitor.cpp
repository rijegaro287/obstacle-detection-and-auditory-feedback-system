#include "performance_monitor.hpp"
#include <stdio.h>

PerformanceMonitor& PerformanceMonitor::get_instance(bool enabled) {
	static PerformanceMonitor instance(enabled);
	return instance;
}

PerformanceMonitor::PerformanceMonitor(bool enabled) {
  this->performance_monitoring_enabled = enabled;
  this->sample_indices.capture_idx = 0;
  this->sample_indices.detection_idx = 0;
  this->sample_indices.feedback_idx = 0;
  this->sample_indices.transmission_idx = 0;
}

PerformanceMonitor::~PerformanceMonitor() {

}

void PerformanceMonitor::add_capture_sample_start() {
  if (!this->performance_monitoring_enabled) {
    return;
  }

  if (this->sample_indices.capture_idx >= N_SAMPLES) {
    return;
  }

  uint64_t idx = this->sample_indices.capture_idx;
  ProcessingTimes& times = this->processing_times[idx];
  times.capture_start_time = std::chrono::steady_clock::now();
}

void PerformanceMonitor::add_capture_sample_end() {
  if (!this->performance_monitoring_enabled) {
    return;
  }

  if (this->sample_indices.capture_idx >= N_SAMPLES) {
    return;
  }

  uint64_t idx = this->sample_indices.capture_idx;
  ProcessingTimes& times = this->processing_times[idx];
  times.capture_end_time = std::chrono::steady_clock::now();

  // printf("Capture Sample %lu recorded.\n", idx);
  // printf("Capture Time: %lu ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(times.capture_end_time - times.capture_start_time).count());

  this->sample_indices.capture_idx++;
}

void PerformanceMonitor::add_detection_sample_start() {
  if (!this->performance_monitoring_enabled) {
    return;
  }

  if (this->sample_indices.detection_idx >= N_SAMPLES) {
    return;
  }

  uint64_t idx = this->sample_indices.detection_idx;
  ProcessingTimes& times = this->processing_times[idx];
  times.detection_start_time = std::chrono::steady_clock::now();
}

void PerformanceMonitor::add_detection_sample_end() {
  if (!this->performance_monitoring_enabled) {
    return;
  }

  if (this->sample_indices.detection_idx >= N_SAMPLES) {
    return;
  }

  uint64_t idx = this->sample_indices.detection_idx;
  ProcessingTimes& times = this->processing_times[idx];
  times.detection_end_time = std::chrono::steady_clock::now();

  // printf("Detection Sample %lu recorded.\n", idx);
  // printf("Detection Time: %lu ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(times.detection_end_time - times.detection_start_time).count());

  this->sample_indices.detection_idx++;
}

void PerformanceMonitor::add_feedback_sample_start() {
  if (!this->performance_monitoring_enabled) {
    return;
  }

  if (this->sample_indices.feedback_idx >= N_SAMPLES) {
    return;
  }

  uint64_t idx = this->sample_indices.feedback_idx;
  ProcessingTimes& times = this->processing_times[idx];
  times.feedback_start_time = std::chrono::steady_clock::now();
}

void PerformanceMonitor::add_feedback_sample_end() {
  if (!this->performance_monitoring_enabled) {
    return;
  }

  if (this->sample_indices.feedback_idx >= N_SAMPLES) {
    return;
  }

  uint64_t idx = this->sample_indices.feedback_idx;
  ProcessingTimes& times = this->processing_times[idx];
  times.feedback_end_time = std::chrono::steady_clock::now();

  // printf("Feedback Sample %lu recorded.\n", idx);
  // printf("Feedback Time: %lu ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(times.feedback_end_time - times.feedback_start_time).count());

  this->sample_indices.feedback_idx++;
}

void PerformanceMonitor::add_transmission_sample_start() {
  if (!this->performance_monitoring_enabled) {
    return;
  }

  if (this->sample_indices.transmission_idx >= N_SAMPLES) {
    return;
  }

  uint64_t idx = this->sample_indices.transmission_idx;
  ProcessingTimes& times = this->processing_times[idx];
  times.transmission_start_time = std::chrono::steady_clock::now();
}

void PerformanceMonitor::add_transmission_sample_end(uint64_t signal_duration_ms) {
  if (!this->performance_monitoring_enabled) {
    return;
  }

  if (this->sample_indices.transmission_idx >= N_SAMPLES) {
    return;
  }

  auto signal_duration = std::chrono::milliseconds(signal_duration_ms);

  uint64_t idx = this->sample_indices.transmission_idx;
  ProcessingTimes& times = this->processing_times[idx];
  times.transmission_end_time = std::chrono::steady_clock::now() - signal_duration;

  if (std::chrono::duration_cast<std::chrono::milliseconds>(times.transmission_end_time - times.transmission_start_time).count() > 1000) {
    times.transmission_end_time = times.transmission_start_time;
  }

  printf("Transmission Sample %lu recorded.\n", idx);
  printf("Transmission Time: %lu ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(times.transmission_end_time - times.transmission_start_time).count());

  this->sample_indices.transmission_idx++;
}
