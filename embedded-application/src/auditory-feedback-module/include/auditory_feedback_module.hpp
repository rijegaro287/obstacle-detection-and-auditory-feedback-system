#pragma once

#include <iostream>
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

#define TOF_AZ_FOV 56
#define TOF_EL_FOV 42

#define VERBAL_AZIMUTH_THRESHOLD TOF_AZ_FOV/3
#define VERBAL_ELEVATION_THRESHOLD TOF_EL_FOV/3

enum FEEDBACK_MODES {
  NON_VERBAL_MODE,
  VERBAL_MODE
};

enum HRIR_CHANNELS {
	LEFT_CHANNEL,
	RIGHT_CHANNEL
};

enum POSITION_CHANNELS {
	AZIMUTH_POSITION,
	ELEVATION_POSITION,
	DISTANCE_POSITION
};

enum VERBAL_FEEDBACK_MASKS {
  HORIZONTAL_CENTERED_MASK = 0b000001,
  RIGHT_MASK               = 0b000010,
  LEFT_MASK                = 0b000100,
  VERTICALLY_CENTERED_MASK = 0b001000,
  ABOVE_MASK               = 0b010000,
  BELOW_MASK               = 0b100000,
};

enum VERBAL_FEEDBACK_IDX {
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
using namespace npy;

class auditory_feedback_module {
public:
  static auditory_feedback_module& get_instance();
  auditory_feedback_module(const auditory_feedback_module&) = delete;
  auditory_feedback_module& operator=(const auditory_feedback_module&) = delete;
  auditory_feedback_module(auditory_feedback_module&&) = delete;
  auditory_feedback_module& operator=(auditory_feedback_module&&) = delete;
  void set_feedback_mode(FEEDBACK_MODES mode);
  void start();
private:
  FEEDBACK_MODES feedback_mode;
  kfr::univector<double, TAP_N_SAMPLES> tap_signal;
  kfr::tensor<double, 3> hrir_tensor;
  kd_tree<3> position_tree;
  kfr::tensor<double, 2> verbal_feedback_tensor;

  auditory_feedback_module();
  ~auditory_feedback_module() = default;

  void init_tap_signal();
  void init_hrir_tensor();
  void init_position_tree();
  void init_verbal_feedback_tensor();
  kfr::univector<double, HRIR_N_TAPS> make_hrir_univector(uint64_t sample, uint64_t channel);
  uint8_t calculate_verbal_position(float azimuth, float elevation, float distance);
  void generate_non_verbal_feedback(float azimuth, float elevation, float distance);
  void generate_verbal_feedback(float azimuth, float elevation, float distance);
  void generate_feedback(float azimuth, float elevation, float distance);
};
