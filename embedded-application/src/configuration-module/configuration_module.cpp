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
	// Load configuration settings
}

void ConfigModule::start() {
	printf("Starting configuration module...\n");

	BLEServer& ble_server = BLEServer::get_instance();
	BTAudioController& bt_audio_controller = BTAudioController::get_instance();

	// thread ble_server_thread(&BLEServer::start, &ble_server);
	thread bt_audio_thread(&BTAudioController::start, &bt_audio_controller);

	// ble_server_thread.join();
	bt_audio_thread.join();
}
