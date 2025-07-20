# 🚗 Real-time Vehicle Detection & Tracking

A high-performance vehicle detection and tracking system using **YOLOv8** and **Supervision** for optimal real-time performance with minimal latency.

## ✨ Features

- **🚀 Real-time Detection**: YOLOv8-based vehicle detection with 27+ FPS
- **🎯 Multi-Vehicle Tracking**: ByteTrack algorithm for persistent vehicle tracking
- **📊 Performance Optimized**: Built for low-latency applications
- **🎨 Clean Visualization**: Professional bounding boxes and labels
- **🔧 Easy Configuration**: Simple command-line interface
- **📱 Multiple Sources**: Support for video files, webcams, and live streams
- **🖥️ Professional GUI**: Qt-based video analysis interface with file browser

## 🛠️ Technology Stack

- **YOLOv8**: Optimized for real-time performance and low latency
- **Supervision**: Modern computer vision annotation library
- **PyTorch**: Deep learning framework
- **OpenCV**: Computer vision processing
- **ByteTrack**: High-performance multi-object tracking

## 🚀 Quick Start

### 1. Setup Environment

```bash
# Create conda environment
conda create -n vehicle_detection python=3.10 -y
conda activate vehicle_detection

# Install dependencies
pip install ultralytics supervision torch torchvision opencv-python
```

### 2. Download YOLO Models

The system will automatically download YOLO models on first run, but you can also download them manually:

```bash
# Download YOLOv8 models (models will be saved to current directory)
python -c "from ultralytics import YOLO; YOLO('yolov8n.pt')"  # nano (6MB)
python -c "from ultralytics import YOLO; YOLO('yolov8s.pt')"  # small (22MB)
python -c "from ultralytics import YOLO; YOLO('yolov8m.pt')"  # medium (50MB)
python -c "from ultralytics import YOLO; YOLO('yolov8l.pt')"  # large (84MB)
python -c "from ultralytics import YOLO; YOLO('yolov8x.pt')"  # xlarge (131MB)
```

**Note**: Models are automatically downloaded to the current directory on first use.

### 3. Run Vehicle Detection

```bash
# Detect vehicles in a video file
python vehicle_detector.py --source your_video.mp4

# Test with included sample video (trimmed for quick testing)
python vehicle_detector.py --source "data/sample_videos/videoplayback testing.mp4"

# Use webcam (camera index 0)
python vehicle_detector.py --source 0

# Use different YOLOv8 model sizes
python vehicle_detector.py --source video.mp4 --model s  # small
python vehicle_detector.py --source video.mp4 --model m  # medium
python vehicle_detector.py --source video.mp4 --model l  # large

# Adjust confidence threshold
python vehicle_detector.py --source video.mp4 --conf 0.5

# Save output video
python vehicle_detector.py --source video.mp4 --save
```

## 🖥️ GUI Options

### Python GUI (Recommended - Simple)
```bash
# Run the Python GUI (uses same YOLO models as command-line)
python gui_detector.py
```

### Qt GUI (Professional Interface)

#### Quick Start
```bash
# Run the Qt GUI with one command
./run_gui.sh
```

#### Manual Build & Run
```bash
# Build Qt GUI (requires Qt development libraries)
cd qt_gui
mkdir build && cd build
cmake ..
make

# Run Qt GUI
./ProfessionalVideoAnalysis.app/Contents/MacOS/ProfessionalVideoAnalysis
```

## 📋 Command Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `--source` | Video source (file path or camera index) | `0` |
| `--model` | YOLOv8 model size (n/s/m/l/x) | `n` |
| `--conf` | Confidence threshold (0.0-1.0) | `0.3` |
| `--save` | Save output video | `False` |

## 🎯 Supported Vehicle Classes

- **Car** (class 2)
- **Truck** (class 7) 
- **Bus** (class 5)
- **Motorcycle** (class 3)
- **Bicycle** (class 1)
- **Person** (class 0)
- **Boat** (class 8)

## 📊 Performance

