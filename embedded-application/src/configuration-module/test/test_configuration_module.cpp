#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include <thread>

#define private public
#define protected public
#include "configuration_module.hpp"
#include "control_module.hpp"
#include "feedback_module.hpp"
#include "image_capture_module.hpp"
#include "obstacle_detection_module.hpp"
#include "transmission_module.hpp"
#include "bt_audio.hpp"
#undef private
#undef protected

using namespace std;

class ConfigModuleTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Ensure deterministic state for each test run.
    FeedbackModule::get_instance().stop_feedback();
    ImageCaptureModule::get_instance().running = false;
    ObstacleDetectionModule::get_instance().running = false;
    TransmissionModule::get_instance().running = false;
    ConfigModule::get_instance().set_response_buffer("");
  }
};

TEST_F(ConfigModuleTest, SplitHandlesDelimiters) {
  auto& module = ConfigModule::get_instance();
  vector<string> tokens = module.split("alpha!beta!gamma", '!');
  ASSERT_EQ(tokens.size(), 3u);
  EXPECT_EQ(tokens[0], "alpha");
  EXPECT_EQ(tokens[1], "beta");
  EXPECT_EQ(tokens[2], "gamma");
}

TEST_F(ConfigModuleTest, MapCommandToCodeRecognisesKnownCommands) {
  auto& module = ConfigModule::get_instance();
  EXPECT_EQ(module.map_command_to_code(START_FEEDBACK_COMMAND), START_FEEDBACK_CODE);
  EXPECT_EQ(module.map_command_to_code(STOP_FEEDBACK_COMMAND), STOP_FEEDBACK_CODE);
  EXPECT_EQ(module.map_command_to_code("unknown"), -1);
}

TEST_F(ConfigModuleTest, ProcessCommandRejectsInvalidFormat) {
  auto& module = ConfigModule::get_instance();
  module.process_command("invalid!command!format!extra");
  EXPECT_EQ(module.get_response_buffer(), "#Error: Invalid command format");
}

TEST_F(ConfigModuleTest, ProcessCommandUnknownReturnsEmptyResponse) {
  auto& module = ConfigModule::get_instance();
  module.process_command("unknown_command");
  EXPECT_TRUE(module.get_response_buffer().empty());
}

TEST_F(ConfigModuleTest, HealthCheckReturnsOkAndClearsBuffer) {
  auto& module = ConfigModule::get_instance();
  module.process_command(HEALTH_CHECK_COMMAND);
  EXPECT_EQ(module.get_response_buffer(), "OK");
  EXPECT_TRUE(module.get_response_buffer().empty());
}

TEST_F(ConfigModuleTest, SetResponseBufferClearsOnRead) {
  auto& module = ConfigModule::get_instance();
  module.set_response_buffer("test");
  EXPECT_EQ(module.get_response_buffer(), "test");
  EXPECT_TRUE(module.get_response_buffer().empty());
}

TEST_F(ConfigModuleTest, SetVolumeCommandUpdatesFeedbackModule) {
  auto& config = ConfigModule::get_instance();
  auto& feedback = FeedbackModule::get_instance();
  float original_volume = feedback.volume;
  feedback.volume = 0.0f;

  config.process_command(string(SET_VOLUME_COMMAND) + "!80");
  EXPECT_NEAR(feedback.volume, 0.80f, 1e-5f);
  EXPECT_EQ(config.get_response_buffer(), "Volume set to 80");

  feedback.volume = original_volume;
}

TEST_F(ConfigModuleTest, SetVolumeCommandValidatesMissingArgument) {
  auto& config = ConfigModule::get_instance();
  config.process_command(string(SET_VOLUME_COMMAND) + "!");
  EXPECT_EQ(config.get_response_buffer(), "#Error: No volume value provided");
}

