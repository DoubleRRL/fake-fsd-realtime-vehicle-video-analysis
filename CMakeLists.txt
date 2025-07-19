cmake_minimum_required(VERSION 3.20)
project(RealTimeVideoAnalysis)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Find required packages
find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)

# Add source files
set(SOURCES
    src/main.cpp
    src/core/Pipeline.cpp
    src/core/BufferPool.cpp
    src/core/PerformanceMonitor.cpp
    src/modules/VideoInputModule.cpp
    src/modules/PreprocessingModule.cpp
    src/modules/DetectionModule.cpp
    src/modules/TrackingModule.cpp
    src/modules/LabelingModule.cpp
    src/modules/PredictionModule.cpp
    src/modules/RenderingModule.cpp
    src/modules/GUIModule.cpp
    src/utils/MetalUtils.cpp
    src/utils/CoreMLUtils.cpp
    src/utils/MathUtils.cpp
    src/utils/ModelConverter.cpp
)

# Add executables
add_executable(RealTimeVideoAnalysis ${SOURCES})

# Add model converter executable
add_executable(ModelConverter 
    src/utils/ModelConverter.cpp
    src/utils/model_converter_main.cpp
)

# Add benchmark executable
add_executable(Benchmark
    src/utils/BenchmarkRunner.cpp
    src/utils/benchmark_main.cpp
)

# Include directories
target_include_directories(RealTimeVideoAnalysis PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/modules
    ${CMAKE_SOURCE_DIR}/src/utils
    ${OpenCV_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(RealTimeVideoAnalysis
    ${OpenCV_LIBS}
    Eigen3::Eigen
    ${GLFW_LIBRARIES}
    "-framework AVFoundation"
    "-framework CoreML"
    "-framework Metal"
    "-framework MetalKit"
    "-framework Accelerate"
    "-framework CoreGraphics"
    "-framework CoreText"
    "-framework OpenGL"
    "-framework CoreVideo"
    "-framework CoreMedia"
    "-framework VideoToolbox"
)

# Link libraries for model converter
target_link_libraries(ModelConverter
    "-framework CoreML"
    "-framework Foundation"
    "-framework Accelerate"
)

# Link libraries for benchmark
target_link_libraries(Benchmark
    ${OpenCV_LIBS}
    "-framework CoreML"
    "-framework Foundation"
    "-framework Accelerate"
    "-framework AVFoundation"
    "-framework Metal"
    "-framework MetalKit"
)

# Compiler flags for optimization
target_compile_options(RealTimeVideoAnalysis PRIVATE
    -O3
    -march=native
    -flto
    -ffast-math
    -DNDEBUG
    -Wall
    -Wextra
)

# Enable Metal shaders compilation
add_custom_command(TARGET RealTimeVideoAnalysis POST_BUILD
    COMMAND xcrun -sdk macosx metal -c ${CMAKE_SOURCE_DIR}/shaders/*.metal -o ${CMAKE_BINARY_DIR}/shaders.air
    COMMAND xcrun -sdk macosx metallib ${CMAKE_BINARY_DIR}/shaders.air -o ${CMAKE_BINARY_DIR}/shaders.metallib
    COMMENT "Compiling Metal shaders"
)

# Copy resources to build directory
add_custom_command(TARGET RealTimeVideoAnalysis POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/models ${CMAKE_BINARY_DIR}/models
    COMMENT "Copying resources to build directory"
)

# Install targets
install(TARGETS RealTimeVideoAnalysis ModelConverter Benchmark DESTINATION bin)
install(DIRECTORY resources/ DESTINATION share/RealTimeVideoAnalysis/resources)
install(DIRECTORY models/ DESTINATION share/RealTimeVideoAnalysis/models) 