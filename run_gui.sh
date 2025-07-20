#!/bin/bash

# Vehicle Detection GUI Launcher
echo "🚗 Starting Vehicle Detection GUI..."

# Check if we're in the right directory
if [ ! -f "gui.py" ]; then
    echo "❌ Error: gui.py not found in current directory"
    echo "Please run this script from the 'realtime car vision' directory"
    exit 1
fi

# Check if vehicle_detector.py exists
if [ ! -f "vehicle_detector.py" ]; then
    echo "❌ Error: vehicle_detector.py not found"
    echo "Please run this script from the 'realtime car vision' directory"
    exit 1
fi

# Check if YOLO models exist
if [ ! -f "yolov8n.pt" ]; then
    echo "❌ Error: YOLO models not found"
    echo "Please download YOLO models first"
    exit 1
fi

echo "✅ All files found"
echo "🎯 Launching GUI..."

# Run the GUI
python gui.py 