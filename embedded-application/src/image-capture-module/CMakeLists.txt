cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(ImageCaptureModule)

# AddressSanitizer configuration
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(ArducamDepthCamera REQUIRED)

set(CLIBS ArducamDepthCamera::ArducamDepthCamera4C)
set(CXXLIBS ArducamDepthCamera::ArducamDepthCamera)

# Define the module library
add_library(image_capture_module STATIC image_capture_module.cpp)
target_include_directories(image_capture_module PUBLIC include)
target_link_libraries(image_capture_module PRIVATE ${CXXLIBS} ${OpenCV_LIBS})

# Define the test executable
add_executable(test_image_capture_module test/test_image_capture_module.cpp image_capture_iface.cpp)
target_include_directories(test_image_capture_module PRIVATE include)
target_link_libraries(test_image_capture_module PRIVATE image_capture_module ${CXXLIBS} ${OpenCV_LIBS})

# Define preview depth executable
add_executable(preview_depth preview_depth.cpp)
target_include_directories(preview_depth PRIVATE include)
target_link_libraries(preview_depth PRIVATE image_capture_module ${CXXLIBS} ${OpenCV_LIBS})

# Enable testing for this module
enable_testing()
add_test(NAME test_image_capture_module 
         COMMAND test_image_capture_module
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
