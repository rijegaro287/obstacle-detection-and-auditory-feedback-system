#pragma once

#include <iostream>
#include <vector>

#include <alsa/asoundlib.h>

#include "control_iface.hpp"

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

	void convert_to_pcm(const vector<double>& interleaved, vector<int16_t>& pcm, double max_value);
	void interleave_audio(const vector<double>& left_channel, const vector<double>& right_channel, vector<double>& interleaved);
	void preprocess_audio(audio_data_t& signal, vector<int16_t>& pcm, double max_value, uint64_t start_idx, uint64_t end_idx);
	void send_pcm_data(vector<int16_t>& pcm, uint64_t sample_rate);
	void send_audio(audio_data_t& signal);
};
