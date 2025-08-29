#include "transmission_module.hpp"
#include "control_iface.hpp"

#include <chrono>
#include <thread>
#include <math.h>

#include "bluetooth_controller.h"

void interleave_audio(const vector<double>& left_channel, const vector<double>& right_channel, vector<double>& interleaved) {
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

void convert_to_pcm(const vector<double>& interleaved, vector<int16_t>& pcm, double max_value) {
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
