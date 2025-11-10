#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include <thread>
#include <algorithm>
#include <unordered_set>
#include <gio/gio.h>

#define private public
#define protected public
#include "configuration_module.hpp"
#include "configuration_iface.hpp"
#include "ble_server.hpp"
#include "control_module.hpp"
#include "feedback_module.hpp"
#include "image_capture_module.hpp"
#include "obstacle_detection_module.hpp"
#include "transmission_module.hpp"
#include "bt_audio.hpp"
#undef private
#undef protected

using namespace std;

namespace {

GVariant* make_device_variant(const char* object_path,
                              const char* name,
                              const char* address) {
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a{oa{sa{sv}}}"));

  g_variant_builder_open(&builder, G_VARIANT_TYPE("{oa{sa{sv}}}"));
  g_variant_builder_add(&builder, "o", object_path);

  g_variant_builder_open(&builder, G_VARIANT_TYPE("a{sa{sv}}"));
  g_variant_builder_open(&builder, G_VARIANT_TYPE("{sa{sv}}"));
  g_variant_builder_add(&builder, "s", BLUEZ_DEVICE_IFACE);

  g_variant_builder_open(&builder, G_VARIANT_TYPE("a{sv}"));
  g_variant_builder_add(&builder, "{sv}", "Name", g_variant_new_string(name));
  g_variant_builder_add(&builder, "{sv}", "Address", g_variant_new_string(address));
  g_variant_builder_close(&builder);

  g_variant_builder_close(&builder);
  g_variant_builder_close(&builder);
  g_variant_builder_close(&builder);

  return g_variant_ref_sink(g_variant_builder_end(&builder));
}

GVariant* make_devices_variant(const vector<pair<string, string>>& entries) {
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a{oa{sa{sv}}}"));

  for (const auto& entry : entries) {
    string sanitized_address = entry.second;
    replace(sanitized_address.begin(), sanitized_address.end(), ':', '_');
    string object_path = string(BLUEZ_ADAPTER_PATH) + "/dev_" + sanitized_address;

    g_variant_builder_open(&builder, G_VARIANT_TYPE("{oa{sa{sv}}}"));
    g_variant_builder_add(&builder, "o", object_path.c_str());

    g_variant_builder_open(&builder, G_VARIANT_TYPE("a{sa{sv}}"));
    g_variant_builder_open(&builder, G_VARIANT_TYPE("{sa{sv}}"));
    g_variant_builder_add(&builder, "s", BLUEZ_DEVICE_IFACE);

    g_variant_builder_open(&builder, G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(&builder, "{sv}", "Name", g_variant_new_string(entry.first.c_str()));
    g_variant_builder_add(&builder, "{sv}", "Address", g_variant_new_string(entry.second.c_str()));
    g_variant_builder_close(&builder);

    g_variant_builder_close(&builder);
    g_variant_builder_close(&builder);
    g_variant_builder_close(&builder);
  }

  return g_variant_ref_sink(g_variant_builder_end(&builder));
}

GVariant* make_non_device_variant(const char* object_path) {
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a{oa{sa{sv}}}"));

  g_variant_builder_open(&builder, G_VARIANT_TYPE("{oa{sa{sv}}}"));
  g_variant_builder_add(&builder, "o", object_path);

  g_variant_builder_open(&builder, G_VARIANT_TYPE("a{sa{sv}}"));
  g_variant_builder_open(&builder, G_VARIANT_TYPE("{sa{sv}}"));
  g_variant_builder_add(&builder, "s", BLUEZ_ADAPTER_IFACE);
  g_variant_builder_open(&builder, G_VARIANT_TYPE("a{sv}"));
  g_variant_builder_close(&builder);
  g_variant_builder_close(&builder);
  g_variant_builder_close(&builder);
  g_variant_builder_close(&builder);

  return g_variant_ref_sink(g_variant_builder_end(&builder));
}

GVariant* make_device_variant_without_address(const char* object_path,
                                              const char* name) {
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a{oa{sa{sv}}}"));

  g_variant_builder_open(&builder, G_VARIANT_TYPE("{oa{sa{sv}}}"));
  g_variant_builder_add(&builder, "o", object_path);

  g_variant_builder_open(&builder, G_VARIANT_TYPE("a{sa{sv}}"));
  g_variant_builder_open(&builder, G_VARIANT_TYPE("{sa{sv}}"));
  g_variant_builder_add(&builder, "s", BLUEZ_DEVICE_IFACE);

  g_variant_builder_open(&builder, G_VARIANT_TYPE("a{sv}"));
  g_variant_builder_add(&builder, "{sv}", "Name", g_variant_new_string(name));
  g_variant_builder_close(&builder);

  g_variant_builder_close(&builder);
  g_variant_builder_close(&builder);
  g_variant_builder_close(&builder);

  return g_variant_ref_sink(g_variant_builder_end(&builder));
}

} // namespace

class ConfigModuleTest : public ::testing::Test {
protected:
  vector<BlueZDevice> stub_devices;
  unordered_set<string> paired_addresses;
  unordered_set<string> connected_addresses;

