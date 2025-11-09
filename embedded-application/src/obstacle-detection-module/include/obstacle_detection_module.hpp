#pragma once

/**
 * @file obstacle_detection_module.hpp
 * @brief Detects and characterizes obstacles within pre-processed camera
 * frames.
 */

/**
 * @defgroup obstacle_detection_module Obstacle Detection Module
 * @brief Segments and scores obstacles from pre-processed frames.
 * @{
 */

#include <opencv2/opencv.hpp>
#include "control_iface.hpp"

/**
 * @class ObstacleDetectionModule
 * @ingroup obstacle_detection_module
 * @brief Singleton responsible for running the vision pipeline that segments
 * obstacles, filters them, and publishes the most relevant instance.
 */
class ObstacleDetectionModule {
public:
	ObstacleDetectionModule(const ObstacleDetectionModule&) = delete;
	ObstacleDetectionModule& operator=(const ObstacleDetectionModule&) = delete;
	ObstacleDetectionModule(ObstacleDetectionModule&&) = delete;
	ObstacleDetectionModule& operator=(ObstacleDetectionModule&&) = delete;

	/**
	 * @brief Retrieve the obstacle detection singleton.
	 * @return Reference to the detection module.
	 */
	static ObstacleDetectionModule& get_instance();

	/**
	 * @brief Display the depth image for debugging purposes.
	 * @param depthImage Depth map to visualize.
	 */
	void previewDepth(cv::Mat& depthImage);

	/**
	 * @brief Display the selected obstacle overlay for debugging.
	 * @param obstacle Obstacle descriptor containing visualization data.
	 */
	void viewDetection(Obstacle& obstacle);

	/**
	 * @brief Run the detection pipeline on a frame and associated depth map.
	 * @param image Pre-processed RGB visualization.
	 * @param depthMap Corresponding depth map.
	 * @return Obstacle descriptor highlighting the most relevant obstacle.
	 */
	Obstacle detect(cv::Mat& image, cv::Mat& depthMap);

	/** @brief Mark the detection loop as active. */
	void start_detection();

	/** @brief Pause the detection loop. */
	void stop_detection();
    
	/**
	 * @brief Worker thread entry point that processes frames from the control
	 * module and publishes obstacle descriptors.
	 */
	void start();

private:
	bool running; /**< Indicates whether detection is active. */

	/**
	 * @brief Collection of intermediate matrices describing connected
	 * components within the segmented mask.
	 */
	struct Components {
		cv::Mat labels;     /**< Connected-component labels. */
		cv::Mat stats;      /**< Statistics for each component. */
		cv::Mat centroids;  /**< Centroids for each component. */
		cv::Mat image;      /**< Colored mask visualization. */
	};
    
	/** @brief HSV lower bound for the primary red detection range. */
	cv::Scalar lowerRed1_;
	/** @brief HSV upper bound for the primary red detection range. */
	cv::Scalar upperRed1_;
	/** @brief HSV lower bound for the secondary red detection range. */
	cv::Scalar lowerRed2_;
	/** @brief HSV upper bound for the secondary red detection range. */
	cv::Scalar upperRed2_;
	/** @brief HSV lower bound for orange detection. */
	cv::Scalar lowerOrange_;
	/** @brief HSV upper bound for orange detection. */
	cv::Scalar upperOrange_;

	/** @brief Horizontal field-of-view in degrees. */
	static constexpr double FOV_X_DEG = 62.8;
	/** @brief Vertical field-of-view in degrees. */
	static constexpr double FOV_Y_DEG = 37.9;

	/**
	 * @brief Segment red and orange hues from the input image.
	 * @param image Pre-processed RGB visualization frame.
	 * @return Binary mask highlighting pixels within the configured colour ranges.
	 */
	cv::Mat segmentRed(const cv::Mat& image) const;
	/**
	 * @brief Remove mask regions whose depth exceeds the accepted threshold.
	 * @param mask Binary mask produced by @ref segmentRed.
	 * @param depthImage Depth map aligned with the mask.
	 * @param maxDepthThreshold Maximum depth (in millimetres) to keep.
	 * @return Filtered mask containing only nearby pixels.
	 */
	cv::Mat filterByDepth(const cv::Mat& mask, const cv::Mat& depthImage, float maxDepthThreshold = 2000.0f) const;
	/**
	 * @brief Remove small or sparse blobs that do not meet density requirements.
	 * @param mask Binary mask highlighting candidate obstacles.
	 * @param minArea Minimum area (pixels) for a component to survive.
	 * @param minDensity Minimum fill ratio required for a component to survive.
	 * @return Mask containing only dense, sufficiently large blobs.
	 */
	cv::Mat filterByColorDensity(const cv::Mat& mask, double minArea = 700.0, double minDensity = 0.95) const;
	/**
	 * @brief Split the mask into connected components and compute statistics.
	 * @param mask Binary obstacle mask.
	 * @return Struct bundling labels, stats, centroids, and a visualization image.
	 */
	Components divideComponents(const cv::Mat& mask) const;
	/**
	 * @brief Choose the most relevant obstacle based on heuristic scoring.
	 * @param components Connected-component metadata.
	 * @param depthMap Depth map aligned with the components.
	 * @return Descriptor for the selected obstacle.
	 */
	Obstacle selectObstacle(Components& components, const cv::Mat& depthMap) const;
	/**
	 * @brief Enrich an obstacle descriptor with azimuth/elevation angles.
	 * @param obstacle Obstacle descriptor to update.
	 * @return Updated obstacle descriptor containing polar angles.
	 */
	Obstacle calculateAngles(Obstacle& obstacle);
	/**
	 * @brief Map a raw azimuth value to the sensor's field-of-view range.
	 * @param azimuth Raw azimuth in degrees.
	 * @return Normalized azimuth taking into account the sensor geometry.
	 */
	double mapAzimuth(double azimuth);

	/** @brief Construct the detection module with default segmentation ranges. */
	ObstacleDetectionModule();
	~ObstacleDetectionModule() = default;
};

/// @}
