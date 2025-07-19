# Quick Start Guide

Get up and running with real-time object detection and tracking in minutes!

## 🚀 Prerequisites

- macOS 10.15 or later
- Homebrew installed
- Apple Silicon Mac (M1/M2/M3) recommended for best performance

## ⚡ 5-Minute Setup

### 1. Install Dependencies

```bash
# Install required packages
brew install opencv qt@6 ffmpeg

# Verify installations
opencv_version
ffmpeg -version
```

### 2. Clone and Setup

```bash
# Clone the repository
git clone <repository-url>
cd "realtime car vision"

# Download YOLO models for object detection
./scripts/download_yolo_models.sh
```

### 3. Build and Run

```bash
# Build and launch the application
./scripts/build_and_run.sh
```

## 🎬 First Steps

### 1. Launch the Application
- The application opens with a file browser sidebar
- You'll see a modern, dark-themed interface

### 2. Add Your Video
- Place your video file in `data/sample_videos/` directory
- Or navigate to any folder containing video files
- Supported formats: MP4, AVI, MOV, MKV, WMV, FLV, WebM

### 3. Load and Play
- Double-click on any video file to load it
- Use playback controls to navigate through the video
- Check "Show Annotations" to enable real-time detection

### 4. Adjust Settings
- **Confidence Threshold**: 0.5 (default) - adjust for more/fewer detections
- **Show Annotations**: Toggle detection overlays on/off
- **Performance Panel**: Monitor FPS, detection time, and tracking metrics

## 🎯 What You'll See

### Detection Overlays
- **Green boxes**: Vehicles (car, truck, bus, motorcycle)
- **Blue boxes**: People
- **Red boxes**: Other objects
- **Track IDs**: Unique numbers for each tracked object
- **Confidence**: Detection confidence percentage

### Performance Metrics
- **FPS**: Current processing speed (target: 30+ FPS)
- **Detection Time**: YOLO inference time per frame
- **Tracking Time**: SORT algorithm processing time
- **Active Tracks**: Number of objects currently being tracked

## 📊 Performance Expectations

### Apple Silicon Macs (M1/M2/M3)
- **YOLOv8n**: 30-60 FPS (real-time)
- **YOLOv8s**: 15-30 FPS (better accuracy)
- **Detection Time**: 10-30ms per frame
- **Tracking Time**: <1ms per frame

### Intel Macs
- **YOLOv8n**: 15-30 FPS
- **YOLOv8s**: 8-15 FPS
- **Detection Time**: 20-50ms per frame
- **Tracking Time**: <1ms per frame

## 🎬 Recommended Test Videos

### Traffic Scenarios
- Highway traffic footage
- Intersection monitoring
- Parking lot surveillance
- Urban street scenes

### Video Specifications
- **Resolution**: 720p or 1080p (automatically resized)
- **Frame Rate**: 30 FPS (optimal)
- **Duration**: 30 seconds to 5 minutes
- **Format**: MP4 (recommended)

## 🔧 Troubleshooting

### Build Issues
```bash
# Clean build
cd qt_gui
rm -rf CMakeCache.txt CMakeFiles
cmake -DCMAKE_PREFIX_PATH="/opt/homebrew/opt/opencv;/opt/homebrew/opt/qt@6" .
make clean && make
```

### No Detections
- Ensure "Show Annotations" is checked
- Lower confidence threshold (try 0.3)
- Check video quality and lighting
- Verify YOLO models are downloaded

### Poor Performance
- Close other applications
- Use YOLOv8n model (faster)
- Try lower resolution videos
- Check system memory usage

### Video Won't Load
- Verify file format (MP4 recommended)
- Check file permissions
- Try a different video file
- Ensure file isn't corrupted

## 🎯 Next Steps

### Advanced Usage
1. **Custom Models**: Train your own YOLO models
2. **Batch Processing**: Process multiple videos
3. **Export Results**: Save detection data to CSV/JSON
4. **Performance Tuning**: Optimize for your specific use case

### Development
1. **Code Review**: Examine `detection_tracker.cpp` for algorithm details
2. **Customization**: Modify detection classes or tracking parameters
3. **Integration**: Add to your own projects
4. **Contributing**: Submit improvements and bug fixes

## 📚 Learning Resources

- [YOLOv8 Documentation](https://docs.ultralytics.com/)
- [SORT Algorithm Paper](https://arxiv.org/abs/1602.00763)
- [OpenCV DNN Module](https://docs.opencv.org/4.x/d6/d0f/group__dnn.html)
- [COCO Dataset](https://cocodataset.org/)

## 🆘 Need Help?

1. **Check the README**: Comprehensive documentation
2. **Review Error Messages**: Most issues have clear solutions
3. **Test with Sample Videos**: Ensure system is working
4. **Performance Monitoring**: Use built-in metrics to diagnose issues

---

**Ready to detect and track objects in real-time? Load your video and start detecting!** 🎬🚗 