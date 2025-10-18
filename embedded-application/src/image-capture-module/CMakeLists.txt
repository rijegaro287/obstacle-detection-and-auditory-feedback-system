cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(ImageCaptureModule)

# AddressSanitizer configuration
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(ArducamDepthCamera REQUIRED)

find_package(GTest REQUIRED)
include(GoogleTest)

set(CLIBS ArducamDepthCamera::ArducamDepthCamera4C)
set(CXXLIBS ArducamDepthCamera::ArducamDepthCamera)

# Define the module library
add_library(image_capture_module STATIC 
  image_capture_module.cpp
)
target_include_directories(image_capture_module PUBLIC 
  include
  ../control-module/include
  ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(image_capture_module PUBLIC
  ${CXXLIBS}
  ${OpenCV_LIBS}
)
target_link_libraries(image_capture_module PRIVATE control_iface)

# Define the interface library
add_library(image_capture_iface STATIC
  image_capture_iface.cpp
)
target_include_directories(image_capture_iface PUBLIC include)
target_link_libraries(image_capture_iface PRIVATE image_capture_module)

# Define the test executable
add_executable(test_image_capture_module 
  test/test_image_capture_module_gtest.cpp  
)
target_link_libraries(test_image_capture_module PRIVATE 
  GTest::gtest_main
  image_capture_module
)
gtest_discover_tests(test_image_capture_module
  DISCOVERY_TIMEOUT 30
)

# Define preview depth executable
add_executable(preview_depth preview_depth.cpp)
target_include_directories(preview_depth PRIVATE include)
target_link_libraries(preview_depth PRIVATE image_capture_module ${CXXLIBS} ${OpenCV_LIBS})
