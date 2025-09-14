#pragma once

#include "control_iface.hpp"

#include <iostream>
#include <vector>

#include <alsa/asoundlib.h>

using namespace std;

class TransmissionModule {
public:
  TransmissionModule(const TransmissionModule&) = delete;
  TransmissionModule& operator=(const TransmissionModule&) = delete;
  TransmissionModule(TransmissionModule&&) = delete;
  TransmissionModule& operator=(TransmissionModule&&) = delete;

  static TransmissionModule& get_instance();

	void start();
private:
	snd_pcm_t *pcm_handle;

  TransmissionModule();
  ~TransmissionModule();

	void convert_to_pcm(const vector<float>& interleaved, vector<int16_t>& pcm, float max_value);
	void interleave_audio(const vector<float>& left_channel, const vector<float>& right_channel, vector<float>& interleaved);
	void preprocess_audio(Audio& signal, vector<int16_t>& pcm, float max_value, uint64_t start_idx, uint64_t end_idx);
	void send_pcm_data(vector<int16_t>& pcm, uint64_t sample_rate);
	void send_audio(Audio& signal);
};
