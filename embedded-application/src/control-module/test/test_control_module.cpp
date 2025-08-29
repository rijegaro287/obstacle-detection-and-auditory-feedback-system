
#include <thread>
#include <mutex>

#include "control_module.hpp"

using namespace std;

int main() {
  control_module& ctrl_module = control_module::get_instance();
  ctrl_module.start();

  // feedback_module& feedback_module = feedback_module::get_instance();

  return 0;
}
