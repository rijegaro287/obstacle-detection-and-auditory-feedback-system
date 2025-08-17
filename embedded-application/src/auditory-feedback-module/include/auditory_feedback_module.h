#pragma once

#include <stdint.h>

#include "kfr/all.hpp"
#include "npy.hpp"

#define TAP_SIGNAL_PATH "./tap_alert.npy"
#define HRIR_PATH "./dataset/hrirs.npy"
#define POSITION_PATH "./dataset/positions.npy"

#define SAMPLE_RATE 48000
#define TAP_N_SAMPLES 48000
#define HRIR_N_SAMPLES 16020
#define HRIR_N_TAPS 256
#define HRIR_N_CHANNELS 2
#define POSITION_N_CHANNELS 3

enum HRIR_CHANNELS {
	LEFT_CHANNEL,
	RIGHT_CHANNEL
};

enum POSITION_CHANNELS {
	AZIMUTH_POSITION,
	ELEVATION_POSITION,
	DISTANCE_POSITION
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
  
  void generate_feedback(float azimuth, float elevation, float distance);

private:
  univector<double, TAP_N_SAMPLES> tap_signal;
  tensor<double, 3> hrir_tensor;
  tensor<double, 2> position_tensor;
  
  auditory_feedback_module();
  ~auditory_feedback_module() = default;

  void init_hrir_tensor(vector<double>& hrirs_data);
  void init_position_tensor(vector<double> &positions_data);
  univector<double, HRIR_N_TAPS> make_hrir_univector(uint64_t sample, uint64_t channel);
  

  // void processAudio(const kfr::signal<float>& input);
  // void generateFeedback();
};
