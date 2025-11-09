/**
 * @file configuration_module.cpp
 * @brief Implements the BLE-backed configuration module command handlers.
 */

#include "configuration_module.hpp"
#include "control_iface.hpp"

#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

#include "ble_server.hpp"
#include "bt_audio.hpp"

ConfigModule& ConfigModule::get_instance() {
	static ConfigModule instance;
	return instance;
}

/**
 * @brief Initialize the configuration module internal state.
 */
ConfigModule::ConfigModule() {
	this->response_buffer = "";
	this->found_devices.clear();
}

vector<string> ConfigModule::split(const string& s, char delim) {
	vector<string> elems;
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

int64_t ConfigModule::map_command_to_code(const string& command) {
	if (command == HEALTH_CHECK_COMMAND) return HEALTH_CHECK_CODE;
	else if (command == AUDIO_HEALTH_CHECK_COMMAND) return AUDIO_HEALTH_CHECK_CODE;
	else if (command == START_DISCOVERY_COMMAND) return START_DISCOVERY_CODE;
	else if (command == STOP_DISCOVERY_COMMAND) return STOP_DISCOVERY_CODE;
	else if (command == GET_DEVICES_COMMAND) return GET_DEVICES_CODE;
	else if (command == PAIR_DEVICE_COMMAND) return PAIR_DEVICE_CODE;
	else if (command == CONNECT_DEVICE_COMMAND) return CONNECT_DEVICE_CODE;
	else if (command == DISCONNECT_DEVICE_COMMAND) return DISCONNECT_DEVICE_CODE;
	else if (command == START_FEEDBACK_COMMAND) return START_FEEDBACK_CODE;
	else if (command == STOP_FEEDBACK_COMMAND) return STOP_FEEDBACK_CODE;
	else if (command == SET_VOLUME_COMMAND) return SET_VOLUME_CODE;
	else if (command == SET_FEEDBACK_MODE_COMMAND) return SET_FEEDBACK_MODE_CODE;
	else return -1;
}

string ConfigModule::health_check_command() {
	printf("Health check OK\n");
	return "OK";
}

string ConfigModule::audio_health_check_command() {
	return "OK";

	BlueZDevice *connected_device = BTAudioController::get_instance().connected_device;
	
	if (connected_device == nullptr) {
		printf("No audio device connected\n");
		return "#Error: No audio device connected";
	}

	if (!BTAudioController::get_instance().is_connected(*connected_device)) {
		printf("Audio device not connected\n");
		return "#Error: Audio device not connected";
	}

	printf("Audio health check OK\n");
	return "OK";
}

string ConfigModule::start_discovery_command() {
	printf("Starting discovery...\n");
	this->found_devices.clear();
	BTAudioController::get_instance().start_discovery();
	return "Discovery started";
}

string ConfigModule::stop_discovery_command() {
	printf("Stopping discovery...\n");
	BTAudioController::get_instance().stop_discovery();
	return "Discovery stopped";
}

string ConfigModule::get_devices_command() {
	printf("Getting discovered devices...\n");
	BTAudioController::get_instance().get_discovered_devices(this->found_devices);

	string response = "";
	for (const auto& device : found_devices) {
		string device_info = "$" + string(device.name) + "@" + string(device.address);
		response += device_info;
	}

	return response;
}

string ConfigModule::pair_device_command(vector<string>& args) {
	if (args.size() != 1 || args[0].empty()) {
		printf("No device address provided\n");
		return "#Error: No device address provided";
	}

	printf("Pairing with audio device %s...\n", args[0].c_str());
	string address = args[0];
	int64_t device_idx = BTAudioController::get_instance().find_device_idx(this->found_devices, address);

	if (device_idx < 0) {
		printf("Device not found: %s\n", address.c_str());
		return "#Error: Device not found";
	}

	BlueZDevice& device = this->found_devices[device_idx];

	if (BTAudioController::get_instance().is_paired(device)) {
		return "Device already paired: " + string(device.name);
	}

	if (BTAudioController::get_instance().pair_device(device) < 0) {
		printf("Failed to pair with device: %s\n", device.name);
		return "#Error: Failed to pair with device";
	}

	return "Paired with device: " + string(device.name);
}

string ConfigModule::connect_device_command(vector<string>& args) {
	if (args.size() != 1 || args[0].empty()) {
		printf("No device address provided\n");
		return "#Error: No device address provided";
	}
	
	printf("Connecting to audio device %s...\n", args[0].c_str());
	string address = args[0];
	int64_t device_idx = BTAudioController::get_instance().find_device_idx(this->found_devices, address);

	if (device_idx < 0) {
		printf("Device not found: %s\n", address.c_str());
		return "#Error: Device not found";
	}

	BlueZDevice& device = this->found_devices[device_idx];
	BTAudioController::get_instance().connected_device = &device;

	if (BTAudioController::get_instance().connect_device(device) < 0) {
		BTAudioController::get_instance().connected_device = nullptr;
		return "#Error: Failed to connect to device";
	}

	return "Connected to device: " + string(device.name);
}

string ConfigModule::disconnect_device_command() {
	BlueZDevice *connected_device = BTAudioController::get_instance().connected_device;
	if (connected_device == nullptr) {
		printf("No device connected\n");
		return "#Error: No device connected";
	}

	printf("Disconnecting from audio device %s...\n", connected_device->name);
	if (!BTAudioController::get_instance().is_connected(*connected_device)) {
		return "Device already disconnected: " + string(connected_device->name);
	}

	if (BTAudioController::get_instance().disconnect_device(*connected_device) < 0) {
		printf("Failed to disconnect from device: %s\n", connected_device->name);
		return "#Error: Failed to disconnect from device";
	}

	BTAudioController::get_instance().cleanup(this->found_devices);

	return "Disconnected from device: " + string(connected_device->name);
}

string ConfigModule::start_feedback_command() {
	printf("Starting feedback...\n");
	IControl::start_feedback();
	return "Feedback started";
}

string ConfigModule::stop_feedback_command() {
	printf("Stopping feedback...\n");
	IControl::stop_feedback();
	return "Feedback stopped";
}

string ConfigModule::set_volume_command(vector<string>& args) {
		if (args.size() != 1 || args[0].empty()) {
		printf("No volume value provided\n");
		return "#Error: No volume value provided";
	}
	
	printf("Setting volume to %s...\n", args[0].c_str());
	uint64_t volume = stoi(args[0]);
	IControl::set_volume(volume);

	return "Volume set to " + to_string(volume);
}

string ConfigModule::set_feedback_mode_command(vector<string>& args) {
	if (args.size() != 1 || args[0].empty()) {
		printf("No feedback mode provided\n");
		return "#Error: No feedback mode provided";
	}

	printf("Setting feedback mode to %s...\n", args[0].c_str());
	string mode_string = args[0];
	FEEDBACK_MODES mode;
	if (mode_string == NON_VERBAL_MODE_STRING) {
		mode = NON_VERBAL_MODE;
	}
	else if (mode_string == VERBAL_MODE_STRING) {
		mode = VERBAL_MODE;
	}
	else {
		printf("Invalid feedback mode: %s\n", mode_string.c_str());
		return "#Error: Invalid feedback mode";
	}

	IControl::set_feedback_mode(mode);

	return "Feedback mode set to " + mode_string;
}

void ConfigModule::process_command(const string& command) {
	vector<string> tokens;
	uint64_t command_code;
	string response;

	tokens = this->split(command, '!');
	if (tokens.size() == 0 || tokens.size() > 2) {
		printf("Invalid command format\n");
		response = "#Error: Invalid command format";
		goto set_response;
	}

	command_code = this->map_command_to_code(tokens[0]);
	tokens.erase(tokens.begin());
	
	switch (command_code) {
		case HEALTH_CHECK_CODE: {
			response = this->health_check_command();
			break;
		}
		case AUDIO_HEALTH_CHECK_CODE: {
			IControl::set_received_audio_commands(true);
			response = this->audio_health_check_command();
			break;
		}
		case START_DISCOVERY_CODE: {
			IControl::set_received_audio_commands(true);
			response = this->start_discovery_command();
			break;
		}
		case STOP_DISCOVERY_CODE: {
			IControl::set_received_audio_commands(true);
			response = this->stop_discovery_command();
			break;
		}
		case GET_DEVICES_CODE: {
			IControl::set_received_audio_commands(true);
			response = this->get_devices_command();
			break;
		}
		case PAIR_DEVICE_CODE: {
			IControl::set_received_audio_commands(true);
			response = this->pair_device_command(tokens);
			break;
		}
		case CONNECT_DEVICE_CODE: {
			IControl::set_received_audio_commands(true);
			response = this->connect_device_command(tokens);
			break;
		}
		case DISCONNECT_DEVICE_CODE: {
			response = this->disconnect_device_command();
			break;
		}
		case START_FEEDBACK_CODE: {
			IControl::set_received_audio_commands(true);
			response = this->start_feedback_command();
			break;
		}
		case STOP_FEEDBACK_CODE: {
			IControl::set_received_audio_commands(true);
			response = this->stop_feedback_command();
			break;
		}
		case SET_VOLUME_CODE: {
			IControl::set_received_audio_commands(true);
			response = this->set_volume_command(tokens);
			break;
		}
		case SET_FEEDBACK_MODE_CODE: {
			IControl::set_received_audio_commands(true);
			response = this->set_feedback_mode_command(tokens);
			break;
		}
		default: {
			printf("Unknown command %s\n", command.c_str());
			break;
		}
	}

	set_response:
	std::this_thread::sleep_for(std::chrono::milliseconds(COMMAND_RETURN_SLEEP_MS));
	this->set_response_buffer(response);
}

void ConfigModule::start() {
	BLEServer& ble_server = BLEServer::get_instance();
	BTAudioController& bt_audio_controller = BTAudioController::get_instance();

	ble_server.start();
}

void ConfigModule::set_response_buffer(const string& buffer) {
	this->response_buffer = buffer;
}

string ConfigModule::get_response_buffer() {
	string temp = this->response_buffer;
	this->response_buffer.clear();
	return temp;
}
