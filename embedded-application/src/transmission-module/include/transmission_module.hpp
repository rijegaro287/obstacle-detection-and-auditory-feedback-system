#pragma once

#include "control_iface.hpp"

#include <iostream>
#include <vector>

#include <alsa/asoundlib.h>

using namespace std;

/**
 * @file transmission_module.hpp
 * @brief ALSA-based audio output module that streams feedback buffers to the
 * default PCM device.
 */

/**
 * @defgroup transmission_module Transmission Module
 * @brief Handles ALSA-backed audio playback for spatialized feedback cues.
 * @{
 */

/**
 * @class TransmissionModule
 * @ingroup transmission_module
 * @brief Singleton that consumes audio data from @ref IControl and pushes it
 * to the ALSA PCM backend, handling interleaving, conversion, and streaming.
 */
class TransmissionModule {
public:
  TransmissionModule(const TransmissionModule&) = delete;
  TransmissionModule& operator=(const TransmissionModule&) = delete;
  TransmissionModule(TransmissionModule&&) = delete;
  TransmissionModule& operator=(TransmissionModule&&) = delete;

  /**
   * @brief Retrieve the singleton instance responsible for audio transmission.
   * @return Reference to the transmission module.
   */
  static TransmissionModule& get_instance();

  /** @brief Enable playback of incoming audio buffers. */
  void start_transmission();
  /** @brief Disable playback of incoming audio buffers. */
  void stop_transmission();

  /** @brief Worker loop that continuously streams audio when enabled. */
  void start();
private:
  bool running;            /**< Indicates whether audio transmission is active. */
  snd_pcm_t *pcm_handle;   /**< ALSA PCM handle used for playback. */

  /** @brief Initialize the ALSA device using default 48 kHz stereo settings. */
  TransmissionModule();
  /** @brief Drain and release the ALSA resources on shutdown. */
  ~TransmissionModule();

  /**
   * @brief Convert interleaved floating-point samples to 16-bit PCM.
   * @param interleaved Stereo samples arranged as L, R pairs.
   * @param pcm Output buffer receiving the converted integers.
   * @param max_value Peak absolute value observed across the signal.
   * @param gain Gain factor to apply before quantization.
   */
  void convert_to_pcm(const vector<float>& interleaved, vector<int16_t>& pcm, float max_value, float gain);
  /**
   * @brief Interleave independent left and right channels into a single buffer.
   * @param left_channel Samples for the left channel.
   * @param right_channel Samples for the right channel.
   * @param interleaved Output buffer storing the interleaved samples.
   */
  void interleave_audio(const vector<float>& left_channel, const vector<float>& right_channel, vector<float>& interleaved);
  /**
   * @brief Prepare a slice of audio for playback, including interleaving and conversion.
   * @param signal Full stereo audio payload retrieved from the control module.
   * @param pcm Output buffer receiving the 16-bit PCM data.
   * @param max_value Peak absolute value across the entire signal.
   * @param start_idx First sample index to process (inclusive).
   * @param end_idx Last sample index to process (exclusive).
   */
  void preprocess_audio(Audio& signal, vector<int16_t>& pcm, float max_value, uint64_t start_idx, uint64_t end_idx);
  /**
   * @brief Write a processed PCM buffer to the ALSA device.
   * @param pcm Interleaved 16-bit samples ready for playback.
   * @param sample_rate Sample rate associated with the buffer.
   */
  void send_pcm_data(vector<int16_t>& pcm, uint64_t sample_rate);
  /**
   * @brief Stream an entire stereo buffer to ALSA in fixed-size chunks.
   * @param signal Stereo audio payload to transmit.
   */
  void send_audio(Audio& signal);
};

/// @}
