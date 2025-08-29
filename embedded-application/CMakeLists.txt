cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(ObstacleDetectionAndAuditoryFeedbackSystem)

# AddressSanitizer configuration
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

# Add all modules as subdirectories
# add_subdirectory(src/image-capture-module)
# add_subdirectory(src/obstacle-detection-module)
add_subdirectory(src/feedback-module)
add_subdirectory(src/transmission-module)
# add_subdirectory(src/configuration-module)
# add_subdirectory(src/control-module)

# Create main executable that includes all modules
# Uncomment the following lines if you want to create a main application that uses all modules.
# main_application cannot be built if some module contains a main function.

add_executable(main_application
  src/main.cpp
#   src/image-capture-module/image_capture_module.cpp
#   src/obstacle-detection-module/obstacle_detection_module.cpp
  src/feedback-module/feedback_module.cpp
  src/transmission-module/transmission_module.cpp
#   src/configuration-module/configuration_module.cpp
  # src/control-module/control_module.cpp
)

# Include directories for all modules
target_include_directories(main_application PRIVATE 
#   src/image-capture-module/include
#   src/obstacle-detection-module/include
  src/feedback-module/include
  src/transmission-module/include
#   src/configuration-module/include
  # src/control-module/include
)

# Enable testing at the top level
# enable_testing()
