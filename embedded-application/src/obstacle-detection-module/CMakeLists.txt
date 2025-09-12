cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(ObstacleDetectionModule)

# AddressSanitizer configuration
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(ArducamDepthCamera REQUIRED)

set(CLIBS ArducamDepthCamera::ArducamDepthCamera4C)
set(CXXLIBS ArducamDepthCamera::ArducamDepthCamera)

# Define the module executable
#add_executable(obstacle_detection_module obstacle_detection_module.cpp
#               ../image-capture-module/image_capture_module.cpp
#)
#target_include_directories(obstacle_detection_module PRIVATE
#    ${CMAKE_CURRENT_SOURCE_DIR}/include
#    ${CMAKE_CURRENT_SOURCE_DIR}/../image-capture-module/include
#)
#target_link_libraries(obstacle_detection_module PRIVATE ${CXXLIBS} ${OpenCV_LIBS})

# Define the test executable
add_executable(test_obstacle_detection_module test/test_obstacle_detection_module.cpp
               ../image-capture-module/image_capture_module.cpp
               obstacle_detection_module.cpp
               obstacle_detection_iface.cpp)
target_include_directories(test_obstacle_detection_module PUBLIC 
                            ${CMAKE_CURRENT_SOURCE_DIR}/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/../image-capture-module/include
                            ../control-module/include)
target_link_libraries(test_obstacle_detection_module PUBLIC ${CXXLIBS} ${OpenCV_LIBS}
                      control_iface)

# Define the module library
add_library(obstacle_detection_module STATIC
  obstacle_detection_module.cpp
)
target_include_directories(obstacle_detection_module PUBLIC
  include
  ../control-module/include
  ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(obstacle_detection_module PUBLIC
  ${CXXLIBS}
  ${OpenCV_LIBS}
)
target_link_libraries(obstacle_detection_module PRIVATE control_iface)

# Define the interface library
add_library(obstacle_detection_iface STATIC
  obstacle_detection_iface.cpp
)
target_include_directories(obstacle_detection_iface PUBLIC include)
target_link_libraries(obstacle_detection_iface PRIVATE obstacle_detection_module)

# Enable testing for this module
enable_testing()

# Add the test
add_test(NAME test_obstacle_detection_module 
         COMMAND test_obstacle_detection_module
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
