/**
 * @file performance_monitor.cpp
 * @brief Implements latency tracking for the processing pipeline.
 */

#include "performance_monitor.hpp"

#include <math.h>
#include <stdio.h>

PerformanceMonitor& PerformanceMonitor::get_instance() {
	static PerformanceMonitor instance;
	return instance;
}

/**
 * @brief Initialize counters and disable monitoring by default.
 */
PerformanceMonitor::PerformanceMonitor() {
  this->performance_monitoring_enabled = false;
  this->repetition_count = 0;
  this->sample_indices.capture_idx = 0;
  this->sample_indices.detection_idx = 0;
  this->sample_indices.feedback_idx = 0;
}

/**
 * @brief Default destructor retained for completeness.
 */
PerformanceMonitor::~PerformanceMonitor() {

}

bool PerformanceMonitor::record_performance_stats() {
  if (this->sample_indices.capture_idx >= N_SAMPLES &&
      this->sample_indices.detection_idx >= N_SAMPLES &&
      this->sample_indices.feedback_idx >= N_SAMPLES
  ) {
    printf("Recording performance stats for repetition %lu\n", this->repetition_count + 1);
    PerformanceStats stats = this->map_performance_stats();
    this->performance_stats[this->repetition_count] = stats;
    this->repetition_count++;
    this->sample_indices = SampleIndices();
    if (this->repetition_count >= N_REPEATS) {
      return true;
    }
  }
  return false;
}

void PerformanceMonitor::print_performance_stats() {
  printf("==================== Performance Statistics ====================\n");
  for (uint64_t i = 0; i < this->repetition_count; i++) {
    PerformanceStats& stats = this->performance_stats[i];
    printf("Repetition %lu:\n", i + 1);
    printf("\tTotal - Avg: %.2f ms, Stddev: %.2f ms\n", stats.total_avg_ms, stats.total_stddev_ms);
    printf("\tCapture - Avg: %.2f ms, Stddev: %.2f ms\n", stats.capture_avg_ms, stats.capture_stddev_ms);
    printf("\tDetection - Avg: %.2f ms, Stddev: %.2f ms\n", stats.detection_avg_ms, stats.detection_stddev_ms);
    printf("\tFeedback - Avg: %.2f ms, Stddev: %.2f ms\n", stats.feedback_avg_ms, stats.feedback_stddev_ms);
  }
}

double PerformanceMonitor::compute_stddev(uint64_t values[], double mean) {
  double sum = 0.0;
  for (uint64_t i = 0; i < N_SAMPLES; i++) {
    sum += pow((values[i] - mean), 2);
  }
  return sqrt(sum / N_SAMPLES);
}

TotalTimes PerformanceMonitor::map_total_times() {
  TotalTimes total_times = TotalTimes();
  for (uint64_t i = 0; i < N_SAMPLES; i++) {
    ProcessingTimes& times = this->processing_times[i];
    total_times.capture_times_ms[i] = std::chrono::duration_cast<std::chrono::milliseconds>(times.capture_end_time - times.capture_start_time).count();
    total_times.detection_times_ms[i] = std::chrono::duration_cast<std::chrono::milliseconds>(times.detection_end_time - times.detection_start_time).count();
    total_times.feedback_times_ms[i] = std::chrono::duration_cast<std::chrono::milliseconds>(times.feedback_end_time - times.feedback_start_time).count();
    total_times.total_time_ms[i] = std::chrono::duration_cast<std::chrono::milliseconds>(times.feedback_end_time - times.detection_start_time).count();
    total_times.total_time_ms[i] += total_times.capture_times_ms[i];
  }
  return total_times;
}

PerformanceStats PerformanceMonitor::map_performance_stats() {
  TotalTimes total_times = this->map_total_times();
  PerformanceStats stats = PerformanceStats();
  for (uint64_t i = 0; i < N_SAMPLES; i++) {
    stats.total_avg_ms += (double)total_times.total_time_ms[i];
    stats.capture_avg_ms += (double)total_times.capture_times_ms[i];
    stats.detection_avg_ms += (double)total_times.detection_times_ms[i];
    stats.feedback_avg_ms += (double)total_times.feedback_times_ms[i];
  }

  stats.total_avg_ms /= (double)N_SAMPLES;
  stats.capture_avg_ms /= (double)N_SAMPLES;
  stats.detection_avg_ms /= (double)N_SAMPLES;
  stats.feedback_avg_ms /= (double)N_SAMPLES;

  stats.total_stddev_ms = this->compute_stddev(
    total_times.total_time_ms,
    stats.total_avg_ms
  );

  stats.capture_stddev_ms = this->compute_stddev(
    total_times.capture_times_ms,
    stats.capture_avg_ms
  );

  stats.detection_stddev_ms = this->compute_stddev(
    total_times.detection_times_ms,
    stats.detection_avg_ms
  );

  stats.feedback_stddev_ms = this->compute_stddev(
    total_times.feedback_times_ms,
    stats.feedback_avg_ms
  );

  return stats;
}

void PerformanceMonitor::set_performance_monitoring(bool enabled) {
  this->performance_monitoring_enabled = enabled;
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
  TimePoint now = std::chrono::steady_clock::now();

  if (std::chrono::duration_cast<std::chrono::milliseconds>(now - times.capture_start_time).count() > 100) {
    return;
  }

  times.capture_end_time = now;
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
  TimePoint now = std::chrono::steady_clock::now();

  if (std::chrono::duration_cast<std::chrono::milliseconds>(now - times.detection_start_time).count() > 100) {
    return;
  }

  times.detection_end_time = now;

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
  TimePoint now = std::chrono::steady_clock::now();

  if (std::chrono::duration_cast<std::chrono::milliseconds>(now - times.feedback_start_time).count() > 100) {
    return;
  }

  times.feedback_end_time = now;

  this->sample_indices.feedback_idx++;
}
