#!/bin/bash

echo "🚗 Setting up Real-time Vehicle Detection System"
echo "================================================"

# Check if conda is available
if ! command -v conda &> /dev/null; then
    echo "❌ Conda is not installed. Please install Anaconda or Miniconda first."
    echo "   Download from: https://docs.conda.io/en/latest/miniconda.html"
    exit 1
fi

# Create conda environment
echo "📦 Creating conda environment..."
conda create -n vehicle_detection python=3.10 -y

# Activate environment and install packages
echo "🔧 Installing dependencies..."
conda activate vehicle_detection && pip install ultralytics supervision torch torchvision opencv-python

# Test installation
echo "🧪 Testing installation..."
conda activate vehicle_detection && python -c "
try:
    from ultralytics import YOLO
    import supervision as sv
    import cv2
    import torch
    print('✅ All dependencies installed successfully!')
except ImportError as e:
    print(f'❌ Import error: {e}')
    exit(1)
"

echo ""
echo "🎉 Setup complete!"
echo ""
echo "To use the vehicle detection system:"
echo "1. Activate the environment: conda activate vehicle_detection"
echo "2. Run detection: python vehicle_detector.py --source your_video.mp4"
echo "3. Use webcam: python vehicle_detector.py --source 0"
echo ""
echo "For help: python vehicle_detector.py --help" 