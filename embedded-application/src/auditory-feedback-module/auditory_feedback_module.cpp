#include <iostream>
#include "auditory_feedback_module.h"

#define AUDIO_SAMPLES 48000
#define HRIR_SAMPLES 16020
#define HRIR_TAPS 256
#define HRIR_CHANNELS 2
#define POSITION_CHANNELS 3

using namespace std;
using namespace kfr;
using namespace npy;

tensor<double, 3> init_hrir_tensor(vector<double> &hrirs_data) {
    tensor<double, 3> hrir_tensor({HRIR_SAMPLES, HRIR_TAPS, HRIR_CHANNELS});
    for (size_t i = 0; i < HRIR_SAMPLES; i++) {
        for (size_t j = 0; j < HRIR_TAPS; j++) {
            for (size_t k = 0; k < HRIR_CHANNELS; k++) {
                hrir_tensor(i, j, k) = hrirs_data[i * (HRIR_CHANNELS * HRIR_TAPS) + (j * HRIR_CHANNELS) + k];
            }
        }
    }
    return hrir_tensor;
}

tensor<double, 2> init_position_tensor(vector<double> &positions_data) {
    tensor<double, 2> position_tensor({HRIR_SAMPLES, POSITION_CHANNELS});
    for (size_t i = 0; i < HRIR_SAMPLES; i++) {
        for (size_t j = 0; j < POSITION_CHANNELS; j++) {
            position_tensor(i, j) = positions_data[i * POSITION_CHANNELS + j];
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

    univector<double, HRIR_TAPS> hrir_vector_l = make_hrir_univector(hrir_tensor, 0, 0);
    univector<double, HRIR_TAPS> hrir_vector_r = make_hrir_univector(hrir_tensor, 0, 1);
    filter_fir<double> filter_l(hrir_vector_l);
    filter_fir<double> filter_r(hrir_vector_r);

    univector<double> output_l;
    univector<double> output_r;
    filter_l.apply(output_l, audio_vector);
    filter_r.apply(output_r, audio_vector);

    return 0;
}
