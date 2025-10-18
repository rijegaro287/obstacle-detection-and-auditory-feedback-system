#include "configuration_module.hpp"

#include <gtest/gtest.h>

using namespace std;

TEST(ConfigModuleTest, BasicFunctionality) {
  ConfigModule& cm = ConfigModule::get_instance();
}
