#include <iostream>

#include "auditory_feedback_module.hpp"

auditory_feedback_module& auditory_feedback_module::get_instance() {
	static auditory_feedback_module instance;
	return instance;
}

auditory_feedback_module::auditory_feedback_module() {
	this->init_tap_signal();
	this->init_hrir_tensor();
	this->init_position_tree();
	this->init_verbal_feedback_tensor();
}

void auditory_feedback_module::init_tap_signal() {
	npy_data tap = read_npy<double>(TAP_SIGNAL_PATH);
	this->tap_signal = make_univector(tap.data);
}

void auditory_feedback_module::init_hrir_tensor() {
	npy_data hrirs = read_npy<double>(HRIR_PATH);
	uint64_t n_samples = hrirs.shape[0];
	uint64_t n_taps = hrirs.shape[1];
	uint64_t n_channels = hrirs.shape[2];

	this->hrir_tensor = tensor<double, 3>({n_samples, n_taps, n_channels});
	for (uint64_t i = 0; i < n_samples; i++) {
		for (uint64_t j = 0; j < n_taps; j++) {
			for (uint64_t k = 0; k < n_channels; k++) {
				hrir_tensor(i, j, k) = hrirs.data[i * (n_channels * n_taps) + (j * n_channels) + k];
			}
		}
	}
}

void auditory_feedback_module::init_position_tree() {
	npy_data positions = read_npy<double>(POSITION_PATH);
	uint64_t n_samples = positions.shape[0];
	uint64_t n_channels = positions.shape[1];

	for (uint64_t i = 0; i < n_samples; i++) {
		uint64_t sample_addr = i * n_channels;
		float azimuth = positions.data[sample_addr + AZIMUTH_POSITION];
		float elevation = positions.data[sample_addr + ELEVATION_POSITION];
		float distance = positions.data[sample_addr + DISTANCE_POSITION];
		this->position_tree.insert(i, {azimuth, elevation, distance});
	}
}

void auditory_feedback_module::init_verbal_feedback_tensor() {
	npy_data verbal_feedback = read_npy<double>(VERBAL_FEEDBACK_PATH);
	uint64_t n_positions = verbal_feedback.shape[0];
	uint64_t n_samples = verbal_feedback.shape[1];

	this->verbal_feedback_tensor = tensor<double, 2>({n_positions, n_samples});
	for (uint64_t i = 0; i < n_positions; i++) {
		for (uint64_t j = 0; j < n_samples; j++) {
			this->verbal_feedback_tensor(i, j) = verbal_feedback.data[(i * n_samples) + j];
		}
	}
}

univector<double, HRIR_N_TAPS> auditory_feedback_module::make_hrir_univector(uint64_t sample, uint64_t channel) {
	univector<double, HRIR_N_TAPS> hrir;
	for (uint64_t i = 0; i < HRIR_N_TAPS; i++) {
		hrir[i] = this->hrir_tensor(sample, i, channel);
	}
	return hrir;
}

void auditory_feedback_module::generate_feedback(uint64_t sample_idx) {
	univector<double> output_l(this->tap_signal.size());
	univector<double> output_r(this->tap_signal.size());

	univector<double, HRIR_N_TAPS> hrir_l = this->make_hrir_univector(sample_idx, LEFT_CHANNEL);
	univector<double, HRIR_N_TAPS> hrir_r = this->make_hrir_univector(sample_idx, RIGHT_CHANNEL);
	
	filter_fir<double> filter_l(hrir_l);
	filter_fir<double> filter_r(hrir_r);

	filter_l.apply(output_l, this->tap_signal);
	filter_r.apply(output_r, this->tap_signal);

	npy_data<double> output_l_npy;
	npy_data<double> output_r_npy;
	
	vector<double> rend_l(this->tap_signal.size());
	vector<double> rend_r(this->tap_signal.size());
	for (uint64_t i = 0; i < this->tap_signal.size(); i++) {
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

void auditory_feedback_module::start() {
	float azimuth = 332.0f;
	float elevation = 10.0f;
	float distance = 0.5f;

	printf("Azimuth: %f, Elevation: %f, Distance: %f\n", azimuth, elevation, distance);

	uint64_t sample_idx = this->position_tree.find_nearest({azimuth, elevation, distance});
	this->generate_feedback(sample_idx);
}

int main() {
	auditory_feedback_module& feedback_module = auditory_feedback_module::get_instance();
	feedback_module.start();

	// kd_tree<3> kd_tree;
	// kd_tree.insert(0, {0.0, 0.0, 0.0});
	// kd_tree.insert(1, {1.0, 1.0, 1.0});
	// kd_tree.insert(2, {2.0, 2.0, 2.0});
	// kd_tree.insert(3, {3.0, 3.0, 3.0});
	// kd_tree.insert(4, {4.0, 4.0, 4.0});

	// array<double, 3> target = {3.4, 0.21, 2.0};
	// uint64_t nearest = kd_tree.find_nearest(target);
	// cout << "Nearest neighbor index: " << nearest << endl;

	return 0;
}
