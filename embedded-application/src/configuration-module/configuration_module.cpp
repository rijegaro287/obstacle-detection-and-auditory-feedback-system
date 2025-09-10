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

	BLEServer::start();

	// thread ble_server_thread(&BLEServer::start);
	// thread bt_audio_thread(&BTAudioController::start);

	// while (true) {
	// 	printf("Configuration module running...\n");
	// 	std::this_thread::sleep_for(std::chrono::seconds(1));
	// }

	// ble_server_thread.join();
	// bt_audio_thread.join();
}
