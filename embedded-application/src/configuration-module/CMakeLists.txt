cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(ConfigurationModule)

# AddressSanitizer configuration
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")



# Define the module executable
add_executable(configuration_module configuration_module.cpp)
target_include_directories(configuration_module PRIVATE include)

# Define the test executable
add_executable(test_configuration_module test/test_configuration_module.cpp)
target_include_directories(test_configuration_module PRIVATE include)

# Enable testing for this module
enable_testing()

# Add the test
add_test(NAME test_configuration_module 
         COMMAND test_configuration_module
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
