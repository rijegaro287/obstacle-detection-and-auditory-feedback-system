/**
 * @file transmission_iface.cpp
 * @brief Implements the @ref ITransmission façade.
 */

#include "transmission_module.hpp"
#include "transmission_iface.hpp"

/** See @ref ITransmission::start_transmission. */
void ITransmission::start_transmission() {
  TransmissionModule::get_instance().start_transmission();
}

/** See @ref ITransmission::stop_transmission. */
void ITransmission::stop_transmission() {
  TransmissionModule::get_instance().stop_transmission();
}
