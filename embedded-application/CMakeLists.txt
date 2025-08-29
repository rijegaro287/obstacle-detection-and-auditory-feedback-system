cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ObstacleDetectionAndAuditoryFeedbackSystem)

# AddressSanitizer configuration
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

# Add all modules as subdirectories
# add_subdirectory(src/image-capture-module)
# add_subdirectory(src/obstacle-detection-module)
add_subdirectory(src/control-module)
add_subdirectory(src/feedback-module)
add_subdirectory(src/transmission-module)
add_subdirectory(src/configuration-module)

# Create main executable that includes all modules
# Uncomment the following lines if you want to create a main application that uses all modules.
# main_application cannot be built if some module contains a main function.

add_executable(main_application
  src/main.cpp
)
target_link_libraries(main_application PRIVATE
  control_module
  configuration_module
  feedback_module
  transmission_module
)

# Enable testing at the top level
enable_testing()
