#include "transmission_module.hpp"
#include "transmission_iface.hpp"

void ITransmission::start_transmission() {
  TransmissionModule::get_instance().start_transmission();
}

void ITransmission::stop_transmission() {
  TransmissionModule::get_instance().stop_transmission();
}
