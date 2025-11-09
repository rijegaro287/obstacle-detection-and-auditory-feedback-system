#pragma once

/**
 * @file feedback_module.hpp
 * @brief Spatial audio renderer that transforms obstacle detections into
 * stereo feedback, supporting both verbal and non-verbal modalities.
 */

/**
 * @defgroup feedback_module Feedback Module
 * @brief Generates spatialized audio cues from detected obstacles.
 * @{
 */

#include "control_iface.hpp"
#include "feedback_iface.hpp"

#include <cstdint>
#include "kfr/all.hpp"
#include "npy.hpp"

#include "kd_tree.hpp"

/** @brief Dataset path for the tap-alert waveform. */
#define TAP_SIGNAL_PATH "./src/feedback-module/dataset/tap_alert.npy"
/** @brief Dataset path for head-related impulse responses. */
#define HRIR_PATH "./src/feedback-module/dataset/hrirs.npy"
/** @brief Dataset path for spatial position samples. */
#define POSITION_PATH "./src/feedback-module/dataset/positions.npy"
/** @brief Dataset path for pre-rendered verbal feedback signals. */
#define VERBAL_FEEDBACK_PATH "./src/feedback-module/dataset/verbal_feedback_signals.npy"

/** @brief Sample rate used for the non-verbal feedback pipeline. */
#define NON_VERBAL_SAMPLE_RATE 48000
/** @brief Sample rate used for the verbal feedback pipeline. */
#define VERBAL_SAMPLE_RATE 48000

/** @brief Number of samples in the tap signal dataset. */
#define TAP_N_SAMPLES 48000
/** @brief Number of taps per HRIR in the dataset. */
#define HRIR_N_TAPS 256

/** @brief Horizontal field-of-view (degrees) of the ToF sensor. */
#define TOF_AZ_FOV 63
/** @brief Vertical field-of-view (degrees) of the ToF sensor. */
#define TOF_EL_FOV 42

/** @brief Horizontal threshold to decide verbal cues. */
#define VERBAL_AZIMUTH_THRESHOLD TOF_AZ_FOV/3
/** @brief Vertical threshold to decide verbal cues. */
#define VERBAL_ELEVATION_THRESHOLD TOF_EL_FOV/3

/**
 * @enum HRIR_CHANNELS
 * @brief Indexes describing the channel layout inside the HRIR dataset.
 */
enum HRIR_CHANNELS {
	LEFT_CHANNEL,  /**< Left ear impulse response channel. */
	RIGHT_CHANNEL  /**< Right ear impulse response channel. */
};

/**
 * @enum POSITION_CHANNELS
 * @brief Indexes used to extract azimuth, elevation, and distance from the
 * `positions.npy` dataset.
 */
enum POSITION_CHANNELS {
	AZIMUTH_POSITION,   /**< Azimuth coordinate index. */
	ELEVATION_POSITION, /**< Elevation coordinate index. */
	DISTANCE_POSITION   /**< Distance coordinate index. */
};

/**
 * @enum VERBAL_FEEDBACK_MASKS
 * @brief Bitmask that describes discrete spatial quadrants for mapping verbal
 * prompts.
 */
enum VERBAL_FEEDBACK_MASKS {
	HORIZONTALLY_CENTERED_MASK = 0b000001, /**< Obstacle centered horizontally. */
	RIGHT_MASK               = 0b000010,   /**< Obstacle located to the right. */
	LEFT_MASK                = 0b000100,   /**< Obstacle located to the left. */
	VERTICALLY_CENTERED_MASK = 0b001000,   /**< Obstacle centered vertically. */
	ABOVE_MASK               = 0b010000,   /**< Obstacle above the horizontal plane. */
	BELOW_MASK               = 0b100000,   /**< Obstacle below the horizontal plane. */
};

/**
 * @enum VERBAL_FEEDBACK_IDX
 * @brief Discrete index used to retrieve the appropriate verbal waveform.
 */
enum VERBAL_FEEDBACK_IDX {
	FRONT,        /**< Obstacle directly ahead. */
	ABOVE,        /**< Obstacle directly above. */
	BELOW,        /**< Obstacle directly below. */
	RIGHT,        /**< Obstacle on the right. */
	LEFT,         /**< Obstacle on the left. */
	ABOVE_RIGHT,  /**< Obstacle above and to the right. */
	ABOVE_LEFT,   /**< Obstacle above and to the left. */
	BELOW_RIGHT,  /**< Obstacle below and to the right. */
	BELOW_LEFT    /**< Obstacle below and to the left. */
};

