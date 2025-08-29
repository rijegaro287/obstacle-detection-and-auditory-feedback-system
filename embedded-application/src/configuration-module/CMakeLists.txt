cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(ConfigurationModule)

# AddressSanitizer configuration
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

add_library(configuration_module STATIC
  configuration_module.cpp
)
target_include_directories(configuration_module PUBLIC
  include
  ../control-module/include
)
target_link_libraries(configuration_module PRIVATE control_iface)

add_library(configuration_iface STATIC
  configuration_iface.cpp
)
target_include_directories(configuration_iface PUBLIC include)
target_link_libraries(configuration_iface PRIVATE configuration_module)

# Define the test executable
add_executable(test_configuration_module
  test/test_configuration_module.cpp
)
target_link_libraries(test_configuration_module PRIVATE configuration_module)

# Enable testing for this module
enable_testing()

# Add the test
add_test(NAME test_configuration_module 
         COMMAND test_configuration_module
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
