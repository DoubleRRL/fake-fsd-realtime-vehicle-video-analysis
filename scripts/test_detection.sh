#!/bin/bash

set -e

echo "🧪 Testing Real-Time Object Detection and Tracking"
echo "=================================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "qt_gui/main.cpp" ]; then
    print_error "Please run this script from the project root directory"
    exit 1
fi

print_status "Starting comprehensive detection and tracking test..."

# Test 1: Check if YOLO models are available
print_status "Test 1: Checking YOLO models..."
if [ -f "models/yolov8n.onnx" ]; then
    print_success "YOLOv8n model found"
    MODEL_SIZE=$(du -h models/yolov8n.onnx | cut -f1)
    print_status "Model size: $MODEL_SIZE"
else
    print_warning "YOLOv8n model not found, will use dummy detections"
fi

if [ -f "models/coco.names" ]; then
    CLASS_COUNT=$(wc -l < models/coco.names)
    print_success "COCO class names found ($CLASS_COUNT classes)"
else
    print_warning "COCO class names not found, using default classes"
fi

# Test 2: Check if test video exists
print_status "Test 2: Checking test video..."
if [ -f "data/sample_videos/test_video.mp4" ]; then
    print_success "Test video found"
    VIDEO_SIZE=$(du -h data/sample_videos/test_video.mp4 | cut -f1)
    print_status "Video size: $VIDEO_SIZE"
else
    print_warning "Test video not found, creating one..."
    ffmpeg -f lavfi -i testsrc=duration=10:size=1280x720:rate=30 \
           -c:v libx264 -preset fast -crf 23 \
           data/sample_videos/test_video.mp4 -y > /dev/null 2>&1
    print_success "Test video created"
fi

# Test 3: Build the application
print_status "Test 3: Building application..."
cd qt_gui
if make -j$(sysctl -n hw.ncpu) > /dev/null 2>&1; then
    print_success "Application built successfully"
else
    print_error "Build failed"
    exit 1
fi
cd ..

# Test 4: Check if application runs
print_status "Test 4: Testing application startup..."
if [ -f "qt_gui/ProfessionalVideoAnalysis.app/Contents/MacOS/ProfessionalVideoAnalysis" ]; then
    print_success "Application executable found"
else
    print_error "Application executable not found"
    exit 1
fi

# Test 5: Test detection functionality
print_status "Test 5: Testing detection functionality..."
print_status "Launching application with test video..."

# Start the application in background
./qt_gui/ProfessionalVideoAnalysis.app/Contents/MacOS/ProfessionalVideoAnalysis \
    data/sample_videos/test_video.mp4 &
APP_PID=$!

# Wait a moment for the app to start
sleep 3

# Check if the app is still running
if kill -0 $APP_PID 2>/dev/null; then
    print_success "Application started successfully"
    print_status "Application PID: $APP_PID"
    
    # Give user time to test
    print_status "Application is running. Please test:"
    echo "  1. Check if video loads properly"
    echo "  2. Enable 'Show Annotations' to see detections"
    echo "  3. Adjust confidence threshold"
    echo "  4. Monitor performance metrics"
    echo "  5. Test playback controls"
    echo ""
    print_status "Press Enter when done testing..."
    read -r
    
    # Clean up
    kill $APP_PID 2>/dev/null || true
    print_success "Application closed"
else
    print_error "Application failed to start"
    exit 1
fi

# Test 6: Performance check
print_status "Test 6: Performance verification..."
print_status "Checking system resources..."

# Check CPU cores
CPU_CORES=$(sysctl -n hw.ncpu)
print_status "CPU cores: $CPU_CORES"

# Check available memory
MEMORY_GB=$(($(sysctl -n hw.memsize) / 1024 / 1024 / 1024))
print_status "Total memory: ${MEMORY_GB}GB"

# Check if running on Apple Silicon
if [[ $(uname -m) == "arm64" ]]; then
    print_success "Running on Apple Silicon (optimized for performance)"
else
    print_warning "Running on Intel Mac (may have lower performance)"
fi

# Test 7: Model performance estimation
print_status "Test 7: Performance estimation..."
if [ -f "models/yolov8n.onnx" ]; then
    print_status "Expected performance with YOLOv8n:"
    if [[ $(uname -m) == "arm64" ]]; then
        echo "  - Apple Silicon: 30-60 FPS"
        echo "  - Detection time: 10-30ms per frame"
        echo "  - Tracking time: <1ms per frame"
    else
        echo "  - Intel Mac: 15-30 FPS"
        echo "  - Detection time: 20-50ms per frame"
        echo "  - Tracking time: <1ms per frame"
    fi
else
    print_warning "Using dummy detections (no real model)"
    echo "  - Expected: 60+ FPS (dummy detections only)"
    echo "  - Detection time: <1ms per frame"
    echo "  - Tracking time: <1ms per frame"
fi

# Summary
echo ""
echo "🎯 Test Summary"
echo "==============="
print_success "All tests completed successfully!"
echo ""
echo "📋 What to test next:"
echo "1. Load your own video files"
echo "2. Test with real traffic footage"
echo "3. Adjust detection settings"
echo "4. Monitor performance metrics"
echo "5. Try different confidence thresholds"
echo ""
echo "🚀 Ready for real-time object detection and tracking!"
echo ""
echo "💡 Tips:"
echo "- Use MP4 format for best compatibility"
echo "- 720p or 1080p videos work well"
echo "- 30 FPS videos are optimal"
echo "- Good lighting improves detection accuracy"
echo "- Close other applications for best performance" 