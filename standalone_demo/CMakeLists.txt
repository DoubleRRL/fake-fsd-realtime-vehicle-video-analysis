cmake_minimum_required(VERSION 3.20)
project(ProfessionalVideoGUI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenCV REQUIRED)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Add executable
add_executable(ProfessionalVideoGUI professional_gui_demo.cpp)

# Include directories
target_include_directories(ProfessionalVideoGUI PRIVATE 
    ${OpenCV_INCLUDE_DIRS}
    /opt/homebrew/include
)

# Link libraries
target_link_libraries(ProfessionalVideoGUI 
    ${OpenCV_LIBS}
    glfw
    OpenGL::GL
)

# Compiler flags to silence OpenGL deprecation warnings
target_compile_definitions(ProfessionalVideoGUI PRIVATE GL_SILENCE_DEPRECATION)

# Set C++ standard
set_target_properties(ProfessionalVideoGUI PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
) 