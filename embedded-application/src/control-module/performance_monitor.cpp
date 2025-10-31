#include "performance_monitor.hpp"

#include <math.h>
#include <stdio.h>

PerformanceMonitor& PerformanceMonitor::get_instance() {
	static PerformanceMonitor instance;
	return instance;
}

PerformanceMonitor::PerformanceMonitor() {
  this->performance_monitoring_enabled = false;
  this->sample_indices.capture_idx = 0;
  this->sample_indices.detection_idx = 0;
  this->sample_indices.feedback_idx = 0;
}

PerformanceMonitor::~PerformanceMonitor() {

}

bool PerformanceMonitor::print_performance_stats() {
  if (this->sample_indices.capture_idx == N_SAMPLES &&
      this->sample_indices.detection_idx == N_SAMPLES &&
      this->sample_indices.feedback_idx == N_SAMPLES
  ) {
    PerformanceStats stats = this->map_performance_stats();
    printf("==================== Performance Statistics ====================\n");
    printf("Capture - Avg: %.2f ms, Stddev: %.2f ms\n", stats.capture_avg_ms, stats.capture_stddev_ms);
    printf("Detection - Avg: %.2f ms, Stddev: %.2f ms\n", stats.detection_avg_ms, stats.detection_stddev_ms);
    printf("Feedback - Avg: %.2f ms, Stddev: %.2f ms\n", stats.feedback_avg_ms, stats.feedback_stddev_ms);
    return true;
  }
  return false;
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
    total_times.total_time_ms[i] = std::chrono::duration_cast<std::chrono::milliseconds>(times.transmission_start_time - times.capture_start_time).count();
    total_times.capture_times_ms[i] = std::chrono::duration_cast<std::chrono::milliseconds>(times.capture_end_time - times.capture_start_time).count();
    total_times.detection_times_ms[i] = std::chrono::duration_cast<std::chrono::milliseconds>(times.detection_end_time - times.detection_start_time).count();
    total_times.feedback_times_ms[i] = std::chrono::duration_cast<std::chrono::milliseconds>(times.feedback_end_time - times.feedback_start_time).count();
  }
  return total_times;
}

PerformanceStats PerformanceMonitor::map_performance_stats() {
  TotalTimes total_times = this->map_total_times();
  PerformanceStats stats = PerformanceStats();
  for (uint64_t i = 0; i < N_SAMPLES; i++) {
    stats.capture_avg_ms += (double)total_times.capture_times_ms[i];
    stats.detection_avg_ms += (double)total_times.detection_times_ms[i];
    stats.feedback_avg_ms += (double)total_times.feedback_times_ms[i];
  }

  stats.capture_avg_ms /= (double)N_SAMPLES;
  stats.detection_avg_ms /= (double)N_SAMPLES;
  stats.feedback_avg_ms /= (double)N_SAMPLES;

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
