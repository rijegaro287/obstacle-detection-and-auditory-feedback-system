#include "feedback_module.hpp"
#include "control_iface.hpp"

#include <iostream>
#include <thread>
#include <chrono>

FeedbackModule& FeedbackModule::get_instance() {
	static FeedbackModule instance;
	return instance;
}

FeedbackModule::FeedbackModule() {
  this->feedback_mode = NON_VERBAL_MODE;
	this->init_tap_signal();
	this->init_hrir_tensor();
	this->init_position_tree();
	this->init_verbal_feedback_tensor();
}

void FeedbackModule::init_tap_signal() {
	npy_data tap = read_npy<double>(TAP_SIGNAL_PATH);
	this->tap_signal = kfr::make_univector(tap.data);
}

void FeedbackModule::init_hrir_tensor() {
	npy_data hrirs = read_npy<double>(HRIR_PATH);
	uint64_t n_samples = hrirs.shape[0];
	uint64_t n_taps = hrirs.shape[1];
	uint64_t n_channels = hrirs.shape[2];

	this->hrir_tensor = kfr::tensor<double, 3>({n_samples, n_taps, n_channels});
	for (uint64_t i = 0; i < n_samples; i++) {
		for (uint64_t j = 0; j < n_taps; j++) {
			for (uint64_t k = 0; k < n_channels; k++) {
				hrir_tensor(i, j, k) = hrirs.data[i * (n_channels * n_taps) + (j * n_channels) + k];
			}
		}
	}
}

