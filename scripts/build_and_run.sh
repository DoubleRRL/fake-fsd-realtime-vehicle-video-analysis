#!/bin/bash

set -e

# Print header
clear || true
echo "🚀 Building and Running Professional Video Analysis"
echo "=================================================="

# Check if running from project root (look for a file that always exists)
if [ ! -f qt_gui/CMakeLists.txt ]; then
    echo "❌ Error: Please run this script from the project root directory"
    echo "   Current directory: $(pwd)"
    echo "   Expected file: qt_gui/CMakeLists.txt"
    exit 1
fi

# Check if Qt6 is installed
if ! brew list qt@6 >/dev/null 2>&1; then
    echo "❌ Qt6 not found. Installing Qt6..."
    brew install qt@6
fi

cd qt_gui

# Build the Qt application
echo "🔨 Building Qt application..."
cmake -DCMAKE_PREFIX_PATH="/opt/homebrew/opt/opencv;/opt/homebrew/opt/qt@6" .
make -j$(sysctl -n hw.ncpu)

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo ""
    echo "🎥 Running Professional Video Analysis..."
    echo "   Features:"
    echo "   - File browser for easy video selection"
    echo "   - Real-time video playback"
    echo "   - Annotation overlay support"
    echo "   - Performance monitoring"
    echo ""
    
    # Check if video file argument is provided
    if [ -n "$1" ]; then
        ./ProfessionalVideoAnalysis.app/Contents/MacOS/ProfessionalVideoAnalysis "$1"
    else
        echo "📹 Usage examples:"
        echo "   $0                           # Open with file browser"
        echo "   $0 /path/to/video.mp4       # Load specific video file"
        echo ""
        echo "🎬 Starting application..."
        ./ProfessionalVideoAnalysis.app/Contents/MacOS/ProfessionalVideoAnalysis
    fi
else
    echo "❌ Build failed!"
    exit 1
fi 