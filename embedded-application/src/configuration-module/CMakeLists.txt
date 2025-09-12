cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

project(ConfigurationModule)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)

# AddressSanitizer configuration
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

add_library(bluetooth_controller STATIC
  bt-controller/bt_controller.cpp
  bt-controller/ble_server.cpp
  bt-controller/bt_audio.cpp
)
target_include_directories(bluetooth_controller PUBLIC
  include/bt-controller
  ${GLIB_INCLUDE_DIRS}
  ${GIO_INCLUDE_DIRS}
)
target_link_libraries(bluetooth_controller PUBLIC
  ${GLIB_LIBRARIES}
  ${GIO_LIBRARIES}
)

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

add_library(configuration_iface STATIC
  configuration_iface.cpp
)
target_include_directories(configuration_iface PUBLIC include)
target_link_libraries(configuration_iface PUBLIC configuration_module)

# # Define the test executable
# add_executable(test_configuration_module
#   test/test_configuration_module.cpp
# )
# target_link_libraries(test_configuration_module PUBLIC configuration_module)

# # Enable testing for this module
# enable_testing()

# # Add the test
# add_test(NAME test_configuration_module 
#          COMMAND test_configuration_module
#          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
