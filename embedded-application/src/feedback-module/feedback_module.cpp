#include "feedback_module.hpp"

#include <iostream>

// #include "control_module.hpp"

feedback_module& feedback_module::get_instance() {
	static feedback_module instance;
	return instance;
}

feedback_module::feedback_module() {
  this->feedback_mode = NON_VERBAL_MODE;
	this->init_tap_signal();
	this->init_hrir_tensor();
	this->init_position_tree();
	this->init_verbal_feedback_tensor();
}

void feedback_module::init_tap_signal() {
	npy_data tap = read_npy<double>(TAP_SIGNAL_PATH);
	this->tap_signal = kfr::make_univector(tap.data);
}

void feedback_module::init_hrir_tensor() {
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

void feedback_module::init_position_tree() {
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

void feedback_module::init_verbal_feedback_tensor() {
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

void feedback_module::set_feedback_mode(FEEDBACK_MODES mode) {
	this->feedback_mode = mode;
}

kfr::univector<double, HRIR_N_TAPS> feedback_module::make_hrir_univector(uint64_t sample, uint64_t channel) {
	kfr::univector<double, HRIR_N_TAPS> hrir;
	for (uint64_t i = 0; i < HRIR_N_TAPS; i++) {
		hrir[i] = this->hrir_tensor(sample, i, channel);
	}
	return hrir;
}

uint8_t feedback_module::calculate_verbal_position(float azimuth, float elevation, float distance) {
	uint8_t position = 0;

	if (azimuth < (360 - VERBAL_AZIMUTH_THRESHOLD/2) && azimuth >= (360 - TOF_AZ_FOV/2)) {
		printf("Verbal feedback: Object is to the right\n");
		position |= RIGHT_MASK;
	}
	else if (azimuth > VERBAL_AZIMUTH_THRESHOLD/2 && azimuth <= TOF_AZ_FOV/2) {
		printf("Verbal feedback: Object is to the left\n");
		position |= LEFT_MASK;
	}
	else if (azimuth <= VERBAL_AZIMUTH_THRESHOLD/2 || azimuth >= (360 - VERBAL_AZIMUTH_THRESHOLD/2)) {
		printf("Verbal feedback: Object is horizontally centered\n");
		position |= HORIZONTAL_CENTERED_MASK;
	}
	else {
		printf("Verbal feedback: Azimuth angle out of range\n");
	}

	if (elevation > VERBAL_ELEVATION_THRESHOLD/2 && elevation <= TOF_EL_FOV/2) {
		printf("Verbal feedback: Object is above\n");
		position |= ABOVE_MASK;
	}
	else if (elevation < -VERBAL_ELEVATION_THRESHOLD/2 && elevation >= -TOF_EL_FOV/2) {
		printf("Verbal feedback: Object is below\n");
		position |= BELOW_MASK;
	}
	else if (elevation <= VERBAL_ELEVATION_THRESHOLD/2 && elevation >= -VERBAL_ELEVATION_THRESHOLD/2) {
		printf("Verbal feedback: Object is vertically centered\n");
		position |= VERTICALLY_CENTERED_MASK;
	}
	else {
		printf("Verbal feedback: Elevation angle out of range\n");
	}

	return position;
}

void feedback_module::generate_non_verbal_feedback(float azimuth, float elevation, float distance) {
	uint64_t sample_idx = this->position_tree.find_nearest({azimuth, elevation, distance});

	kfr::univector<double> output_l(this->tap_signal.size());
	kfr::univector<double> output_r(this->tap_signal.size());

	kfr::univector<double, HRIR_N_TAPS> hrir_l = this->make_hrir_univector(sample_idx, LEFT_CHANNEL);
	kfr::univector<double, HRIR_N_TAPS> hrir_r = this->make_hrir_univector(sample_idx, RIGHT_CHANNEL);
	
	kfr::filter_fir<double> filter_l(hrir_l);
	kfr::filter_fir<double> filter_r(hrir_r);

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

	write_npy("./output_non_verbal_l.npy", output_l_npy);
	write_npy("./output_non_verbal_r.npy", output_r_npy);
}

void feedback_module::generate_verbal_feedback(float azimuth, float elevation, float distance) {
	uint8_t position_idx;

	uint8_t position = this->calculate_verbal_position(azimuth, elevation, distance);
	switch (position) {
	case HORIZONTAL_CENTERED_MASK | VERTICALLY_CENTERED_MASK:
		printf("Verbal feedback: Position is FRONT\n");
		position_idx = FRONT;
		break;
	case ABOVE_MASK | HORIZONTAL_CENTERED_MASK:
		printf("Verbal feedback: Position is ABOVE\n");
		position_idx = ABOVE;
		break;
	case BELOW_MASK | HORIZONTAL_CENTERED_MASK:
		printf("Verbal feedback: Position is BELOW\n");
		position_idx = BELOW;
		break;
	case RIGHT_MASK | VERTICALLY_CENTERED_MASK:
		printf("Verbal feedback: Position is RIGHT\n");
		position_idx = RIGHT;
		break;
	case LEFT_MASK | VERTICALLY_CENTERED_MASK:
		printf("Verbal feedback: Position is LEFT\n");
		position_idx = LEFT;
		break;
	case ABOVE_MASK | RIGHT_MASK:
		printf("Verbal feedback: Position is ABOVE RIGHT\n");
		position_idx = ABOVE_RIGHT;
		break;
	case ABOVE_MASK | LEFT_MASK:
		printf("Verbal feedback: Position is ABOVE LEFT\n");
		position_idx = ABOVE_LEFT;
		break;
	case BELOW_MASK | RIGHT_MASK:
		printf("Verbal feedback: Position is BELOW RIGHT\n");
		position_idx = BELOW_RIGHT;
		break;
	case BELOW_MASK | LEFT_MASK:
		printf("Verbal feedback: Position is BELOW LEFT\n");
		position_idx = BELOW_LEFT;
		break;
	default:
		printf("Verbal feedback: Position is UNKNOWN\n");
		return;
	}

	uint64_t n_samples = this->verbal_feedback_tensor.shape()[1];
	npy_data<double> output_npy;
	vector<double> output(n_samples);
	for (uint64_t i = 0; i < n_samples; i++) {
		output[i] = this->verbal_feedback_tensor(position_idx, i);
	}
	output_npy.data = output;
	output_npy.shape = {n_samples};

	write_npy("./output_verbal.npy", output_npy);
}


void feedback_module::generate_feedback(float azimuth, float elevation, float distance) {
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

void feedback_module::start() {
	// control_module& ctrl_module = control_module::get_instance();
	// vector<vector<float>> test_positions = {
	// 	// {  0.0f,   0.0f, 0.5f}, // FRONT
	// 	// {  0.0f,  10.0f, 0.5f}, // ABOVE
	// 	// {  0.0f, -10.0f, 0.5f}, // BELOW
	// 	// {340.0f, 	 0.0f, 0.5f}, // RIGHT
	// 	// { 25.0f,   0.0f, 0.5f}, // LEFT
	// 	// {340.0f,  10.0f, 0.5f}, // ABOVE RIGHT
	// 	// { 25.0f,  10.0f, 0.5f}, // ABOVE LEFT
	// 	{340.0f, -10.0f, 0.5f}, // BELOW RIGHT
	// 	// { 25.0f, -10.0f, 0.5f}, // BELOW LEFT
	// };

	// for (uint64_t idx = 0; idx < test_positions.size(); ++idx) {
	// 	float azimuth = test_positions[idx][0];
	// 	float elevation = test_positions[idx][1];
	// 	float distance = test_positions[idx][2];

	// 	printf("===========================================================\n");
	// 	printf("Test position: (%f, %f, %f)\n", azimuth, elevation, distance);
	// 	this->generate_feedback(azimuth, elevation, distance);
	// 	printf("===========================================================\n");
	// }
}
