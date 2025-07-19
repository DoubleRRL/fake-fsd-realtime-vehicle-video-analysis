#!/bin/bash

set -e

# Print header
clear || true
echo "🚀 Building and Running Real-Time Video Analysis Demo"
echo "=================================================="

# Check if running from project root (look for a file that always exists)
if [ ! -f standalone_demo/CMakeLists_professional.txt ]; then
    echo "❌ Error: Please run this script from the project root directory"
    echo "   Current directory: $(pwd)"
    echo "   Expected file: standalone_demo/CMakeLists_professional.txt"
    exit 1
fi

cd standalone_demo

# Always use the professional CMakeLists
cp CMakeLists_professional.txt CMakeLists.txt

# Build
cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/opencv .
EXECUTABLE="ProfessionalVideoGUI"

# Fix GLFW linking issue if link.txt exists
if [ -f "CMakeFiles/ProfessionalVideoGUI.dir/link.txt" ]; then
    echo "🔧 Fixing GLFW library path..."
    sed -i '' 's/-lglfw/\/opt\/homebrew\/lib\/libglfw.dylib/g' CMakeFiles/ProfessionalVideoGUI.dir/link.txt
fi

make -j$(sysctl -n hw.ncpu)

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo ""
    echo "🎥 Running the professional GUI..."
    echo "   Use the file browser to select a video file."
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
    echo "❌ Build failed!"
    exit 1
fi 