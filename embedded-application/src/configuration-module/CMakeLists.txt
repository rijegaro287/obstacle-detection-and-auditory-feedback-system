cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

project(ConfigurationModule)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)

find_package(GTest REQUIRED)
include(GoogleTest)

# AddressSanitizer configuration
if(NOT COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
endif()

add_library(bluetooth_controller STATIC
  bt-controller/bt_controller.cpp
  bt-controller/ble_server.cpp
  bt-controller/bt_audio.cpp
)
target_include_directories(bluetooth_controller PUBLIC
  include
  include/bt-controller
  ${GLIB_INCLUDE_DIRS}
  ${GIO_INCLUDE_DIRS}
)
target_link_libraries(bluetooth_controller PUBLIC
  configuration_iface
  ${GLIB_LIBRARIES}
  ${GIO_LIBRARIES}
)
enable_coverage(bluetooth_controller)

add_library(configuration_module STATIC
  configuration_module.cpp
)
target_include_directories(configuration_module PUBLIC
  include
  include/bt-controller
  ../control-module/include
)
target_link_libraries(configuration_module PUBLIC
  bluetooth_controller
  control_iface
)
enable_coverage(configuration_module)

add_library(configuration_iface STATIC
  configuration_iface.cpp
)
target_include_directories(configuration_iface PUBLIC include)
target_link_libraries(configuration_iface PUBLIC configuration_module)
enable_coverage(configuration_iface)

# Define the test executable
enable_testing()

add_executable(test_configuration_module 
  test/test_configuration_module.cpp
)
target_link_libraries(test_configuration_module PRIVATE 
  GTest::gtest_main
  control_module
)
gtest_discover_tests(test_configuration_module
  DISCOVERY_TIMEOUT 30
)
enable_coverage(test_configuration_module)
