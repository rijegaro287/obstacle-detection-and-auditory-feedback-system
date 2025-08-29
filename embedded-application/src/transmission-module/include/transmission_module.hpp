#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <math.h>

#include <alsa/asoundlib.h>
#include "npy.hpp"

#include "bluetooth_controller.h"

using namespace npy;
using namespace std;

void convert_to_pcm(const vector<double>& interleaved, vector<int16_t>& pcm, double max_value);
void interleave_audio(const vector<double>& left_channel, const vector<double>& right_channel, vector<double>& interleaved);

	// GError *error = NULL;

	// GMainLoop *loop = g_main_loop_new(NULL, FALSE);
	// GDBusProxy *adapter_proxy = g_dbus_proxy_new_for_bus_sync(
	// 	G_BUS_TYPE_SYSTEM,
	// 	G_DBUS_PROXY_FLAGS_NONE,
	// 	NULL,
	// 	BLUEZ_SERVICE,
	// 	ADAPTER_PATH,
	// 	ADAPTER_INTERFACE,
	// 	NULL,
	// 	&error
	// );
	
	// if (error) {
	// 	printf("Error creating adapter object_manager_proxy: %s\n", error->message);
	// 	g_error_free(error);
	// 	return -1;
	// }

	// BlueZDevice devices[MAX_DEVICES] = {0};
	// int64_t status = 0;
	// while (true) {
	// 	printf("=======================================\n");
	// 	printf("Starting discovery...\n");
	// 	if (start_discovery(adapter_proxy) < 0) {
	// 		printf("Failed to start discovery\n");
	// 		status = -1;
	// 		break;
	// 	}

	// 	this_thread::sleep_for(std::chrono::seconds(3));

	// 	printf("Stopping discovery...\n");
	// 	if (stop_discovery(adapter_proxy) < 0) {
	// 		printf("Failed to stop discovery\n");
	// 		status = -1;
	// 		break;
	// 	}

	// 	int64_t device_count = scan_devices(devices, MAX_DEVICES);
	// 	if (device_count < 0) {
	// 		printf("Failed to scan devices\n");
	// 		status = -1;
	// 		break;
	// 	}

	// 	print_devices(devices, device_count);

	// 	GDBusProxy *device_proxy = NULL;
	// 	for (uint64_t idx = 0; idx < device_count; idx++) {
	// 		BlueZDevice device = devices[idx];
	// 		if (strcmp(device.name, "QCY H3") == 0) {
	// 			printf("Connecting to device: %s (%s)\n", device.name, device.address);

	// 			device_proxy = create_device_proxy(device);
	// 			if (device_proxy == NULL) {
	// 				printf("Failed to create device proxy\n");
	// 				status = -1;
	// 				break;
	// 			}

	// 			if (!is_paired(device_proxy)) {
	// 				printf("Device not paired, pairing...\n");
	// 				if (pair_device(device_proxy) < 0) {
	// 					printf("Failed to pair to device\n");
	// 					status = -1;
	// 					break;
	// 				}
	// 			}
	// 			printf("Device paired, connecting...\n");
				
	// 			if (!is_connected(device_proxy)) {
	// 				printf("Device not connected, connecting...\n");
	// 				if (connect_to_device_profile(device_proxy, A2DP_SINK_UUID) < 0) {
	// 					printf("Failed to connect to profile\n");
	// 					status = -1;
	// 					break;
	// 				}
	// 			}
	// 			printf("Device connected successfully!\n");
	// 		}
	// 	}

	// 	if (device_proxy) g_object_unref(device_proxy);
	// 	clear_devices(devices);
	// 	printf("=======================================\n");
	// 	this_thread::sleep_for(std::chrono::seconds(1));
	// }

	// g_main_loop_unref(loop);
	// g_object_unref(adapter_proxy);

	// return status;