void FeedbackModule::init_position_tree() {
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

void FeedbackModule::init_verbal_feedback_tensor() {
	npy_data verbal_feedback = read_npy<double>(VERBAL_FEEDBACK_PATH);
	uint64_t n_positions = verbal_feedback.shape[0];
	uint64_t n_samples = verbal_feedback.shape[1];

	this->verbal_feedback_tensor = kfr::tensor<double, 2>({n_positions, n_samples});
	for (uint64_t i = 0; i < n_positions; i++) {
		for (uint64_t j = 0; j < n_samples; j++) {
			this->verbal_feedback_tensor(i, j) = verbal_feedback.data[(i * n_samples) + j];
		}
	}
}

void FeedbackModule::set_feedback_mode(FEEDBACK_MODES mode) {
	this->feedback_mode = mode;
}

kfr::univector<double, HRIR_N_TAPS> FeedbackModule::make_hrir_univector(uint64_t sample, uint64_t channel) {
	kfr::univector<double, HRIR_N_TAPS> hrir;
	for (uint64_t i = 0; i < HRIR_N_TAPS; i++) {
		hrir[i] = this->hrir_tensor(sample, i, channel);
	}
	return hrir;
}

uint8_t FeedbackModule::calculate_verbal_position(float azimuth, float elevation, float distance) {
	uint8_t position = 0;

	if (azimuth < (360 - VERBAL_AZIMUTH_THRESHOLD/2) && azimuth >= (360 - TOF_AZ_FOV/2)) {
		position |= RIGHT_MASK;
	}
	else if (azimuth > VERBAL_AZIMUTH_THRESHOLD/2 && azimuth <= TOF_AZ_FOV/2) {
		position |= LEFT_MASK;
	}
	else if (azimuth <= VERBAL_AZIMUTH_THRESHOLD/2 || azimuth >= (360 - VERBAL_AZIMUTH_THRESHOLD/2)) {
		position |= HORIZONTAL_CENTERED_MASK;
	}
	else {
		printf("Verbal feedback: Azimuth angle out of range\n");
	}

	if (elevation > VERBAL_ELEVATION_THRESHOLD/2 && elevation <= TOF_EL_FOV/2) {
		position |= ABOVE_MASK;
	}
	else if (elevation < -VERBAL_ELEVATION_THRESHOLD/2 && elevation >= -TOF_EL_FOV/2) {
		position |= BELOW_MASK;
	}
	else if (elevation <= VERBAL_ELEVATION_THRESHOLD/2 && elevation >= -VERBAL_ELEVATION_THRESHOLD/2) {
		position |= VERTICALLY_CENTERED_MASK;
	}
	else {
		printf("Verbal feedback: Elevation angle out of range\n");
	}

	return position;
}

void FeedbackModule::generate_non_verbal_feedback(float azimuth, float elevation, float distance) {
	audio_data_t signal;
	signal.left_signal = vector<double>(this->tap_signal.size());
	signal.right_signal = vector<double>(this->tap_signal.size());
	signal.sample_rate = NON_VERBAL_SAMPLE_RATE;

	uint64_t sample_idx = this->position_tree.find_nearest({azimuth, elevation, distance});

	kfr::univector<double> output_l(this->tap_signal.size());
	kfr::univector<double> output_r(this->tap_signal.size());

	kfr::univector<double, HRIR_N_TAPS> hrir_l = this->make_hrir_univector(sample_idx, LEFT_CHANNEL);
	kfr::univector<double, HRIR_N_TAPS> hrir_r = this->make_hrir_univector(sample_idx, RIGHT_CHANNEL);
	
	kfr::filter_fir<double> filter_l(hrir_l);
	kfr::filter_fir<double> filter_r(hrir_r);

	filter_l.apply(output_l, this->tap_signal);
	filter_r.apply(output_r, this->tap_signal);

	for (uint64_t i = 0; i < this->tap_signal.size(); i++) {
		signal.left_signal[i] = output_l[i];
		signal.right_signal[i] = output_r[i];
	}

	IControl::set_audio_data(signal);

	// npy_data<double> output_l_npy;
	// npy_data<double> output_r_npy;
	
	// vector<double> rend_l(this->tap_signal.size());
	// vector<double> rend_r(this->tap_signal.size());

	// output_l_npy.data = rend_l;
	// output_r_npy.data = rend_r;
	
	// output_l_npy.shape = {this->tap_signal.size()};
	// output_r_npy.shape = {this->tap_signal.size()};

	// write_npy("./output_non_verbal_l.npy", output_l_npy);
	// write_npy("./output_non_verbal_r.npy", output_r_npy);
}

void FeedbackModule::generate_verbal_feedback(float azimuth, float elevation, float distance) {
	uint64_t n_samples = this->verbal_feedback_tensor.shape()[1];

	audio_data_t signal;
	signal.left_signal = vector<double>(n_samples);
	signal.right_signal = vector<double>(n_samples);
	signal.sample_rate = VERBAL_SAMPLE_RATE;

	uint8_t position_idx;

	uint8_t position = this->calculate_verbal_position(azimuth, elevation, distance);
	switch (position) {
	case HORIZONTAL_CENTERED_MASK | VERTICALLY_CENTERED_MASK:
		position_idx = FRONT;
		break;
	case ABOVE_MASK | HORIZONTAL_CENTERED_MASK:
		position_idx = ABOVE;
		break;
	case BELOW_MASK | HORIZONTAL_CENTERED_MASK:
		position_idx = BELOW;
		break;
	case RIGHT_MASK | VERTICALLY_CENTERED_MASK:
		position_idx = RIGHT;
		break;
	case LEFT_MASK | VERTICALLY_CENTERED_MASK:
		position_idx = LEFT;
		break;
	case ABOVE_MASK | RIGHT_MASK:
		position_idx = ABOVE_RIGHT;
		break;
	case ABOVE_MASK | LEFT_MASK:
		position_idx = ABOVE_LEFT;
		break;
	case BELOW_MASK | RIGHT_MASK:
		position_idx = BELOW_RIGHT;
		break;
	case BELOW_MASK | LEFT_MASK:
		position_idx = BELOW_LEFT;
		break;
	default:
		return;
	}

	for (uint64_t i = 0; i < n_samples; i++) {
		signal.left_signal[i] = this->verbal_feedback_tensor(position_idx, i);
		signal.right_signal[i] = this->verbal_feedback_tensor(position_idx, i);
	}
	printf("Verbal feedback generated\n");

	IControl::set_audio_data(signal);
	// npy_data<double> output_npy;
	// output_npy.data = signal.left_signal;
	// output_npy.shape = {n_samples};

	// write_npy("./output_verbal.npy", output_npy);
}

void FeedbackModule::generate_feedback(float azimuth, float elevation, float distance) {
	if (this->feedback_mode == NON_VERBAL_MODE) {
		printf("Generating non-verbal feedback...\n");
		this->generate_non_verbal_feedback(azimuth, elevation, distance);
	} 
	else if (this->feedback_mode == VERBAL_MODE) {
		printf("Generating verbal feedback...\n");
		this->generate_verbal_feedback(azimuth, elevation, distance);
	}
	else {
		printf("Invalid feedback mode\n");
	}
}

void FeedbackModule::start() {
	while (true) {
		obstacle_position_t position = IControl::get_obstacle_position();
		if (position.distance == 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		printf("Obstacle Position - Azimuth: %.2f, Elevation: %.2f, Distance: %.2f\n", 
					 position.azimuth, position.elevation, position.distance);

		this->generate_feedback(position.azimuth, position.elevation, position.distance);

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}
