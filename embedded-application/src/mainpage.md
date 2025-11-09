# Obstacle Detection and Auditory Feedback System

@tableofcontents

## Overview
The project delivers an embedded C++ pipeline that detects nearby obstacles with a time-of-flight camera and guides the user through spatialized audio feedback. A Bluetooth Low Energy (BLE) controller exposes configuration commands so the system can be orchestrated from a companion device. The CMake-based build targets embedded Linux and links against ALSA, OpenCV, and GLib/BlueZ.

## Runtime Pipeline
1. **Configuration:** The BLE controller receives commands and updates runtime settings via the @ref configuration_module "Configuration Module".
2. **Image Capture:** The @ref image_capture_module "Image Capture Module" acquires depth frames, normalizes them, and publishes results to the shared control state.
3. **Obstacle Detection:** The @ref obstacle_detection_module "Obstacle Detection Module" segments, filters, and scores candidate obstacles in each frame.
4. **Control:** The @ref control_module "Control Module" synchronizes data and life-cycle events across all stages.
5. **Feedback Rendering:** The @ref feedback_module "Feedback Module" converts obstacle descriptors into verbal or non-verbal stereo audio cues.
6. **Transmission:** The @ref transmission_module "Transmission Module" streams the generated audio buffers to the ALSA backend for playback.

Each module exposes a façade interface (`IConfiguration`, `IImageCapture`, `IObstacleDetection`, `IControl`, `IFeedback`, `ITransmission`) to keep dependencies narrow while the module singletons manage internal state.

## Key Features
- **Configurable Feedback Modes:** Switch between verbal prompts and HRIR-based non-verbal cues at runtime.
- **BLE Command Channel:** Issue start/stop, volume, and device-management commands over BLE.
- **Performance Monitoring:** Capture timing samples across pipeline stages for profiling and dashboards.
- **Visualization Hooks:** Optional preview utilities help debug capture and detection stages.

## Developer Guide
- **Build:** Run `./compile.sh` or use the CMake targets under `build/`.
- **Unit Tests:** Execute `./run_unit_tests.sh` to validate module behavior.
- **Performance Tests:** Execute `./run_performance_tests.sh` to profile end-to-end latency.
- **Documentation:** Regenerate the Doxygen site with `doxygen Doxyfile` (output under `docs/html`).

## Additional Resources
- Python scripts under `scripts/` plot performance metrics and benchmarking data.
- Dataset assets for the feedback renderer reside in `src/feedback-module/dataset/`.
- Generated documentation includes class diagrams and module group overviews to aid onboarding.
