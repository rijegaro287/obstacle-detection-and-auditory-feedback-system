#include "transmission_module.hpp"

#include <chrono>
#include <thread>
#include <math.h>

#define AUDIO_CHUNK_N_SAMPLES 256
#define OUTPUT_CHUNK_N_SAMPLES 2*AUDIO_CHUNK_N_SAMPLES
#define PCM_LATENCY 0

TransmissionModule& TransmissionModule::get_instance() {
	static TransmissionModule instance;
	return instance;
}

TransmissionModule::TransmissionModule() {
	int64_t error;

	if (snd_pcm_open(&this->pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
		printf("Error opening 48kHz PCM device\n");
		this->pcm_handle = nullptr;
		return;
	}

	error = snd_pcm_set_params(
		this->pcm_handle,
		SND_PCM_FORMAT_S16_LE,
		SND_PCM_ACCESS_RW_INTERLEAVED,
		2, 
		48000, 
		1,
		PCM_LATENCY
	);

	if (error < 0) {
		printf("Error setting 48kHz PCM parameters: %s\n", snd_strerror(error));
		return;
	}
}

TransmissionModule::~TransmissionModule() {
	if (this->pcm_handle) {
		snd_pcm_drain(pcm_handle);
		snd_pcm_close(pcm_handle);
	}
}

void TransmissionModule::convert_to_pcm(const vector<float>& interleaved, 
																				vector<int16_t>& pcm,
																				float max_value,
																				float gain) {
	if (interleaved.size() % 2 != 0) {
		printf("Error: Interleaved buffer size is not valid.\n");
		return;
	}
	
	if (pcm.size() != interleaved.size()) {
		printf("Error: PCM buffer size is not valid.\n");
		return;
	}
	
	float scale = INT16_MAX / max_value;
	for (uint64_t i = 0; i < interleaved.size(); i++) {
		pcm[i] = static_cast<int16_t>(interleaved[i] * scale * gain);
		if (pcm[i] > INT16_MAX) pcm[i] = INT16_MAX;
		if (pcm[i] < INT16_MIN) pcm[i] = INT16_MIN;
	}
}

void TransmissionModule::interleave_audio(const vector<float>& left_channel, 
																					const vector<float>& right_channel,
																					vector<float>& interleaved) {
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

float get_max_value(const vector<float>& audio) {
	float max_value = 0.0;
	for (const auto& sample : audio) {
		if (fabs(sample) > max_value) {
			max_value = fabs(sample);
		}
	}
	return max_value;
}

void TransmissionModule::preprocess_audio(Audio& signal, 
																				  vector<int16_t>& pcm, 
																					float max_value,
																					uint64_t start_idx,
																					uint64_t end_idx) {
		vector<float> left_chunk(signal.left_signal.begin() + start_idx,
															signal.left_signal.begin() + end_idx);
		vector<float> right_chunk(signal.right_signal.begin() + start_idx,
															signal.right_signal.begin() + end_idx);
		vector<float> interleaved_chunk(2 * left_chunk.size());

		interleave_audio(left_chunk, right_chunk, interleaved_chunk);
		convert_to_pcm(interleaved_chunk, pcm, max_value, signal.gain);
}

void TransmissionModule::send_pcm_data(vector<int16_t>& pcm, uint64_t sample_rate) {
	if (pcm.empty() || sample_rate == 0) {
		printf("Error: PCM data is invalid.\n");
		return;
	}

	if (this->pcm_handle == nullptr) {
		printf("Error: pcm_handle is null\n");
		return;
	}

	int64_t error = snd_pcm_writei(this->pcm_handle, pcm.data(), pcm.size()/2);
	if (error == -EPIPE) {
		snd_pcm_prepare(this->pcm_handle);
	}
	else if (error < 0) {
		printf("Error writing to PCM device: %s\n", snd_strerror(error));
		return;
	}
}

void TransmissionModule::send_audio(Audio& signal) {
	if (signal.left_signal.size() != signal.right_signal.size()) {
		printf("Error: Left and right channel sizes do not match.\n");
		return;
	}

	if (signal.left_signal.empty() || signal.sample_rate == 0) {
		printf("Error: Audio signal is empty or sample rate is invalid.\n");
		return;
	}

	float max_value = std::max(get_max_value(signal.left_signal),
	                           get_max_value(signal.right_signal));

	uint64_t signal_size = signal.left_signal.size();
	const uint64_t chunk_n = AUDIO_CHUNK_N_SAMPLES;

	for (uint64_t start_idx = 0; start_idx < signal_size; start_idx += chunk_n) {
		uint64_t end_idx = start_idx + chunk_n;
		if (end_idx > signal_size) end_idx = signal_size;

		uint64_t current_chunk_samples = end_idx - start_idx;
		if (current_chunk_samples == 0) break;

		vector<int16_t> processed(2 * current_chunk_samples);

		this->preprocess_audio(signal, processed, max_value, start_idx, end_idx);
		this->send_pcm_data(processed, signal.sample_rate);
	}
}

void TransmissionModule::start_transmission() {
	this->running = true;
}

void TransmissionModule::stop_transmission() {
	this->running = false;
}

void TransmissionModule::start() {
	while (true) {
		if (!this->running) {
			std::this_thread::sleep_for(std::chrono::milliseconds(PAUSED_SLEEP_MS));
			continue;
		}

		Audio signal = IControl::get_audio_data();
		if (signal.left_signal.empty() ||
				signal.right_signal.empty() ||
				signal.sample_rate == 0
		) {
			std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_SLEEP_MS));
			continue;
		}

		uint64_t n_samples = signal.left_signal.size();
		uint64_t sample_rate = signal.sample_rate;
		uint64_t signal_duration_ms = (n_samples * 1000) / sample_rate;

		IControl::add_transmission_sample_start();
		send_audio(signal);
		// IControl::add_transmission_sample_end(signal_duration_ms);

		uint64_t sleep_ms = signal_duration_ms / 10;
		std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
	}
}
