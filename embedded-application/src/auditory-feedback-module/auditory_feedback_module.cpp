#include <iostream>
#include "auditory_feedback_module.h"

using namespace std;
using namespace npy;
using namespace kfr;

int main() {
    npy_data audio = read_npy<double>("./tap_alert.npy");
    npy_data hrirs = read_npy<double>("./dataset/hrirs.npy");
    npy_data positions = read_npy<double>("./dataset/positions.npy");

    tensor<double, 2> audio_tensor = {};

    printf("oela    %zu\n", audio.data.size());
    printf("hrirs   %zu\n", hrirs.data.size());
    printf("positions %zu\n", positions.data.size());

    return 0;
}
