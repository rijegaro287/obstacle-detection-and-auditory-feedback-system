#pragma once

/**
 * @file control_module.hpp
 * @brief Central coordinator that synchronizes data exchange between all
 * processing modules and exposes shared state via @ref IControl.
 */

/**
 * @defgroup control_module Control Module
 * @brief Shared state manager orchestrating capture, detection, feedback, and transmission.
 * @{
 */

#include "control_iface.hpp"
#include "performance_monitor.hpp"

#include <vector>
#include <mutex>
#include <opencv2/opencv.hpp>

using namespace std;

/**
 * @class ControlModule
 * @ingroup control_module
 * @brief Thread-safe singleton that owns the shared pipeline state and issues
 * start/stop commands to the capture, detection, feedback, and transmission
 * modules.
 */
class ControlModule {
public:
	ControlModule(const ControlModule&) = delete;
	ControlModule& operator=(const ControlModule&) = delete;
	ControlModule(ControlModule&&) = delete;
	ControlModule& operator=(ControlModule&&) = delete;

	/**
	 * @brief Retrieve the singleton instance backing the @ref IControl façade.
	 * @return Reference to the control module.
	 */
	static ControlModule& get_instance();

	/**
	 * @brief Fetch and clear the latest captured frame.
	 * @return Copy of the frame data; depth/image matrices will be empty when no
	 * frame is pending.
	 */
	Frame get_frame();

	/**
	 * @brief Publish a frame produced by the capture module.
	 * @param frame Frame data ready for consumption by downstream stages.
	 */
	void set_frame(const Frame frame);
  
	/**
	 * @brief Fetch and clear the latest detected obstacle.
	 * @return Copy of the obstacle descriptor; `meanDepth` equals zero when
	 * no obstacle is available.
	 */
	Obstacle get_obstacle();

	/**
	 * @brief Publish an obstacle descriptor computed by the detection module.
	 * @param obstacle Detailed description of the chosen obstacle.
	 */
	void set_obstacle(const Obstacle obstacle);
  
	/**
	 * @brief Fetch and clear the latest feedback audio buffers.
	 * @return Copy of the stereo audio payload.
	 */
	Audio get_audio_data();

	/**
	 * @brief Publish audio buffers prepared for transmission.
	 * @param data Stereo audio buffer together with metadata.
	 */
	void set_audio_data(const Audio& data);
  
	/**
	 * @brief Issue start commands to all runtime modules in the feedback chain.
	 */
	void start_feedback();

	/**
	 * @brief Issue stop commands to all runtime modules and clear shared state.
	 */
	void stop_feedback();

	/**
	 * @brief Update the playback volume delegated to the feedback subsystem.
	 * @param volume Volume percentage in the range [0, 100].
	 */
	void set_volume(uint64_t volume);

	/**
	 * @brief Change the audio feedback modality.
	 * @param mode Requested feedback mode.
	 */
	void set_feedback_mode(FEEDBACK_MODES mode);
  
	/**
	 * @brief Record whether audio commands were recently received. Used to
	 * automatically stop feedback when commands cease.
	 * @param status True when commands were seen during the last cycle.
	 */
	void set_received_audio_commands(bool status);

	/**
	 * @brief Release the mutexes guarding shared state, enabling producers to
	 * write once initialization is complete.
	 */
	void unlock_mutexes();

	/**
	 * @brief Execute the control module loop that enforces safety timeouts.
	 */
	void start();
private:
	bool received_commands; /**< Tracks whether commands arrived recently. */

	Frame frame;            /**< Latest captured frame. */
	Obstacle obstacle;      /**< Latest detected obstacle. */
	Audio audio_data;       /**< Latest generated audio buffers. */

	mutex frame_mtx;        /**< Guards access to @ref frame. */
	mutex obstacle_mtx;     /**< Guards access to @ref obstacle. */
	mutex audio_mtx;        /**< Guards access to @ref audio_data. */

	ControlModule();
	~ControlModule();
};

/// @}
