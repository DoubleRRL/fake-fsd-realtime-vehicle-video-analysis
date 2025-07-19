#!/bin/bash

echo "🚀 Building and Running Real-Time Video Analysis Demo"
echo "=================================================="

# Check if we're in the right directory
if [ ! -f "standalone_demo/simple_video_demo.cpp" ]; then
    echo "❌ Error: Please run this script from the project root directory"
    echo "   Current directory: $(pwd)"
    echo "   Expected files: standalone_demo/simple_video_demo.cpp"
    exit 1
fi

# Go to standalone demo directory
cd standalone_demo

# Check if user wants GUI version
if [ "$1" = "gui" ] || [ "$2" = "gui" ]; then
    echo "🎨 Building GUI version..."
    cmake -f CMakeLists_simple_gui.txt -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/opencv . && make -j$(sysctl -n hw.ncpu)
    EXECUTABLE="SimpleVideoGUI"
elif [ "$1" = "pro" ] || [ "$2" = "pro" ]; then
    echo "🚀 Building professional version..."
    cmake -f CMakeLists_professional.txt -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/opencv . && make -j$(sysctl -n hw.ncpu)
    EXECUTABLE="ProfessionalVideoGUI"
else
    echo "📦 Building simple demo..."
    cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/opencv . && make -j$(sysctl -n hw.ncpu)
    EXECUTABLE="SimpleVideoDemo"
fi

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo ""
    echo "🎥 Running the demo..."
    echo "   Press 'q' to quit, 's' to save a frame"
    echo ""
    
    # Check if camera argument is provided
    if [ "$1" = "camera" ]; then
        ./$EXECUTABLE camera
    elif [ -n "$1" ] && [ "$1" != "gui" ] && [ "$1" != "pro" ]; then
        ./$EXECUTABLE "$1"
    else
        echo "📹 Usage examples:"
        echo "   $0 camera                    # Use webcam (simple demo)"
        echo "   $0 camera gui                # Use webcam (GUI version)"
        echo "   $0 pro                       # Professional version (file browser)"
        echo "   $0 /path/to/video.mp4       # Use video file (simple demo)"
        echo "   $0 /path/to/video.mp4 gui   # Use video file (GUI version)"
        echo ""
        if [ "$EXECUTABLE" = "ProfessionalVideoGUI" ]; then
            echo "🎬 Starting professional version..."
            ./$EXECUTABLE
        else
            echo "🎬 Starting with webcam..."
            ./$EXECUTABLE camera
        fi
    fi
else
    echo "❌ Build failed! Please check the error messages above."
    exit 1
fi 