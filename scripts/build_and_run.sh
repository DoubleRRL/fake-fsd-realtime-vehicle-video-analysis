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

echo "🚀 Building professional version..."
cmake -f CMakeLists_professional.txt -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/opencv . && make -j$(sysctl -n hw.ncpu)
EXECUTABLE="ProfessionalVideoGUI"

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo ""
    echo "🎥 Running the demo..."
    echo "   Press 'q' to quit, 's' to save a frame"
    echo ""
    
    # Check if video file argument is provided
    if [ -n "$1" ]; then
        ./$EXECUTABLE "$1"
    else
        echo "📹 Usage examples:"
        echo "   $0                           # Professional version (file browser)"
        echo "   $0 /path/to/video.mp4       # Load specific video file"
        echo ""
        echo "🎬 Starting professional version..."
        ./$EXECUTABLE
    fi
else
    echo "❌ Build failed! Please check the error messages above."
    exit 1
fi 