  void SetUp() override {
    FeedbackModule::get_instance().stop_feedback();
    ImageCaptureModule::get_instance().running = false;
    ObstacleDetectionModule::get_instance().running = false;
    TransmissionModule::get_instance().running = false;
    ConfigModule::get_instance().set_response_buffer("");

    paired_addresses.clear();
    connected_addresses.clear();

    BTAudioController::reset_test_overrides();
    auto& overrides = BTAudioController::test_overrides;
    overrides.start_discovery = []() -> int64_t { return 0; };
    overrides.stop_discovery = []() -> int64_t { return 0; };
    overrides.get_discovered_devices = [this](vector<BlueZDevice>& dest) -> int64_t {
      dest = this->stub_devices;
      return static_cast<int64_t>(dest.size());
    };
    overrides.pair_device = [this](BlueZDevice& device) -> int64_t {
      this->paired_addresses.insert(string(device.address));
      return 0;
    };
    overrides.connect_device = [this](BlueZDevice& device) -> int64_t {
      this->connected_addresses.insert(string(device.address));
      BTAudioController::get_instance().connected_device = &device;
      return 0;
    };
    overrides.disconnect_device = [this](BlueZDevice& device) -> int64_t {
      this->connected_addresses.erase(string(device.address));
      if (BTAudioController::get_instance().connected_device == &device) {
        BTAudioController::get_instance().connected_device = nullptr;
      }
      return 0;
    };
    overrides.is_paired = [this](BlueZDevice& device) -> bool {
      return this->paired_addresses.count(string(device.address)) > 0;
    };
    overrides.is_connected = [this](BlueZDevice& device) -> bool {
      return this->connected_addresses.count(string(device.address)) > 0;
    };

    auto& audio = BTAudioController::get_instance();
    audio.connected_device = nullptr;

    stub_devices.clear();
  BlueZDevice device_a{};
  strncpy(device_a.name, "DeviceA", BUFFER_SIZE_L - 1);
  strncpy(device_a.address, "AA:BB:CC", BUFFER_SIZE_S - 1);
  BlueZDevice device_b{};
  strncpy(device_b.name, "DeviceB", BUFFER_SIZE_L - 1);
  strncpy(device_b.address, "11:22:33", BUFFER_SIZE_S - 1);
    stub_devices.push_back(device_a);
    stub_devices.push_back(device_b);
  }

