#include "transmission_module.hpp"

#include <chrono>
#include <thread>
#include <math.h>

#define AUDIO_CHUNK_N_SAMPLES 1024
#define OUTPUT_CHUNK_N_SAMPLES 2*AUDIO_CHUNK_N_SAMPLES
#define PCM_LATENCY 0

TransmissionModule& TransmissionModule::get_instance() {
	static TransmissionModule instance;
	return instance;
}

TransmissionModule::TransmissionModule() {
	if (snd_pcm_open(&this->pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
		printf("Error opening PCM device\n");
		return;
	}
}

TransmissionModule::~TransmissionModule() {
	if (pcm_handle) {
		snd_pcm_drain(pcm_handle);
		snd_pcm_close(pcm_handle);
	}
}

void TransmissionModule::convert_to_pcm(const vector<double>& interleaved, 
																				vector<int16_t>& pcm,
																				double max_value) {
	if (interleaved.size() % 2 != 0) {
		printf("Error: Interleaved buffer size is not valid.\n");
		return;
	}
	
	if (pcm.size() != interleaved.size()) {
		printf("Error: PCM buffer size is not valid.\n");
		return;
	}
	
	double scale = INT16_MAX / max_value;
	for (uint64_t i = 0; i < interleaved.size(); i++) {
		pcm[i] = static_cast<int16_t>(interleaved[i] * scale);
		if (pcm[i] > INT16_MAX) pcm[i] = INT16_MAX;
		if (pcm[i] < INT16_MIN) pcm[i] = INT16_MIN;
	}
}

void TransmissionModule::interleave_audio(const vector<double>& left_channel, 
																					const vector<double>& right_channel,
																					vector<double>& interleaved) {
	if (left_channel.size() != right_channel.size()) {
		printf("Error: Left and right channel sizes do not match.\n");
		return;
	}

	if (interleaved.size() != left_channel.size() * 2) {
		printf("Error: Interleaved buffer size should be %zu.\n", left_channel.size() * 2);
		return;
	}

	for (uint64_t i = 0; i < left_channel.size(); i++) {
		interleaved[2 * i] = left_channel[i];
		interleaved[(2 * i) + 1] = right_channel[i];
	}
}	

double get_max_value(const vector<double>& audio) {
	double max_value = 0.0;
	for (const auto& sample : audio) {
		if (fabs(sample) > max_value) {
			max_value = fabs(sample);
		}
	}
	return max_value;
}

void TransmissionModule::preprocess_audio(audio_data_t& signal, 
																				  vector<int16_t>& pcm, 
																					double max_value,
																					uint64_t start_idx,
																					uint64_t end_idx) {
		vector<double> left_chunk(signal.left_signal.begin() + start_idx,
															signal.left_signal.begin() + end_idx);
		vector<double> right_chunk(signal.right_signal.begin() + start_idx,
															signal.right_signal.begin() + end_idx);
		vector<double> interleaved_chunk(2 * left_chunk.size());

		interleave_audio(left_chunk, right_chunk, interleaved_chunk);
		convert_to_pcm(interleaved_chunk, pcm, max_value);
}

void TransmissionModule::send_pcm_data(vector<int16_t>& pcm, uint64_t sample_rate) {
	if (pcm.empty() || sample_rate == 0) {
		printf("Error: PCM data is invalid.\n");
		return;
	}

	int64_t error = snd_pcm_set_params(
		this->pcm_handle,
		SND_PCM_FORMAT_S16_LE,
		SND_PCM_ACCESS_RW_INTERLEAVED,
		2, 
		sample_rate, 
		1,
		PCM_LATENCY
	);

	if (error < 0) {
		printf("Error setting PCM parameters: %s\n", snd_strerror(error));
		return;
	}

	error = snd_pcm_writei(this->pcm_handle, pcm.data(), pcm.size()/2);
	if (error == -EPIPE) {
		snd_pcm_prepare(this->pcm_handle);
	} 
	else if (error < 0) {
		printf("Error writing to PCM device: %s\n", snd_strerror(error));
	}
}

void TransmissionModule::send_audio(audio_data_t& signal) {
	if (signal.left_signal.size() != signal.right_signal.size()) {
		printf("Error: Left and right channel sizes do not match.\n");
		return;
	}

	printf("Sending audio...\n");
  double max_value = max(get_max_value(signal.left_signal),
												 get_max_value(signal.right_signal));

	uint64_t signal_size = signal.left_signal.size();
	vector<int16_t> processed(2 * signal_size);

	uint64_t n_chunks = (signal_size + AUDIO_CHUNK_N_SAMPLES - 1) / AUDIO_CHUNK_N_SAMPLES;
	for (uint64_t chunk_idx = 0; chunk_idx < n_chunks; chunk_idx++) {
		uint64_t start_idx = chunk_idx * AUDIO_CHUNK_N_SAMPLES;
		uint64_t end_idx = min(start_idx + AUDIO_CHUNK_N_SAMPLES, signal_size);

		vector<int16_t> processed_chunk(2 * (end_idx - start_idx));
		this->preprocess_audio(signal, processed_chunk, max_value, start_idx, end_idx);
		this->send_pcm_data(processed_chunk, signal.sample_rate);
	}
}

void TransmissionModule::start() {
	while (true) {
		audio_data_t signal = IControl::get_audio_data();
			if (signal.left_signal.empty() ||
					signal.right_signal.empty() ||
					signal.sample_rate == 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			continue;
		}
		send_audio(signal);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}
