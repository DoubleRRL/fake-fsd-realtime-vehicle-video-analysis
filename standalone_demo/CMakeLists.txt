cmake_minimum_required(VERSION 3.20)
project(SimpleVideoDemo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Add executable
add_executable(SimpleVideoDemo simple_video_demo.cpp)

# Link OpenCV libraries
target_link_libraries(SimpleVideoDemo ${OpenCV_LIBS})

# Include directories
target_include_directories(SimpleVideoDemo PRIVATE ${OpenCV_INCLUDE_DIRS}) 