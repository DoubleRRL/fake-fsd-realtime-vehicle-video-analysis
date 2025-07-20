cmake_minimum_required(VERSION 3.20)
project(ProfessionalVideoAnalysis)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenCV REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Add executable
add_executable(ProfessionalVideoAnalysis main.cpp detection_tracker.cpp)

# Include directories
target_include_directories(ProfessionalVideoAnalysis PRIVATE 
    ${OpenCV_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(ProfessionalVideoAnalysis 
    ${OpenCV_LIBS}
    Qt6::Core
    Qt6::Widgets
)

# Set C++ standard
set_target_properties(ProfessionalVideoAnalysis PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Set output directory
set_target_properties(ProfessionalVideoAnalysis PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# macOS specific settings
if(APPLE)
    set_target_properties(ProfessionalVideoAnalysis PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
    )
endif() 