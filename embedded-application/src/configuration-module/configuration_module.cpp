#include "configuration_module.hpp"
#include "configuration_iface.hpp"

#include <iostream>
#include <thread>
#include <chrono>

#include "ble_server.hpp"
#include "bt_audio.hpp"

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

void ConfigModule::process_command(const string& command) {
	printf("Processing command: %s\n", command.c_str());
	this->set_response_buffer(command);
}

void ConfigModule::start() {
	printf("Starting configuration module...\n");

	BLEServer& ble_server = BLEServer::get_instance();
	BTAudioController& bt_audio_controller = BTAudioController::get_instance();

	thread ble_server_thread(&BLEServer::start, &ble_server);
	// thread bt_audio_thread(&BTAudioController::start, &bt_audio_controller);

	ble_server_thread.join();
	// bt_audio_thread.join();
}
