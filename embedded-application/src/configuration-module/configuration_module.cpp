#include "configuration_module.hpp"
#include "configuration_iface.hpp"

#include "bluetooth_controller.h"

#include <iostream>

ConfigModule& ConfigModule::get_instance() {
	static ConfigModule instance;
	return instance;
}

ConfigModule::ConfigModule() {
	// Load configuration settings
}

void ConfigModule::start() {
    printf("Starting configuration module...\n");
}
