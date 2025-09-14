#include "configuration_module.hpp"
#include "configuration_iface.hpp"

#include <iostream>
#include <thread>
#include <chrono>

#include "ble_server.hpp"
#include "bt_audio.hpp"

#define HEALTH_CHECK_COMMAND "health_check"
#define SCAN_COMMAND "scan"
#define CONNECT_COMMAND "connect"

enum COMMAND_CODE {
	HEALTH_CHECK_CODE,
	SCAN_CODE,
	CONNECT_CODE,
};

ConfigModule& ConfigModule::get_instance() {
	static ConfigModule instance;
	return instance;
}

ConfigModule::ConfigModule() {
	this->response_buffer = "";
}

void ConfigModule::set_response_buffer(const string& buffer) {
	this->response_buffer = buffer;
}

string ConfigModule::get_response_buffer() {
	string temp = this->response_buffer;
	this->response_buffer.clear();
	return temp;
}

vector<string> split(const string& s, char delim) {
	vector<string> elems;
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

vector<BlueZDevice> found_devices;

string health_check_command() {
	printf("Health check OK\n");
	return "OK";
}

string scan_command() {
	printf("Scanning for audio devices...\n");

	BTAudioController::get_instance().scan_devices(found_devices, 3);

	string response = "";
	for (const auto& device : found_devices) {
		string device_info = "$" + string(device.name) + "@" + string(device.address);
		response += device_info;
	}

	return response;
}

string connect_command(vector<string>& args) {
	if (args.size() != 1 || args[0].empty() ) {
		return "#Error: No device address provided";
	}
	
	printf("Connecting to audio device %s...\n", args[0].c_str());
	string address = args[0];
	BlueZDevice* device = BTAudioController::get_instance().find_device(found_devices, address);

	if (device == nullptr) {
		return "#Error: Device not found";
	}

	if (BTAudioController::get_instance().pair_and_connect_device(device) < 0) {
		return "#Error: Failed to connect to device";
	}

	return "Connected to device: " + string(device->name);
}

int64_t map_command_to_code(const string& command) {
	if (command == HEALTH_CHECK_COMMAND) return HEALTH_CHECK_CODE;
	else if (command == SCAN_COMMAND) return SCAN_CODE;
	else if (command == CONNECT_COMMAND) return CONNECT_CODE;
	else return -1;
}

void ConfigModule::process_command(const string& command) {
	printf("Processing command: %s\n", command.c_str());
	vector<string> tokens;
	uint64_t command_code;
	string response;

	tokens = split(command, '!');
	if (tokens.size() == 0 || tokens.size() > 2) {
		printf("Invalid command format\n");
		response = "#Error: Invalid command format";
		goto set_response;
	}

	command_code = map_command_to_code(tokens[0]);
	tokens.erase(tokens.begin());
	switch (command_code) {
		case HEALTH_CHECK_CODE: {
			response = health_check_command();
			break;
		}
		case SCAN_CODE: {
			response = scan_command();
			break;
		}
		case CONNECT_CODE: {
			response = connect_command(tokens);
			break;
		}
		default: {
			printf("Unknown command %s\n", tokens[0].c_str());
			break;
		}
	}

	set_response:
	this->set_response_buffer(response);
}

void ConfigModule::start() {
	printf("Starting configuration module...\n");

	BLEServer& ble_server = BLEServer::get_instance();
	BTAudioController& bt_audio_controller = BTAudioController::get_instance();

	ble_server.start();
}