using namespace std;
using namespace npy;

/**
 * @class FeedbackModule
 * @ingroup feedback_module
 * @brief Singleton that consumes obstacle detections to generate stereo audio
 * feedback, leveraging HRIR convolution for non-verbal cues and pre-recorded
 * phrases for verbal guidance.
 */
class FeedbackModule {
public:
	FeedbackModule(const FeedbackModule&) = delete;
	FeedbackModule& operator=(const FeedbackModule&) = delete;
	FeedbackModule(FeedbackModule&&) = delete;
	FeedbackModule& operator=(FeedbackModule&&) = delete;

	/**
	 * @brief Retrieve the singleton feedback module instance.
	 * @return Reference to the feedback module.
	 */
	static FeedbackModule& get_instance();

	/** @brief Flag the feedback engine as active. */
	void start_feedback();
	/** @brief Flag the feedback engine as inactive. */
	void stop_feedback();

	/**
	 * @brief Adjust the playback volume applied to generated signals.
	 * @param volume Percentage in the range [0, 100].
	 */
	void set_volume(uint64_t volume);

	/**
	 * @brief Switch between verbal and non-verbal feedback strategies.
	 * @param mode Desired feedback mode.
	 */
	void set_feedback_mode(FEEDBACK_MODES mode);

	/**
	 * @brief Continuous worker loop that, when running, fetches obstacles and
	 * pushes audio buffers to the shared control state.
	 */
	void start();
private:
	bool running;                 /**< Indicates whether audio generation is active. */
	float volume;                 /**< Normalized gain (0.0-1.0). */
	FEEDBACK_MODES feedback_mode; /**< Currently selected feedback modality. */

	kfr::univector<float, TAP_N_SAMPLES> tap_signal; /**< Non-verbal base waveform. */
	kfr::tensor<float, 2> verbal_feedback_tensor;    /**< Pre-rendered verbal clips. */
  
	kfr::tensor<float, 3> hrir_tensor; /**< HRIR dataset arranged as [sample][tap][channel]. */
	kd_tree<3> position_tree;          /**< Spatial search structure to match HRIR samples. */

	FeedbackModule();
	~FeedbackModule() = default;

	/** @brief Load the tap waveform into memory. */
	void init_tap_signal();
	/** @brief Load the HRIR dataset into a tensor. */
	void init_hrir_tensor();
	/** @brief Populate the KD-tree with spatial HRIR sample locations. */
	void init_position_tree();
	/** @brief Load the verbal feedback tensor. */
	void init_verbal_feedback_tensor();

	/**
	 * @brief Convert a slice of the HRIR tensor into a univector convenience type.
	 * @param sample Index of the HRIR sample.
	 * @param channel Channel index from @ref HRIR_CHANNELS.
	 * @return Univector representing the requested HRIR.
	 */
	kfr::univector<float, HRIR_N_TAPS> make_hrir_univector(uint64_t sample, uint64_t channel);

	/**
	 * @brief Map an obstacle to the discrete verbal feedback position mask.
	 * @param obstacle Obstacle descriptor containing angles.
	 * @return Bitmask used to index @ref VERBAL_FEEDBACK_IDX.
	 */
	uint8_t calculate_verbal_position(Obstacle obstacle);

	/**
	 * @brief Generate stereo audio by convolving the tap signal with the matched
	 * HRIR pair.
	 * @param obstacle Obstacle descriptor informing azimuth/elevation/distance.
	 * @return Stereo audio buffer ready for transmission.
	 */
	Audio generate_non_verbal_feedback(Obstacle obstacle);

	/**
	 * @brief Select and scale a pre-recorded verbal feedback clip.
	 * @param obstacle Obstacle descriptor informing azimuth/elevation/distance.
	 * @return Stereo audio buffer ready for transmission.
	 */
	Audio generate_verbal_feedback(Obstacle obstacle);

	/**
	 * @brief Dispatch to the appropriate feedback generation strategy.
	 * @param obstacle Obstacle descriptor informing audio rendering.
	 * @return Stereo audio buffer ready for transmission.
	 */
	Audio generate_feedback(Obstacle obstacle);
};

/// @}
