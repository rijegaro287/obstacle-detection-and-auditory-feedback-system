#include "transmission_module.hpp"

snd_pcm_t *pcm_handle;

int main() {
	npy_data verbal_feedback = read_npy<double>("./verbal_feedback_signals.npy");
	uint64_t n_positions = verbal_feedback.shape[0];
	uint64_t n_samples = verbal_feedback.shape[1];

	printf("n_positions: %lu, n_samples: %lu\n", n_positions, n_samples);

	uint64_t position = 5;
	vector<double> signal(n_samples);
	for (uint64_t idx = 0; idx < n_samples; idx++) {
		signal[idx] = verbal_feedback.data[(position * n_samples) + idx];
	}

	printf("Signal size: %lu\n", signal.size());

	vector<double> left_signal(signal.size());
	vector<double> right_signal(signal.size());

	memcpy(left_signal.data(), signal.data(), signal.size() * sizeof(double));
	memcpy(right_signal.data(), signal.data(), signal.size() * sizeof(double));

// ================================
	if (snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
		printf("Error opening PCM device\n");
		return -1;
	}

	int64_t error = snd_pcm_set_params(
		pcm_handle,
		SND_PCM_FORMAT_S16_LE,
		SND_PCM_ACCESS_RW_INTERLEAVED,
		2, 
		22050, 
		1,
		0
	);

	if (error < 0) {
		printf("Error setting PCM parameters: %s\n", snd_strerror(error));
		return -1;
	}

	vector<double> interleaved(left_signal.size() + right_signal.size());
	interleave_audio(left_signal, right_signal, interleaved);

	double max_value = 0.0;
	for (uint64_t i = 0; i < interleaved.size(); i++) {
		if (fabs(interleaved[i]) > max_value) {
			max_value = fabs(interleaved[i]);
		}
	}

	uint64_t CHUNK_N_SAMPLES = 1024;
	uint64_t N_CHUNKS = (interleaved.size() + CHUNK_N_SAMPLES - 1) / CHUNK_N_SAMPLES;

	for (uint64_t chunk_idx = 0; chunk_idx < N_CHUNKS; chunk_idx++) {
		uint64_t start_idx = chunk_idx * CHUNK_N_SAMPLES;
		uint64_t end_idx = min(start_idx + CHUNK_N_SAMPLES, interleaved.size());

		vector<double> interleaved_chunk(interleaved.begin() + start_idx, interleaved.begin() + end_idx);
		vector<int16_t> pcm_chunk(interleaved_chunk.size());

		convert_to_pcm(interleaved_chunk, pcm_chunk, max_value);

		error = snd_pcm_writei(pcm_handle, pcm_chunk.data(), pcm_chunk.size()/2);
		if (error == -EPIPE) {
			snd_pcm_prepare(pcm_handle);
		} 
		else if (error < 0) {
			printf("Error writing to PCM device: %s\n", snd_strerror(error));
		}
	}
	
	snd_pcm_drain(pcm_handle);
	snd_pcm_close(pcm_handle);

 // ========
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
}
