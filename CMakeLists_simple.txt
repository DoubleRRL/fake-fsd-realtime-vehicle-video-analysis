cmake_minimum_required(VERSION 3.20)
project(SimpleVideoProcessor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Add executable
add_executable(SimpleVideoProcessor simple_video_processor.cpp)

# Link OpenCV libraries
target_link_libraries(SimpleVideoProcessor ${OpenCV_LIBS})

# Include directories
target_include_directories(SimpleVideoProcessor PRIVATE ${OpenCV_INCLUDE_DIRS}) 