TEST_F(ConfigModuleTest, SetFeedbackModeCommandUpdatesFeedbackModule) {
  auto& config = ConfigModule::get_instance();
  auto& feedback = FeedbackModule::get_instance();
  FEEDBACK_MODES original_mode = feedback.feedback_mode;

  config.process_command(string(SET_FEEDBACK_MODE_COMMAND) + "!" + VERBAL_MODE_STRING);
  EXPECT_EQ(feedback.feedback_mode, VERBAL_MODE);
  EXPECT_EQ(config.get_response_buffer(), string("Feedback mode set to ") + VERBAL_MODE_STRING);

  feedback.feedback_mode = original_mode;
}

TEST_F(ConfigModuleTest, SetFeedbackModeCommandValidatesArgument) {
  auto& config = ConfigModule::get_instance();
  config.process_command(string(SET_FEEDBACK_MODE_COMMAND) + "!invalid");
  EXPECT_EQ(config.get_response_buffer(), "#Error: Invalid feedback mode");
}

TEST_F(ConfigModuleTest, StartFeedbackCommandEnablesSubsystems) {
  auto& config = ConfigModule::get_instance();
  auto& image_module = ImageCaptureModule::get_instance();
  auto& detection_module = ObstacleDetectionModule::get_instance();
  auto& feedback_module = FeedbackModule::get_instance();
  auto& transmission_module = TransmissionModule::get_instance();

  bool original_image_running = image_module.running;
  bool original_detection_running = detection_module.running;
  bool original_feedback_running = feedback_module.running;
  bool original_transmission_running = transmission_module.running;

  image_module.running = false;
  detection_module.running = false;
  feedback_module.running = false;
  transmission_module.running = false;

  config.process_command(START_FEEDBACK_COMMAND);
  config.get_response_buffer();

  EXPECT_TRUE(image_module.running);
  EXPECT_TRUE(detection_module.running);
  EXPECT_TRUE(feedback_module.running);
  EXPECT_TRUE(transmission_module.running);

  config.process_command(STOP_FEEDBACK_COMMAND);
  config.get_response_buffer();

  EXPECT_FALSE(image_module.running);
  EXPECT_FALSE(detection_module.running);
  EXPECT_FALSE(feedback_module.running);
  EXPECT_FALSE(transmission_module.running);

  image_module.running = original_image_running;
  detection_module.running = original_detection_running;
  feedback_module.running = original_feedback_running;
  transmission_module.running = original_transmission_running;
}

TEST_F(ConfigModuleTest, BTControllerAddrToPathReplacesColons) {
  BTController controller;
  char address[] = "AA:BB:CC:DD";
  char formatted[BUFFER_SIZE_S] = {};

  controller.addr_to_path(address, formatted, BUFFER_SIZE_S);
  EXPECT_STREQ(formatted, "AA_BB_CC_DD");
}

TEST_F(ConfigModuleTest, BTControllerClearDevicesResetsEntries) {
  BTController controller;
  vector<BlueZDevice> devices(2);
  strncpy(devices[0].name, "DeviceA", BUFFER_SIZE_L - 1);
  strncpy(devices[0].address, "AA:BB:CC", BUFFER_SIZE_S - 1);
  strncpy(devices[1].name, "DeviceB", BUFFER_SIZE_L - 1);
  strncpy(devices[1].address, "DD:EE:FF", BUFFER_SIZE_S - 1);

  controller.clear_devices(devices);

  EXPECT_EQ(devices[0].name[0], '\0');
  EXPECT_EQ(devices[0].address[0], '\0');
  EXPECT_EQ(devices[1].name[0], '\0');
  EXPECT_EQ(devices[1].address[0], '\0');
}

TEST_F(ConfigModuleTest, BTAudioControllerFindDeviceIdxFindsMatchingDevice) {
  auto& audio_controller = BTAudioController::get_instance();
  vector<BlueZDevice> devices(2);
  strncpy(devices[0].address, "11:22:33", BUFFER_SIZE_S - 1);
  strncpy(devices[1].address, "AA:BB:CC", BUFFER_SIZE_S - 1);

  EXPECT_EQ(audio_controller.find_device_idx(devices, "11:22:33"), 0);
  EXPECT_EQ(audio_controller.find_device_idx(devices, "AA:BB:CC"), 1);
  EXPECT_EQ(audio_controller.find_device_idx(devices, "FF:EE:DD"), -1);
}
