# Obstacle Detection and Auditory Feedback System

## Table of Contents
- [Overview](#overview)
- [System Highlights](#system-highlights)
- [Hardware Requirements](#hardware-requirements)
- [Repository Layout](#repository-layout)
- [Quick Start](#quick-start)
   - [1. Prepare the Host](#1-prepare-the-host)
   - [2. Generate Audio Datasets (one time)](#2-generate-audio-datasets-one-time)
   - [3. Build the Embedded Application](#3-build-the-embedded-application)
   - [4. Run the Pipeline](#4-run-the-pipeline)
   - [5. Pair a Headset via BLE](#5-pair-a-headset-via-ble)
- [BLE Command Interface](#ble-command-interface)
- [Testing and Quality Gates](#testing-and-quality-gates)
- [Mobile Application](#mobile-application)
- [Documentation and Analysis](#documentation-and-analysis)
- [Additional Resources](#additional-resources)

## Overview
The project delivers a wearable assistive solution that combines a time-of-flight (ToF) depth camera, real-time computer vision, and spatial audio to help people with visual impairments detect and avoid obstacles. A Raspberry Pi 5 performs depth acquisition and obstacle detection in C++, synthesizes directional audio cues (verbal or non-verbal), and streams them over Bluetooth to a headset. A companion Android application exposes a voice-first interface that mirrors the hardware controls, making it possible to configure the device hands-free.

## System Highlights
- Real-time ToF depth capture (240x180) with geometric reasoning to locate salient obstacles within a ~4 m range.
- Multi-threaded C++ pipeline that orchestrates capture, detection, audio rendering, and ALSA playback via a shared control facade.
- Spatial audio renderer based on HRIR convolution (KFR DSP) and Spanish text-to-speech cues generated with Piper TTS.
- Bluetooth Low Energy (BLE) configuration service built on BlueZ D-Bus APIs plus ALSA-based audio streaming to paired headsets.
- Android 13 (API 33) mobile client using Jetpack Compose, BLE GATT, and voice commands in Latin American Spanish.

## Hardware Requirements
- Raspberry Pi 5 (or equivalent SBC running 64-bit Linux) with BLE 5 and ALSA-compatible audio output.
- Arducam Time-of-Flight camera (MIPI CSI-2) plus official Arducam Depth Camera SDK.
- Li-ion battery pack with UPS HAT capable of 5 V / 5 A supply.
- Stereo Bluetooth headset or bone-conduction headphones.

## Repository Layout
| Path | Purpose |
| --- | --- |
| `embedded-application/src` | All C++ modules (configuration, control, capture, detection, feedback, transmission) plus unit tests. |
| `embedded-application/scripts` | Performance plotting utilities and generated figures. |
| `embedded-application/docs/html` | Generated Doxygen documentation (open `index.html`). |
| `embedded-application/test-results` | Coverage reports and performance logs. |
| `mobile-application/app/src` | Android client code (Compose UI, BLE client, voice command processor). |
| `prepare_environment.sh` | Convenience script that installs system packages, Python deps, and Arducam SDK on a fresh Raspberry Pi. |

## Quick Start
### 1. Prepare the Host
The included helper targets Debian-based systems (tested on Raspberry Pi OS 64-bit). Run:
```bash
./prepare_environment.sh
```
This script upgrades the system, installs clang/LLVM, CMake, ALSA, OpenCV, glib/gio, libgtest, and Python tooling, bootstraps a virtual environment at `./venv`, downloads the Piper TTS voice, converts the HRIR dataset to `.npy`, synthesizes reference audio clips, and installs the Arducam ToF SDK.

If you prefer manual setup, ensure the following packages are present:
- `clang clang++ cmake ninja-build llvm llvm-profdata llvm-cov`
- `libopencv-dev libasound2-dev libglib2.0-dev libgtest-dev`
- `python3-venv python3-pip` plus Python deps in `embedded-application/src/feedback-module/install_dependencies.sh`
- Arducam Depth Camera SDK headers and libraries.

### 2. Generate Audio Datasets (one time)
If `embedded-application/src/feedback-module/dataset/*.npy` is missing or you need to regenerate it, run:
```bash
cd embedded-application/src/feedback-module
bash install_dependencies.sh
cd dataset
bash download_dataset.sh
python convert_to_npy.py
python generate_taps.py
python generate_verbal_feedback.py
cd ../../../..
```
This produces:
- `hrirs.npy` and `positions.npy` from the SOFA dataset, filtered to the ToF field of view.
- `tap_alert.npy` for the non-verbal haptic-like cue.
- `verbal_feedback_signals.npy` with Piper-generated Spanish prompts resampled to 48 kHz.

### 3. Build the Embedded Application
```bash
cd embedded-application
./compile.sh
```
CMake generates `build/` and produces the `obstacle-detection-and-auditory-feedback-system` binary alongside per-module static libraries and tests.

### 4. Run the Pipeline
1. Start the service (sudo may be required for BlueZ operations):
   ```bash
   cd embedded-application
   sudo ./build/obstacle-detection-and-auditory-feedback-system
   ```
2. The main process spawns dedicated threads for configuration (BLE), control, capture, detection, feedback, and transmission. Audio is produced only after the mobile client starts feedback mode and a headset is connected.

### 5. Pair a Headset via BLE
- The embedded target exposes a BLE peripheral named `odafs`, with service UUID `9b19df40-4042-4479-0000-131cd24590be` and characteristic UUID `9b19df40-4042-4479-0001-131cd24590be` (read/write).
- Use the Android client (see below) or any BLE GATT tool to issue configuration commands.

## BLE Command Interface
Commands use the format `command!arg1,arg2,...` (arguments optional). Responses are plain-text; errors are prefixed with `#`.

| Command | Description | Typical Response |
| --- | --- | --- |
| `health_check` | Ping BLE service availability. | `OK` |
| `audio_health_check` | Validate ALSA/Bluetooth audio transport. | `OK` or `#Error: ...` |
| `start_discovery` / `stop_discovery` | Control Bluetooth audio device scanning through BlueZ. | Status string |
| `get_devices` | Returns `$name@address` tokens for each discovered sink. | `$Headset@One:Two:...` |
| `pair_device!AA:BB:...` | Pair with a discovered headset. | `Paired with device: ...` |
| `connect_device!AA:BB:...` | Connect and set as active audio target. | `Connected to device: ...` |
| `disconnect_device` | Drop the active connection. | `Disconnected from device: ...` |
| `start_feedback` / `stop_feedback` | Enable or halt the capture-detect-feedback loop. | `Feedback started` / `Feedback stopped` |
| `set_volume!NN` | Set playback gain (0-100). | `Volume set to NN` |
| `set_feedback_mode!non_verbal` or `...!verbal` | Switch audio modality. | `Feedback mode set to ...` |

Command handling is case-sensitive. The configuration module throttles responses using `COMMAND_RETURN_SLEEP_MS` (default 50 ms) to align with GATT read timing.

## Testing and Quality Gates
Run all module tests with coverage instrumentation:
```bash
cd embedded-application
./run_unit_tests.sh
```
This script configures a Debug + coverage build, executes every `test_*` binary, merges LLVM profiles, and generates HTML output at `test-results/coverage/html/index.html` alongside `coverage.info`.

For performance profiling of the end-to-end binary:
```bash
cd embedded-application
./run_performance_tests.sh true   # optional: false to disable logging
```
When logging is enabled, metrics are appended to `perf.log` (CPU%, memory usage, RSS, averages over 100 samples). The `embedded-application/scripts/plot_*` notebooks consume these logs to reproduce the latency figures used in the evaluation.

## Mobile Application
1. Install Android Studio Giraffe (or newer) with Android SDK 33.
2. Open `mobile-application/` or build from the command line:
   ```bash
   cd mobile-application
   ./gradlew assembleDebug
   ```
3. Deploy to a device running Android 13+. The app requests `BLUETOOTH_SCAN`, `BLUETOOTH_CONNECT`, and `ACCESS_FINE_LOCATION`, initializes BLE, and exposes three Compose screens: Connecting, Scanning, and Controls.
4. Voice command UX (press and hold the central command button). Supported Spanish phrases include:
   - "Reanudar" / "Pausar" to start or stop feedback.
   - "Configurar volumen al 60 por ciento" (0-100).
   - "Retroalimentacion verbal" or "retroalimentacion no verbal".
   - "Escanear dispositivos de audio", "Mostrar dispositivos de audio".
   - "Conectar dispositivo de audio numero X", "Desconectar dispositivo de audio".
   - "Ayuda" for an audible cheat-sheet.
5. The app mirrors device state with toasts, haptics, earcons, and synthesized speech (`CommandProcessor.kt`). BLE traffic is serialized through the same command set exposed by the embedded configuration module.

## Documentation and Analysis
Regenerate API docs:
  ```bash
  cd embedded-application
  doxygen Doxyfile
  ```
Open `embedded-application/docs/html/index.html` for module overviews.

## Additional Resources
- [User Manual](docs/user-manual.md)
- [API Reference (Doxygen)](embedded-application/docs/html/index.html)
- [Coverage Report](embedded-application/test-results/coverage/html/index.html)
