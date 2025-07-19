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

# Check if user wants professional version
if [ "$1" = "pro" ] || [ "$2" = "pro" ]; then
    echo "🚀 Building professional version..."
    cmake -f CMakeLists_professional.txt -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/opencv . && make -j$(sysctl -n hw.ncpu)
    EXECUTABLE="ProfessionalVideoGUI"
else
    echo "📦 Building demo..."
    cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/opencv . && make -j$(sysctl -n hw.ncpu)
    EXECUTABLE="SimpleVideoDemo"
fi

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo ""
    echo "🎥 Running the demo..."
    echo "   Press 'q' to quit, 's' to save a frame"
    echo ""
    
    # Check if video file argument is provided
    if [ -n "$1" ] && [ "$1" != "pro" ]; then
        ./$EXECUTABLE "$1"
    else
        echo "📹 Usage examples:"
        echo "   $0 pro                       # Professional version (file browser)"
        echo "   $0 /path/to/video.mp4       # Use video file (demo)"
        echo ""
        if [ "$EXECUTABLE" = "ProfessionalVideoGUI" ]; then
            echo "🎬 Starting professional version..."
            ./$EXECUTABLE
        else
            echo "🎬 Starting demo version..."
            ./$EXECUTABLE
        fi
    fi
else
    echo "❌ Build failed! Please check the error messages above."
    exit 1
fi 