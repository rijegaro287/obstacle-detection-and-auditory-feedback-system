#pragma once

#include <cstdint>

#include "kfr/all.hpp"
#include "npy.hpp"

#include "kd_tree.hpp"

#define TAP_SIGNAL_PATH "./tap_alert.npy"
#define HRIR_PATH "./dataset/hrirs.npy"
#define POSITION_PATH "./dataset/positions.npy"
#define VERBAL_FEEDBACK_PATH "./dataset/verbal_feedback_signals.npy"

#define NON_VERBAL_SAMPLE_RATE 48000
#define VERBAL_SAMPLE_RATE 22050

#define TAP_N_SAMPLES 48000
#define HRIR_N_TAPS 256

enum HRIR_CHANNELS {
	LEFT_CHANNEL,
	RIGHT_CHANNEL
};

enum POSITION_CHANNELS {
	AZIMUTH_POSITION,
	ELEVATION_POSITION,
	DISTANCE_POSITION
};

enum VERBAL_FEEDBACK_POSITIONS {
  FRONT,
  ABOVE,
  BELOW,
  RIGHT,
  LEFT,
  ABOVE_RIGHT,
  ABOVE_LEFT,
  BELOW_RIGHT,
  BELOW_LEFT
};

using namespace std;
using namespace kfr;
using namespace npy;

class auditory_feedback_module {
public:
  static auditory_feedback_module& get_instance();
  auditory_feedback_module(const auditory_feedback_module&) = delete;
  auditory_feedback_module& operator=(const auditory_feedback_module&) = delete;
  auditory_feedback_module(auditory_feedback_module&&) = delete;
  auditory_feedback_module& operator=(auditory_feedback_module&&) = delete;
  
  void start();
private:
  univector<double, TAP_N_SAMPLES> tap_signal;
  tensor<double, 3> hrir_tensor;
  kd_tree<3> position_tree;
  tensor<double, 2> verbal_feedback_tensor;

  auditory_feedback_module();
  ~auditory_feedback_module() = default;

  void init_tap_signal();
  void init_hrir_tensor();
  void init_position_tree();
  void init_verbal_feedback_tensor();
  univector<double, HRIR_N_TAPS> make_hrir_univector(uint64_t sample, uint64_t channel);
  void generate_feedback(uint64_t sample_idx);
};
