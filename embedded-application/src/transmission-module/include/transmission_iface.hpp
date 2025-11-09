#pragma once

/**
 * @file transmission_iface.hpp
 * @brief Public façade delegating audio transmission commands to the
 * @ref TransmissionModule singleton without exposing implementation details.
 */

/**
 * @class ITransmission
 * @ingroup transmission_module
 * @brief Static wrapper used by other modules to request audio streaming
 * operations from the transmission subsystem.
 */
class ITransmission {
private:
public:
  /** @brief Proxy to @ref TransmissionModule::start_transmission. */
  static void start_transmission();
  /** @brief Proxy to @ref TransmissionModule::stop_transmission. */
  static void stop_transmission();
};
