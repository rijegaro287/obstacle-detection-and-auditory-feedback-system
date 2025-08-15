#include <iostream>
#include "auditory_feedback_module.h"

#define SAMPLE_RATE 48000
#define AUDIO_SAMPLES 48000
#define HRIR_SAMPLES 16020
#define HRIR_TAPS 256
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

tensor<double, 3> init_hrir_tensor(vector<double> &hrirs_data) {
    tensor<double, 3> hrir_tensor({HRIR_SAMPLES, HRIR_TAPS, HRIR_N_CHANNELS});
    for (size_t i = 0; i < HRIR_SAMPLES; i++) {
        for (size_t j = 0; j < HRIR_TAPS; j++) {
            for (size_t k = 0; k < HRIR_N_CHANNELS; k++) {
                hrir_tensor(i, j, k) = hrirs_data[i * (HRIR_N_CHANNELS * HRIR_TAPS) + (j * HRIR_N_CHANNELS) + k];
            }
        }
    }
    return hrir_tensor;
}

tensor<double, 2> init_position_tensor(vector<double> &positions_data) {
    tensor<double, 2> position_tensor({HRIR_SAMPLES, POSITION_N_CHANNELS});
    for (size_t i = 0; i < HRIR_SAMPLES; i++) {
        for (size_t j = 0; j < POSITION_N_CHANNELS; j++) {
            position_tensor(i, j) = positions_data[i * POSITION_N_CHANNELS + j];
        }
    }
    return position_tensor;
}

univector<double, HRIR_TAPS> make_hrir_univector(tensor<double, 3> &hrir, uint64_t sample, uint64_t channel) {
    univector<double, HRIR_TAPS> vec;
    for (size_t i = 0; i < HRIR_TAPS; i++) {
        vec[i] = hrir(sample, i, channel);
    }
    return vec;
}

int main() {
    npy_data audio = read_npy<double>("./tap_alert.npy");
    npy_data hrirs = read_npy<double>("./dataset/hrirs.npy");
    npy_data positions = read_npy<double>("./dataset/positions.npy");

    univector<double, AUDIO_SAMPLES> audio_vector = make_univector(audio.data);
    tensor<double, 3> hrir_tensor = init_hrir_tensor(hrirs.data);
    tensor<double, 2> position_tensor = init_position_tensor(positions.data);

    univector<double, HRIR_TAPS> hrir_vector_l = make_hrir_univector(hrir_tensor, 5781, LEFT_CHANNEL);
    univector<double, HRIR_TAPS> hrir_vector_r = make_hrir_univector(hrir_tensor, 5781, RIGHT_CHANNEL);
    filter_fir<double> filter_l(hrir_vector_l);
    filter_fir<double> filter_r(hrir_vector_r);

    univector<double> output_l(audio_vector.size());
    univector<double> output_r(audio_vector.size());
    filter_l.apply(output_l, audio_vector);
    filter_r.apply(output_r, audio_vector);

    // printf("Position: ");
    // for (size_t i = 0; i < POSITION_CHANNELS; i++) {
    //     printf("%f ", position_tensor(5781, i));
    // }
    // printf("\n");

    // npy_data<double> output_l_npy;
    // npy_data<double> output_r_npy;
    
    // vector<double> rend_l(audio_vector.size());
    // vector<double> rend_r(audio_vector.size());
    // for (size_t i = 0; i < audio_vector.size(); i++) {
    //     rend_l[i] = output_l[i];
    //     rend_r[i] = output_r[i];
    // }

    // output_l_npy.data = rend_l;
    // output_l_npy.shape = {audio_vector.size()};

    // output_r_npy.data = rend_r;
    // output_r_npy.shape = {audio_vector.size()};

    // write_npy("./output_l.npy", output_l_npy);
    // write_npy("./output_r.npy", output_r_npy);

    return 0;
}
