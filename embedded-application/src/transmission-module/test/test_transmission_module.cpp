#include <gtest/gtest.h>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>

#define private public
#define protected public
#include "transmission_module.hpp"
#include "transmission_iface.hpp"
#undef private
#undef protected

using namespace std;

class TransmissionModuleTest : public ::testing::Test {
protected:
  void SetUp() override {
    module = &TransmissionModule::get_instance();
  }

  TransmissionModule* module;
};

TEST_F(TransmissionModuleTest, InterleaveAudioFillsDestinationBuffer) {
  vector<float> left = {0.1f, 0.2f, 0.3f};
  vector<float> right = {0.4f, 0.5f, 0.6f};
  vector<float> interleaved(left.size() * 2, 0.0f);

  module->interleave_audio(left, right, interleaved);

  EXPECT_FLOAT_EQ(interleaved[0], left[0]);
  EXPECT_FLOAT_EQ(interleaved[1], right[0]);
  EXPECT_FLOAT_EQ(interleaved[4], left[2]);
  EXPECT_FLOAT_EQ(interleaved[5], right[2]);
}

TEST_F(TransmissionModuleTest, InterleaveAudioWithMismatchedSizesLeavesBufferUntouched) {
  vector<float> left = {0.1f};
  vector<float> right = {0.2f, 0.3f};
  vector<float> interleaved(4, 1.0f);
  vector<float> original = interleaved;

  module->interleave_audio(left, right, interleaved);
  EXPECT_EQ(interleaved, original);
}

TEST_F(TransmissionModuleTest, ConvertToPcmScalesAndClampsValues) {
  vector<float> interleaved = {0.5f, -1.5f};
  vector<int16_t> pcm(2, 0);

  module->convert_to_pcm(interleaved, pcm, 1.0f, 1.0f);

  EXPECT_EQ(pcm[0], static_cast<int16_t>(0.5f * numeric_limits<int16_t>::max()));
  EXPECT_EQ(pcm[1], numeric_limits<int16_t>::min());
}

TEST_F(TransmissionModuleTest, PreprocessAudioGeneratesInterleavedPcm) {
  Audio audio;
  audio.left_signal = {0.1f, 0.2f, 0.3f, 0.4f};
  audio.right_signal = {0.5f, 0.6f, 0.7f, 0.8f};
  audio.sample_rate = 48000;
  audio.gain = 0.5f;

  vector<int16_t> pcm(audio.left_signal.size() * 2, 0);
  module->preprocess_audio(audio, pcm, 1.0f, 0, audio.left_signal.size());

  vector<float> expected_interleaved;
  expected_interleaved.reserve(audio.left_signal.size() * 2);
  for (size_t i = 0; i < audio.left_signal.size(); ++i) {
    expected_interleaved.push_back(audio.left_signal[i]);
    expected_interleaved.push_back(audio.right_signal[i]);
  }

  vector<int16_t> expected(pcm.size());
  float scale = static_cast<float>(numeric_limits<int16_t>::max());
  for (size_t i = 0; i < expected.size(); ++i) {
    float value = expected_interleaved[i] * scale * audio.gain;
    if (value > numeric_limits<int16_t>::max()) value = static_cast<float>(numeric_limits<int16_t>::max());
    if (value < numeric_limits<int16_t>::min()) value = static_cast<float>(numeric_limits<int16_t>::min());
    expected[i] = static_cast<int16_t>(value);
  }

  EXPECT_EQ(pcm, expected);
}

TEST_F(TransmissionModuleTest, ConvertToPcmValidatesBufferLayout) {
  vector<float> odd_samples = {0.1f};
  vector<int16_t> pcm = {7, 8};
  vector<int16_t> original = pcm;
  module->convert_to_pcm(odd_samples, pcm, 1.0f, 1.0f);
  EXPECT_EQ(pcm, original);

  vector<float> even_samples = {0.1f, -0.2f};
  vector<int16_t> short_pcm = {1};
  module->convert_to_pcm(even_samples, short_pcm, 1.0f, 1.0f);
  EXPECT_EQ(short_pcm[0], 1);
}

TEST_F(TransmissionModuleTest, SendPcmDataGuardsAgainstInvalidArguments) {
  vector<int16_t> empty_pcm;
  module->send_pcm_data(empty_pcm, 48000);

  vector<int16_t> pcm = {0, 1};
  module->send_pcm_data(pcm, 0);

  snd_pcm_t* original_handle = module->pcm_handle;
  module->pcm_handle = nullptr;
  module->send_pcm_data(pcm, 48000);
  module->pcm_handle = original_handle;
}

TEST_F(TransmissionModuleTest, SendAudioValidatesSignal) {
  Audio audio;
  audio.left_signal = {0.1f, 0.2f};
  audio.right_signal = {0.3f};
  audio.sample_rate = 48000;
  audio.gain = 1.0f;
  module->send_audio(audio);

  audio.right_signal = audio.left_signal;
  audio.sample_rate = 0;
  module->send_audio(audio);

  audio.sample_rate = 48000;
  snd_pcm_t* original_handle = module->pcm_handle;
  module->pcm_handle = nullptr;
  module->send_audio(audio);
  module->pcm_handle = original_handle;
}

TEST(TransmissionInterfaceTest, FacadeStartStopToggleModuleState) {
  auto& module = TransmissionModule::get_instance();
  module.running = false;
  ITransmission::start_transmission();
  EXPECT_TRUE(module.running);
  ITransmission::stop_transmission();
  EXPECT_FALSE(module.running);
}
