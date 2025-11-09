#pragma once

/**
 * @file image_capture_module.hpp
 * @brief Interface for the module responsible for acquiring and
 * pre-processing depth frames from the Arducam ToF camera.
 */

/**
 * @defgroup image_capture_module Image Capture Module
 * @brief Handles time-of-flight acquisition and frame pre-processing.
 * @{
 */

#include "control_iface.hpp"

#include "ArducamTOFCamera.hpp"
#include <opencv2/core.hpp>

/**
 * @class ImageCaptureModule
 * @ingroup image_capture_module
 * @brief Singleton that manages the time-of-flight camera lifecycle and
 * publishes processed frames to the control module.
 */
class ImageCaptureModule {
public:
	ImageCaptureModule(const ImageCaptureModule&) = delete;
	ImageCaptureModule& operator=(const ImageCaptureModule&) = delete;
	ImageCaptureModule(ImageCaptureModule&&) = delete;
	ImageCaptureModule& operator=(ImageCaptureModule&&) = delete;

	/**
	 * @brief Retrieve the capture module singleton.
	 * @return Reference to the capture module.
	 */
	static ImageCaptureModule& get_instance();

	/**
	 * @brief Initialize the camera connection and configure the desired range.
	 * @return true when initialization succeeds.
	 */
	bool initialize();

	/**
	 * @brief Capture a frame from the ToF camera.
	 * @return true when a valid frame is available.
	 */
	bool captureFrame();

	/**
	 * @brief Pre-process the depth frame into normalized visualization outputs.
	 * @return Struct containing the raw depth map and a colorized representation.
	 */
	Frame preprocessDepth();

	/** @brief Set the capture loop as active. */
	void start_capture();

	/** @brief Pause the capture loop. */
	void stop_capture();

	/**
	 * @brief Worker thread entry point that continuously captures and publishes
	 * frames.
	 */
	void start();

private:
	bool running;               /**< Indicates whether capture is active. */
	bool camera_initialized;    /**< Tracks camera initialization status. */
	Arducam::ArducamTOFCamera tof_;          /**< Camera handle. */
	Arducam::ArducamFrameBuffer* frame_;     /**< Raw frame buffer pointer. */
	cv::Mat depth_frame_;                     /**< Captured depth map. */
	cv::Mat result_frame_;                    /**< Pre-processed visualization frame. */

	ImageCaptureModule();
	~ImageCaptureModule();
};

/// @}