**Test Results (M1 Mac):**
- **Model**: YOLOv8n (nano) - optimized for speed
- **Resolution**: 1280x720
- **Average FPS**: 26.2
- **Min FPS**: 21.7
- **Max FPS**: 28.5
- **Inference Time**: ~38ms per frame

## 🎮 Controls

- **`q`**: Quit application
- **`s`**: Save screenshot

## 📁 Project Structure

```
realtime car vision/
├── vehicle_detector.py      # Main detection script (Python)
├── requirements.txt         # Python dependencies
├── README.md               # This file
├── yolov8n.pt             # YOLOv8 nano model (6MB)
├── yolov8s.pt             # YOLOv8 small model (22MB)
├── yolov8m.pt             # YOLOv8 medium model (50MB)
├── yolov8l.pt             # YOLOv8 large model (84MB)
├── yolov8x.pt             # YOLOv8 xlarge model (131MB)
├── qt_gui/                # Professional Qt GUI
│   ├── main.cpp           # Qt main application
│   ├── detection_tracker.cpp # Detection and tracking logic
│   └── CMakeLists.txt     # Qt build configuration

├── src/modules/           # Core modules
│   ├── GUIModule.cpp      # GUI module implementation
│   └── GUIModule.hpp      # GUI module header
├── data/
│   └── sample_videos/      # Sample videos for testing
│       └── videoplayback testing.mp4  # Included test video (3.2MB)
└── output_*.mp4           # Output videos (when using --save)
```

## 🔧 Model Selection Guide

The system uses YOLOv8 for optimal real-time performance:

| Model Size | Speed | Accuracy | Use Case |
|------------|-------|----------|----------|
| **n (nano)** | ⚡⚡⚡ | ⭐⭐ | Real-time, embedded |
| **s (small)** | ⚡⚡ | ⭐⭐⭐ | Balanced performance |
| **m (medium)** | ⚡ | ⭐⭐⭐⭐ | High accuracy |
| **l (large)** | 🐌 | ⭐⭐⭐⭐⭐ | Maximum accuracy |
| **x (xlarge)** | 🐌🐌 | ⭐⭐⭐⭐⭐ | Research/benchmarking |

## 🚀 Advanced Usage

### Custom Video Sources

```bash
# RTSP stream
python vehicle_detector.py --source rtsp://192.168.1.100:554/stream

# HTTP stream
python vehicle_detector.py --source http://example.com/stream.m3u8

# Multiple cameras
python vehicle_detector.py --source 0  # Camera 0
python vehicle_detector.py --source 1  # Camera 1
```

### Performance Optimization

```bash
# For maximum speed (lower accuracy)
python vehicle_detector.py --model n --conf 0.5

# For maximum accuracy (lower speed)
python vehicle_detector.py --model l --conf 0.3

# For balanced performance
python vehicle_detector.py --model s --conf 0.4
```

## 🔍 Troubleshooting

### Common Issues

1. **"No module named 'ultralytics'"**
   ```bash
   pip install ultralytics
   ```

2. **"No module named 'supervision'"**
   ```bash
   pip install supervision
   ```

3. **Low FPS**
   - Use smaller model: `--model n`
   - Increase confidence threshold: `--conf 0.5`
   - Reduce video resolution

4. **No detections**
   - Lower confidence threshold: `--conf 0.2`
   - Use larger model: `--model s` or `--model m`
   - Check if video contains vehicles

## 📈 Performance Tips

- **For real-time applications**: Use nano (n) model with confidence > 0.4
- **For accuracy**: Use small (s) or medium (m) model with confidence < 0.3
- **For embedded devices**: Stick with nano (n) model
- **For research**: Use large (l) or xlarge (x) model
- **Optimized for speed**: YOLOv8 provides the best real-time performance

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 📄 License

This project is open source and available under the MIT License.

## 🙏 Acknowledgments

- [Ultralytics](https://github.com/ultralytics/ultralytics) for YOLOv8
- [Supervision](https://github.com/roboflow/supervision) for annotation tools
- [ByteTrack](https://github.com/ifzhang/ByteTrack) for tracking algorithm

---

**🚗 Ready to detect vehicles in real-time!** 🎯 