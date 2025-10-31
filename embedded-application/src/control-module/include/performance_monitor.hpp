#pragma once

#include <cstdint>
#include <chrono>

#define N_SAMPLES 100

typedef std::chrono::steady_clock::time_point TimePoint;

typedef struct SampleIndices_ {
  uint64_t capture_idx;
  uint64_t detection_idx;
  uint64_t feedback_idx;
  uint64_t transmission_idx;
} SampleIndices;

typedef struct ProcessingTimes_ {
  TimePoint capture_start_time;
  TimePoint capture_end_time;

  TimePoint detection_start_time;
  TimePoint detection_end_time;

  TimePoint feedback_start_time;
  TimePoint feedback_end_time;

  TimePoint transmission_start_time;
  TimePoint transmission_end_time;
} ProcessingTimes;

typedef struct PerformanceStats_ {
  double capture_avg_ms;
  double capture_stddev_ms;

  double detection_avg_ms;
  double detection_stddev_ms;

  double feedback_avg_ms;
  double feedback_stddev_ms;

  double transmission_avg_ms;
  double transmission_stddev_ms;
} PerformanceStats;

class PerformanceMonitor {
public:
  PerformanceMonitor(const PerformanceMonitor&) = delete;
  PerformanceMonitor& operator=(const PerformanceMonitor&) = delete;
  PerformanceMonitor(PerformanceMonitor&&) = delete;
  PerformanceMonitor& operator=(PerformanceMonitor&&) = delete;

  static PerformanceMonitor& get_instance(bool enabled = false);

  void add_capture_sample_start();
  void add_capture_sample_end();

  void add_detection_sample_start();
  void add_detection_sample_end();

  void add_feedback_sample_start();
  void add_feedback_sample_end();

  void add_transmission_sample_start();
  void add_transmission_sample_end(uint64_t signal_duration_ms);

private:
  bool performance_monitoring_enabled;

  SampleIndices sample_indices;
  ProcessingTimes processing_times[N_SAMPLES];

  PerformanceMonitor(bool enabled);
  ~PerformanceMonitor();
};
