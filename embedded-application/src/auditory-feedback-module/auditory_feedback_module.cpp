#include <iostream>

#include "auditory_feedback_module.h"

auditory_feedback_module& auditory_feedback_module::get_instance() {
	static auditory_feedback_module instance;
	return instance;
}

auditory_feedback_module::auditory_feedback_module() {
	npy_data tap = read_npy<double>(TAP_SIGNAL_PATH);
	npy_data hrirs = read_npy<double>(HRIR_PATH);
	npy_data positions = read_npy<double>(POSITION_PATH);

	this->tap_signal = make_univector(tap.data);
	this->init_hrir_tensor(hrirs.data);
	this->init_position_tensor(positions.data);
}

void auditory_feedback_module::init_hrir_tensor(vector<double>& hrirs_data) {
	this->hrir_tensor = tensor<double, 3>({HRIR_N_SAMPLES, HRIR_N_TAPS, HRIR_N_CHANNELS});
	for (size_t i = 0; i < HRIR_N_SAMPLES; i++) {
		for (size_t j = 0; j < HRIR_N_TAPS; j++) {
			for (size_t k = 0; k < HRIR_N_CHANNELS; k++) {
				hrir_tensor(i, j, k) = hrirs_data[i * (HRIR_N_CHANNELS * HRIR_N_TAPS) + (j * HRIR_N_CHANNELS) + k];
			}
		}
	}
}

void auditory_feedback_module::init_position_tensor(vector<double>& positions_data) {
	this->position_tensor = tensor<double, 2>({HRIR_N_SAMPLES, POSITION_N_CHANNELS});
	for (size_t i = 0; i < HRIR_N_SAMPLES; i++) {
		for (size_t j = 0; j < POSITION_N_CHANNELS; j++) {
			position_tensor(i, j) = positions_data[i * POSITION_N_CHANNELS + j];
		}
	}
}

univector<double, HRIR_N_TAPS> auditory_feedback_module::make_hrir_univector(uint64_t sample, uint64_t channel) {
	univector<double, HRIR_N_TAPS> hrir;
	for (size_t i = 0; i < HRIR_N_TAPS; i++) {
		hrir[i] = this->hrir_tensor(sample, i, channel);
	}
	return hrir;
}

void auditory_feedback_module::generate_feedback(float azimuth, float elevation, float distance) {
	univector<double> output_l(this->tap_signal.size());
	univector<double> output_r(this->tap_signal.size());

	univector<double, HRIR_N_TAPS> hrir_l = this->make_hrir_univector(5781, LEFT_CHANNEL);
	univector<double, HRIR_N_TAPS> hrir_r = make_hrir_univector(5781, RIGHT_CHANNEL);
	
	filter_fir<double> filter_l(hrir_l);
	filter_fir<double> filter_r(hrir_r);

	filter_l.apply(output_l, this->tap_signal);
	filter_r.apply(output_r, this->tap_signal);

	npy_data<double> output_l_npy;
	npy_data<double> output_r_npy;
	
	vector<double> rend_l(this->tap_signal.size());
	vector<double> rend_r(this->tap_signal.size());
	for (size_t i = 0; i < this->tap_signal.size(); i++) {
			rend_l[i] = output_l[i];
			rend_r[i] = output_r[i];
	}

	output_l_npy.data = rend_l;
	output_r_npy.data = rend_r;
	
	output_l_npy.shape = {this->tap_signal.size()};
	output_r_npy.shape = {this->tap_signal.size()};

	write_npy("./output_l.npy", output_l_npy);
	write_npy("./output_r.npy", output_r_npy);
}


int main() {
	auditory_feedback_module& feedback_module = auditory_feedback_module::get_instance();
	feedback_module.generate_feedback(0.0f, 0.0f, 1.0f);
	return 0;
}