  void TearDown() override {
    BTAudioController::reset_test_overrides();
    BTAudioController::get_instance().connected_device = nullptr;
    stub_devices.clear();
    paired_addresses.clear();
    connected_addresses.clear();
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

TEST_F(ConfigModuleTest, SplitPreservesEmptyTokens) {
  auto& module = ConfigModule::get_instance();
  vector<string> tokens = module.split("leading!!trailing!", '!');
  ASSERT_EQ(tokens.size(), 3u);
  EXPECT_EQ(tokens[0], "leading");
  EXPECT_TRUE(tokens[1].empty());
  EXPECT_EQ(tokens[2], "trailing");
}

TEST_F(ConfigModuleTest, SplitWithoutDelimiterReturnsEntireString) {
  auto& module = ConfigModule::get_instance();
  vector<string> tokens = module.split("solitary", '!');
  ASSERT_EQ(tokens.size(), 1u);
  EXPECT_EQ(tokens.front(), "solitary");
}

TEST_F(ConfigModuleTest, MapCommandToCodeRecognisesKnownCommands) {
  auto& module = ConfigModule::get_instance();
  EXPECT_EQ(module.map_command_to_code(START_FEEDBACK_COMMAND), START_FEEDBACK_CODE);
  EXPECT_EQ(module.map_command_to_code(STOP_FEEDBACK_COMMAND), STOP_FEEDBACK_CODE);
  EXPECT_EQ(module.map_command_to_code("unknown"), -1);
}

TEST_F(ConfigModuleTest, MapCommandToCodeMatchesAllKnownTokens) {
  auto& module = ConfigModule::get_instance();
  struct CommandExpectation {
    const char* token;
    COMMAND_CODE code;
  } expectations[] = {
    {HEALTH_CHECK_COMMAND, HEALTH_CHECK_CODE},
    {AUDIO_HEALTH_CHECK_COMMAND, AUDIO_HEALTH_CHECK_CODE},
    {START_DISCOVERY_COMMAND, START_DISCOVERY_CODE},
    {STOP_DISCOVERY_COMMAND, STOP_DISCOVERY_CODE},
    {GET_DEVICES_COMMAND, GET_DEVICES_CODE},
    {PAIR_DEVICE_COMMAND, PAIR_DEVICE_CODE},
    {CONNECT_DEVICE_COMMAND, CONNECT_DEVICE_CODE},
    {DISCONNECT_DEVICE_COMMAND, DISCONNECT_DEVICE_CODE},
    {START_FEEDBACK_COMMAND, START_FEEDBACK_CODE},
    {STOP_FEEDBACK_COMMAND, STOP_FEEDBACK_CODE},
    {SET_VOLUME_COMMAND, SET_VOLUME_CODE},
    {SET_FEEDBACK_MODE_COMMAND, SET_FEEDBACK_MODE_CODE},
  };

  for (const auto& expectation : expectations) {
    EXPECT_EQ(module.map_command_to_code(expectation.token), expectation.code);
  }
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

TEST_F(ConfigModuleTest, ProcessCommandPairDeviceRequiresAddress) {
  auto& module = ConfigModule::get_instance();
  module.process_command(string(PAIR_DEVICE_COMMAND) + "!");
  EXPECT_EQ(module.get_response_buffer(), "#Error: No device address provided");
}

TEST_F(ConfigModuleTest, ProcessCommandConnectDeviceRequiresAddress) {
  auto& module = ConfigModule::get_instance();
  module.process_command(string(CONNECT_DEVICE_COMMAND) + "!");
  EXPECT_EQ(module.get_response_buffer(), "#Error: No device address provided");
}

TEST_F(ConfigModuleTest, ProcessCommandSetFeedbackModeRequiresArgument) {
  auto& module = ConfigModule::get_instance();
  module.process_command(SET_FEEDBACK_MODE_COMMAND);
  EXPECT_EQ(module.get_response_buffer(), "#Error: No feedback mode provided");
}

TEST_F(ConfigModuleTest, HealthCheckReturnsOkAndClearsBuffer) {
  auto& module = ConfigModule::get_instance();
  module.process_command(HEALTH_CHECK_COMMAND);
  EXPECT_EQ(module.get_response_buffer(), "OK");
  EXPECT_TRUE(module.get_response_buffer().empty());
}

TEST_F(ConfigModuleTest, ProcessCommandStartDiscoverySetsResponse) {
  auto& module = ConfigModule::get_instance();
  module.process_command(START_DISCOVERY_COMMAND);
  EXPECT_EQ(module.get_response_buffer(), "Discovery started");
}

TEST_F(ConfigModuleTest, ProcessCommandStopDiscoverySetsResponse) {
  auto& module = ConfigModule::get_instance();
  module.process_command(STOP_DISCOVERY_COMMAND);
  EXPECT_EQ(module.get_response_buffer(), "Discovery stopped");
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

TEST_F(ConfigModuleTest, AudioHealthCheckReturnsStaticOk) {
  auto& module = ConfigModule::get_instance();
  EXPECT_EQ(module.audio_health_check_command(), "OK");
}

TEST_F(ConfigModuleTest, StartAndStopDiscoveryCommandsAcknowledgeAction) {
  auto& module = ConfigModule::get_instance();
  BlueZDevice placeholder{};
  module.found_devices.push_back(placeholder);
  EXPECT_EQ(module.start_discovery_command(), "Discovery started");
  EXPECT_TRUE(module.found_devices.empty());
  EXPECT_EQ(module.stop_discovery_command(), "Discovery stopped");
}

TEST_F(ConfigModuleTest, GetDevicesCommandFormatsCachedDevices) {
  auto& module = ConfigModule::get_instance();
  module.found_devices.clear();
  BlueZDevice device{};
  strncpy(device.name, "DeviceA", BUFFER_SIZE_L - 1);
  strncpy(device.address, "AA:BB:CC", BUFFER_SIZE_S - 1);
  module.found_devices.push_back(device);

  string response = module.get_devices_command();
  EXPECT_NE(response.find("$DeviceA@AA:BB:CC"), string::npos);
  module.found_devices.clear();
}

TEST_F(ConfigModuleTest, PairDeviceCommandValidatesPresenceInCache) {
  auto& module = ConfigModule::get_instance();
  module.found_devices.clear();
  vector<string> args = {"00:11:22"};
  EXPECT_EQ(module.pair_device_command(args), string("#Error: Device not found"));
}

TEST_F(ConfigModuleTest, ConnectDeviceCommandRejectsMissingAddress) {
  auto& module = ConfigModule::get_instance();
  vector<string> args;
  EXPECT_EQ(module.connect_device_command(args), string("#Error: No device address provided"));
}

TEST_F(ConfigModuleTest, ConnectDeviceCommandValidatesPresenceInCache) {
  auto& module = ConfigModule::get_instance();
  module.found_devices.clear();
  vector<string> args = {"11:22:33"};
  EXPECT_EQ(module.connect_device_command(args), string("#Error: Device not found"));
}

TEST_F(ConfigModuleTest, GetDevicesCommandHandlesEmptyCache) {
  auto& module = ConfigModule::get_instance();
  module.found_devices.clear();
  string response = module.get_devices_command();
  EXPECT_TRUE(response.empty() || response.front() == '$');
}

TEST_F(ConfigModuleTest, DisconnectDeviceCommandWithoutActiveDeviceReturnsError) {
  auto& module = ConfigModule::get_instance();
  auto& audio = BTAudioController::get_instance();

  BlueZDevice* original_device = audio.connected_device;
  audio.connected_device = nullptr;

  EXPECT_EQ(module.disconnect_device_command(), string("#Error: No device connected"));

  audio.connected_device = original_device;
}

TEST_F(ConfigModuleTest, BTControllerParseDevicesExtractsEntries) {
  BTController controller;
  vector<BlueZDevice> devices;

  GVariant* variant = make_device_variant("/org/bluez/hci0/dev_AA_BB_CC", "Speaker", "AA:BB:CC");
  EXPECT_EQ(controller.parse_devices(variant, devices), 0);
  ASSERT_EQ(devices.size(), 1u);
  EXPECT_STREQ(devices[0].name, "Speaker");
  EXPECT_STREQ(devices[0].address, "AA:BB:CC");
  g_variant_unref(variant);

  GVariant* invalid = g_variant_ref_sink(g_variant_new_string("invalid"));
  devices.clear();
  EXPECT_EQ(controller.parse_devices(invalid, devices), -1);
  g_variant_unref(invalid);
}

TEST_F(ConfigModuleTest, BTControllerParseDevicesHandlesMultipleEntries) {
  BTController controller;
  vector<BlueZDevice> devices;

  GVariant* variant = make_devices_variant({{"Speaker", "AA:BB:CC"}, {"Headset", "11:22:33"}});
  EXPECT_EQ(controller.parse_devices(variant, devices), 0);
  ASSERT_EQ(devices.size(), 2u);
  EXPECT_STREQ(devices[0].name, "Speaker");
  EXPECT_STREQ(devices[0].address, "AA:BB:CC");
  EXPECT_STREQ(devices[1].name, "Headset");
  EXPECT_STREQ(devices[1].address, "11:22:33");
  g_variant_unref(variant);
}

TEST_F(ConfigModuleTest, BTControllerParseDevicesSkipsNonDeviceEntries) {
  BTController controller;
  vector<BlueZDevice> devices;

  GVariant* variant = make_non_device_variant("/org/bluez/hci0/dev_dummy");
  EXPECT_EQ(controller.parse_devices(variant, devices), 0);
  EXPECT_TRUE(devices.empty());
  g_variant_unref(variant);
}

TEST_F(ConfigModuleTest, BTControllerParseDevicesSkipsEntriesMissingProperties) {
  BTController controller;
  vector<BlueZDevice> devices;

  GVariant* variant = make_device_variant_without_address("/org/bluez/hci0/dev_missing", "Incomplete");
  EXPECT_EQ(controller.parse_devices(variant, devices), 0);
  EXPECT_TRUE(devices.empty());
  g_variant_unref(variant);
}

TEST_F(ConfigModuleTest, BTControllerPrintDevicesEmitsExpectedOutput) {
  BTController controller;
  vector<BlueZDevice> devices(1);
  strncpy(devices[0].name, "Headset", BUFFER_SIZE_L - 1);
  strncpy(devices[0].address, "11:22:33", BUFFER_SIZE_S - 1);

  testing::internal::CaptureStdout();
  controller.print_devices(devices);
  string output = testing::internal::GetCapturedStdout();
  EXPECT_NE(output.find("Headset"), string::npos);
  EXPECT_NE(output.find("11:22:33"), string::npos);
}

TEST_F(ConfigModuleTest, BTControllerGuardClausesHandleInvalidArguments) {
  BTController controller;
  GVariant* value = g_variant_ref_sink(g_variant_new_boolean(TRUE));
  EXPECT_EQ(controller.set_proxy_property(nullptr, BLUEZ_DEVICE_IFACE, "Powered", value), -1);
  EXPECT_EQ(controller.set_proxy_property(reinterpret_cast<GDBusProxy*>(0x1), nullptr, "Powered", value), -1);
  EXPECT_EQ(controller.set_proxy_property(reinterpret_cast<GDBusProxy*>(0x1), BLUEZ_DEVICE_IFACE, nullptr, value), -1);
  g_variant_unref(value);

  EXPECT_EQ(controller.get_proxy_property(nullptr, BLUEZ_DEVICE_IFACE, "Powered"), nullptr);
  EXPECT_EQ(controller.get_proxy_property(reinterpret_cast<GDBusProxy*>(0x1), nullptr, "Powered"), nullptr);
  EXPECT_EQ(controller.get_proxy_property(reinterpret_cast<GDBusProxy*>(0x1), BLUEZ_DEVICE_IFACE, nullptr), nullptr);
}

TEST_F(ConfigModuleTest, BTAudioControllerGetBooleanValueSafelyHandlesVariants) {
  auto& controller = BTAudioController::get_instance();

  GVariant* true_variant = g_variant_ref_sink(g_variant_new("(v)", g_variant_new_boolean(TRUE)));
  EXPECT_TRUE(controller.get_boolean_value(true_variant));
  g_variant_unref(true_variant);

  EXPECT_FALSE(controller.get_boolean_value(nullptr));
}

TEST_F(ConfigModuleTest, BTAudioControllerGetBooleanValueHandlesFalseVariant) {
  auto& controller = BTAudioController::get_instance();
  GVariant* false_variant = g_variant_ref_sink(g_variant_new("(v)", g_variant_new_boolean(FALSE)));
  EXPECT_FALSE(controller.get_boolean_value(false_variant));
  g_variant_unref(false_variant);
}

TEST_F(ConfigModuleTest, BTAudioControllerCleanupClearsRuntimeState) {
  auto& controller = BTAudioController::get_instance();

  if (controller.main_loop) {
    g_main_loop_unref(controller.main_loop);
  }
  controller.main_loop = g_main_loop_new(nullptr, FALSE);

  vector<BlueZDevice> devices(2);
  controller.connected_device = &devices[0];

  controller.cleanup(devices);

  EXPECT_TRUE(devices.empty());
  EXPECT_EQ(controller.connected_device, nullptr);
  EXPECT_EQ(controller.main_loop, nullptr);

  controller.main_loop = g_main_loop_new(nullptr, FALSE);
}

TEST_F(ConfigModuleTest, IConfigurationFacadeRoundTrip) {
  IConfiguration::set_response_buffer("hello");
  EXPECT_EQ(IConfiguration::get_response_buffer(), "hello");

  IConfiguration::process_command(HEALTH_CHECK_COMMAND);
  EXPECT_EQ(IConfiguration::get_response_buffer(), "OK");

  IConfiguration::disconnect_audio_device();
}

TEST(BLEServerPropertyTest, AdvertisingPropertyHandlersReturnStaticValues) {
  GError* error = nullptr;
  GVariant* type = BLEServer::handle_adv_get_property(nullptr, nullptr, nullptr, nullptr, "Type", nullptr, nullptr);
  ASSERT_NE(type, nullptr);
  EXPECT_STREQ(g_variant_get_string(type, nullptr), "peripheral");
  g_variant_unref(type);

  GVariant* name = BLEServer::handle_adv_get_property(nullptr, nullptr, nullptr, nullptr, "LocalName", nullptr, nullptr);
  ASSERT_NE(name, nullptr);
  EXPECT_STREQ(g_variant_get_string(name, nullptr), DEVICE_NAME);
  g_variant_unref(name);

  GVariant* flags = BLEServer::handle_adv_get_property(nullptr, nullptr, nullptr, nullptr, "ServiceUUIDs", nullptr, nullptr);
  ASSERT_NE(flags, nullptr);
  gsize len = 0;
  const gchar** strv = g_variant_get_strv(flags, &len);
  ASSERT_EQ(len, 1u);
  EXPECT_STREQ(strv[0], SERVICE_UUID);
  g_free(strv);
  g_variant_unref(flags);

  GVariant* appearance = BLEServer::handle_adv_get_property(nullptr, nullptr, nullptr, nullptr, "Appearance", nullptr, nullptr);
  ASSERT_NE(appearance, nullptr);
  EXPECT_EQ(g_variant_get_uint16(appearance), HID_APPEARANCE_CODE);
  g_variant_unref(appearance);

  GVariant* discoverable = BLEServer::handle_adv_get_property(nullptr, nullptr, nullptr, nullptr, "Discoverable", nullptr, nullptr);
  ASSERT_NE(discoverable, nullptr);
  EXPECT_TRUE(g_variant_get_boolean(discoverable));
  g_variant_unref(discoverable);

  GVariant* timeout = BLEServer::handle_adv_get_property(nullptr, nullptr, nullptr, nullptr, "DiscoverableTimeout", nullptr, nullptr);
  ASSERT_NE(timeout, nullptr);
  EXPECT_EQ(g_variant_get_uint16(timeout), 0u);
  g_variant_unref(timeout);

  GVariant* unknown = BLEServer::handle_adv_get_property(nullptr, nullptr, nullptr, nullptr, "Unknown", &error, nullptr);
  EXPECT_EQ(unknown, nullptr);
  ASSERT_NE(error, nullptr);
  g_clear_error(&error);
}

TEST(BLEServerPropertyTest, ServiceAndCharacteristicHandlersValidateInputs) {
  GError* error = nullptr;
  GVariant* uuid = BLEServer::handle_service_get_property(nullptr, nullptr, nullptr, nullptr, "UUID", &error, nullptr);
  ASSERT_NE(uuid, nullptr);
  EXPECT_STREQ(g_variant_get_string(uuid, nullptr), SERVICE_UUID);
  g_variant_unref(uuid);

  GVariant* primary = BLEServer::handle_service_get_property(nullptr, nullptr, nullptr, nullptr, "Primary", &error, nullptr);
  ASSERT_NE(primary, nullptr);
  EXPECT_TRUE(g_variant_get_boolean(primary));
  g_variant_unref(primary);

  GVariant* invalid = BLEServer::handle_service_get_property(nullptr, nullptr, nullptr, nullptr, "Invalid", &error, nullptr);
  EXPECT_EQ(invalid, nullptr);
  ASSERT_NE(error, nullptr);
  g_clear_error(&error);

  GVariant* char_uuid = BLEServer::handle_char_get_property(nullptr, nullptr, nullptr, nullptr, "UUID", &error, nullptr);
  ASSERT_NE(char_uuid, nullptr);
  EXPECT_STREQ(g_variant_get_string(char_uuid, nullptr), CHARACTERISTIC_UUID);
  g_variant_unref(char_uuid);

  GVariant* service = BLEServer::handle_char_get_property(nullptr, nullptr, nullptr, nullptr, "Service", &error, nullptr);
  ASSERT_NE(service, nullptr);
  EXPECT_STREQ(g_variant_get_string(service, nullptr), SERVICE_PATH);
  g_variant_unref(service);

  GVariant* flags = BLEServer::handle_char_get_property(nullptr, nullptr, nullptr, nullptr, "Flags", &error, nullptr);
  ASSERT_NE(flags, nullptr);
  gsize len = 0;
  const gchar** char_flags = g_variant_get_strv(flags, &len);
  ASSERT_EQ(len, 2u);
  EXPECT_STREQ(char_flags[0], "read");
  EXPECT_STREQ(char_flags[1], "write");
  g_free(char_flags);
  g_variant_unref(flags);

  GVariant* char_invalid = BLEServer::handle_char_get_property(nullptr, nullptr, nullptr, nullptr, "Bad", &error, nullptr);
  EXPECT_EQ(char_invalid, nullptr);
  ASSERT_NE(error, nullptr);
  g_clear_error(&error);
}

TEST(BLEServerLifecycleTest, CleanupResetsInternalState) {
  auto& server = BLEServer::get_instance();
  server.cleanup();

  server.main_loop = g_main_loop_new(nullptr, FALSE);

  GError* error = nullptr;
  server.adv_info = g_dbus_node_info_new_for_xml(ADV_XML, &error);
  ASSERT_EQ(error, nullptr);
  error = nullptr;
  server.app_info = g_dbus_node_info_new_for_xml(APP_XML, &error);
  ASSERT_EQ(error, nullptr);
  error = nullptr;
  server.service_info = g_dbus_node_info_new_for_xml(SERVICE_XML, &error);
  ASSERT_EQ(error, nullptr);
  error = nullptr;
  server.char_info = g_dbus_node_info_new_for_xml(CHAR_XML, &error);
  ASSERT_EQ(error, nullptr);

  server.cleanup();

  EXPECT_EQ(server.main_loop, nullptr);
  EXPECT_EQ(server.adv_info, nullptr);
  EXPECT_EQ(server.app_info, nullptr);
  EXPECT_EQ(server.service_info, nullptr);
  EXPECT_EQ(server.char_info, nullptr);
  EXPECT_EQ(server.connection, nullptr);
}
