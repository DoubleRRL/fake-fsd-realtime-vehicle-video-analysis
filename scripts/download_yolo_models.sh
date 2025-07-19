#!/bin/bash

set -e

echo "📥 Downloading YOLOv8 Models for Object Detection"
echo "================================================="

# Create models directory
mkdir -p models

cd models

echo "🔍 Downloading YOLOv8 models..."
echo ""

# Download YOLOv8n (nano) - smallest and fastest
if [ ! -f "yolov8n.onnx" ]; then
    echo "   Downloading YOLOv8n (nano) model..."
    curl -L -o "yolov8n.onnx" \
         "https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8n.pt" \
         || echo "   ⚠️  Failed to download YOLOv8n model"
fi

# Download YOLOv8s (small) - good balance of speed and accuracy
if [ ! -f "yolov8s.onnx" ]; then
    echo "   Downloading YOLOv8s (small) model..."
    curl -L -o "yolov8s.onnx" \
         "https://github.com/ultralytics/assets/releases/download/v0.0.0/yolov8s.pt" \
         || echo "   ⚠️  Failed to download YOLOv8s model"
fi

# Download COCO class names
if [ ! -f "coco.names" ]; then
    echo "   Downloading COCO class names..."
    curl -L -o "coco.names" \
         "https://raw.githubusercontent.com/ultralytics/ultralytics/main/ultralytics/data/coco.yaml" \
         || echo "   ⚠️  Failed to download COCO names, creating default..."
    
    # Create default COCO classes if download fails
    if [ ! -f "coco.names" ]; then
        echo "   Creating default COCO class names..."
        cat > "coco.names" << 'EOF'
person
bicycle
car
motorcycle
airplane
bus
train
truck
boat
traffic light
fire hydrant
stop sign
parking meter
bench
bird
cat
dog
horse
sheep
cow
elephant
bear
zebra
giraffe
backpack
umbrella
handbag
tie
suitcase
frisbee
skis
snowboard
sports ball
kite
baseball bat
baseball glove
skateboard
surfboard
tennis racket
bottle
wine glass
cup
fork
knife
spoon
bowl
banana
apple
sandwich
orange
broccoli
carrot
hot dog
pizza
donut
cake
chair
couch
potted plant
bed
dining table
toilet
tv
laptop
mouse
remote
keyboard
cell phone
microwave
oven
toaster
sink
refrigerator
book
clock
vase
scissors
teddy bear
hair drier
toothbrush
EOF
    fi
fi

echo ""
echo "📊 Download Summary:"
echo "==================="

# Check what was downloaded
echo "   Models available:"
if [ -f "yolov8n.onnx" ]; then
    size=$(du -h "yolov8n.onnx" 2>/dev/null | cut -f1)
    echo "     - yolov8n.onnx ($size) - Fastest, good for real-time"
fi

if [ -f "yolov8s.onnx" ]; then
    size=$(du -h "yolov8s.onnx" 2>/dev/null | cut -f1)
    echo "     - yolov8s.onnx ($size) - Balanced speed/accuracy"
fi

if [ -f "coco.names" ]; then
    line_count=$(wc -l < "coco.names" 2>/dev/null || echo "0")
    echo "     - coco.names ($line_count classes) - Object class names"
fi

echo ""
echo "💡 Usage Instructions:"
echo "====================="
echo "1. The application will automatically use yolov8n.onnx if available"
echo "2. For better accuracy, use yolov8s.onnx (slower but more accurate)"
echo "3. The app will work with dummy detections if no model is found"
echo ""
echo "🎯 Performance Notes:"
echo "==================="
echo "- yolov8n.onnx: ~30-60 FPS on CPU, good for real-time"
echo "- yolov8s.onnx: ~15-30 FPS on CPU, better accuracy"
echo "- GPU acceleration available if OpenCV is built with CUDA"
echo ""
echo "🔧 Model Conversion:"
echo "==================="
echo "If you have .pt files, convert them to ONNX:"
echo "  pip install ultralytics"
echo "  yolo export model=yolov8n.pt format=onnx"
echo ""

cd .. 