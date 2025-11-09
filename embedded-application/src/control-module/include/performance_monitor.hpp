#pragma once

/**
 * @file performance_monitor.hpp
 * @brief Utilities to gather latency metrics for each stage in the processing
 * pipeline and report aggregated statistics.
 */

#include <cstdint>
#include <chrono>

/** @brief Maximum number of samples collected per repetition. */
#define N_SAMPLES 1000
/** @brief Number of repetitions captured before computing statistics. */
#define N_REPEATS 10

/** @brief Alias for time points gathered from the steady clock. */
typedef std::chrono::steady_clock::time_point TimePoint;

/**
 * @struct SampleIndices
 * @brief Current index markers for each pipeline stage while gathering
 * performance samples.
 */
typedef struct SampleIndices_ {
	uint64_t capture_idx;   /**< Index into capture samples array. */
	uint64_t detection_idx; /**< Index into detection samples array. */
	uint64_t feedback_idx;  /**< Index into feedback samples array. */
} SampleIndices;

/**
 * @struct ProcessingTimes
 * @brief Raw timestamps collected for each stage during a single sample.
 */
typedef struct ProcessingTimes_ {
	TimePoint capture_start_time;   /**< Timestamp when capture started. */
	TimePoint capture_end_time;     /**< Timestamp when capture ended. */

	TimePoint detection_start_time; /**< Timestamp when detection started. */
	TimePoint detection_end_time;   /**< Timestamp when detection ended. */

	TimePoint feedback_start_time;  /**< Timestamp when feedback generation started. */
	TimePoint feedback_end_time;    /**< Timestamp when feedback generation ended. */
} ProcessingTimes;

/**
 * @struct TotalTimes
 * @brief Millisecond durations derived from @ref ProcessingTimes for each
 * pipeline stage.
 */
typedef struct TotalTimes_ {
	uint64_t total_time_ms[N_SAMPLES];    /**< Full pipeline latency per sample. */
	uint64_t capture_times_ms[N_SAMPLES]; /**< Capture stage latency per sample. */
	uint64_t detection_times_ms[N_SAMPLES];/**< Detection stage latency per sample. */
	uint64_t feedback_times_ms[N_SAMPLES]; /**< Feedback stage latency per sample. */
} TotalTimes;

/**
 * @struct PerformanceStats
 * @brief Aggregated mean and standard deviation for each stage's timings.
 */
typedef struct PerformanceStats_ {
	double total_avg_ms;      /**< Average full pipeline latency. */
	double total_stddev_ms;   /**< Standard deviation of full pipeline latency. */

	double capture_avg_ms;    /**< Average capture latency. */
	double capture_stddev_ms; /**< Standard deviation of capture latency. */

	double detection_avg_ms;  /**< Average detection latency. */
	double detection_stddev_ms;/**< Standard deviation of detection latency. */

	double feedback_avg_ms;   /**< Average feedback latency. */
	double feedback_stddev_ms;/**< Standard deviation of feedback latency. */
} PerformanceStats;

/**
 * @class PerformanceMonitor
 * @brief Singleton that records execution timestamps and produces statistical
 * reports for performance validation.
 */
class PerformanceMonitor {
public:
	PerformanceMonitor(const PerformanceMonitor&) = delete;
	PerformanceMonitor& operator=(const PerformanceMonitor&) = delete;
	PerformanceMonitor(PerformanceMonitor&&) = delete;
	PerformanceMonitor& operator=(PerformanceMonitor&&) = delete;

	/**
	 * @brief Retrieve the monitor singleton.
	 * @return Reference to the performance monitor.
	 */
	static PerformanceMonitor& get_instance();

	/**
	 * @brief Enable or disable performance monitoring.
	 * @param enabled True to start capturing timings, false to ignore samples.
	 */
	void set_performance_monitoring(bool enabled);

	/**
	 * @brief Aggregate captured data into the statistics buffer.
	 * @return True when sufficient samples have been collected to finalize a
	 * repetition.
	 */
	bool record_performance_stats();

	/**
	 * @brief Print the aggregated statistics to stdout.
	 */
	void print_performance_stats();

	/** @brief Mark the start of a capture sample. */
	void add_capture_sample_start();
	/** @brief Mark the end of a capture sample. */
	void add_capture_sample_end();

	/** @brief Mark the start of a detection sample. */
	void add_detection_sample_start();
	/** @brief Mark the end of a detection sample. */
	void add_detection_sample_end();

	/** @brief Mark the start of a feedback sample. */
	void add_feedback_sample_start();
	/** @brief Mark the end of a feedback sample. */
	void add_feedback_sample_end();
private:
	bool performance_monitoring_enabled; /**< Flag gating the sampling logic. */

	uint64_t repetition_count;           /**< Number of repetitions finalized. */
	SampleIndices sample_indices;        /**< Current indices for each stage. */
	ProcessingTimes processing_times[N_SAMPLES]; /**< Raw timestamps per sample. */
	PerformanceStats performance_stats[N_REPEATS]; /**< Aggregated stats buffer. */

	/**
	 * @brief Compute the standard deviation for the provided values.
	 * @param values Array containing millisecond durations.
	 * @param mean Previously calculated mean value.
	 * @return Standard deviation of the supplied samples.
	 */
	double compute_stddev(uint64_t values[], double mean);

	/**
	 * @brief Convert raw timestamps into millisecond durations.
	 * @return Struct containing the computed durations for each stage.
	 */
	TotalTimes map_total_times();

	/**
	 * @brief Produce aggregated statistics from the recorded samples.
	 * @return Struct containing means and standard deviations for each stage.
	 */
	PerformanceStats map_performance_stats();

	PerformanceMonitor();
	~PerformanceMonitor();
};
