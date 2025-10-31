#pragma once

#include <cstdint>
#include <chrono>

#define N_SAMPLES 1000
#define N_REPEATS 10

typedef std::chrono::steady_clock::time_point TimePoint;

typedef struct SampleIndices_ {
  uint64_t capture_idx;
  uint64_t detection_idx;
  uint64_t feedback_idx;
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

typedef struct TotalTimes_ {
  uint64_t total_time_ms[N_SAMPLES];
  uint64_t capture_times_ms[N_SAMPLES];
  uint64_t detection_times_ms[N_SAMPLES];
  uint64_t feedback_times_ms[N_SAMPLES];
} TotalTimes;

typedef struct PerformanceStats_ {
  double total_avg_ms;
  double total_stddev_ms;

  double capture_avg_ms;
  double capture_stddev_ms;

  double detection_avg_ms;
  double detection_stddev_ms;

  double feedback_avg_ms;
  double feedback_stddev_ms;
} PerformanceStats;

class PerformanceMonitor {
public:
  PerformanceMonitor(const PerformanceMonitor&) = delete;
  PerformanceMonitor& operator=(const PerformanceMonitor&) = delete;
  PerformanceMonitor(PerformanceMonitor&&) = delete;
  PerformanceMonitor& operator=(PerformanceMonitor&&) = delete;

  static PerformanceMonitor& get_instance();

  void set_performance_monitoring(bool enabled);

  bool record_performance_stats();
  void print_performance_stats();

  void add_capture_sample_start();
  void add_capture_sample_end();

  void add_detection_sample_start();
  void add_detection_sample_end();

  void add_feedback_sample_start();
  void add_feedback_sample_end();
private:
  bool performance_monitoring_enabled;

  uint64_t repetition_count;
  SampleIndices sample_indices;
  ProcessingTimes processing_times[N_SAMPLES];
  PerformanceStats performance_stats[N_REPEATS];

  double compute_stddev(uint64_t values[], double mean);

  TotalTimes map_total_times();
  PerformanceStats map_performance_stats();

  PerformanceMonitor();
  ~PerformanceMonitor();
};
