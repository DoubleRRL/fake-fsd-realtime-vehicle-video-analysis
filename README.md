# Real-Time Video Analysis App

## 🚀 Features

- File Browser: Browse and select video files from your computer
- UA-DETRAC Support: Automatic loading and display of annotation files
- Real-Time Annotations: Display detection boxes with vehicle type labels
- Performance Monitoring: Real-time FPS, latency, and processing metrics
- Interactive Controls: Adjust confidence thresholds, toggle features
- Playback Controls: Play, pause, step through frames
- No Camera Access: File browser only, no permissions required

## 🛠️ Quick Start

### 1. Install Dependencies

```bash
# Install Homebrew if you don't have it
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install OpenCV and GLFW
brew install opencv glfw
```

### 2. Run the App

```bash
cd /path/to/your/project/root
./scripts/build_and_run.sh
```

- The professional GUI will open. Use the file browser to select a video file.
- Or run with a specific video file:

```bash
./scripts/build_and_run.sh /path/to/video.mp4
```

## Controls
- SPACE: Play/Pause
- LEFT/RIGHT: Step through frames
- A: Toggle annotations
- P: Toggle performance overlay
- F: Toggle file browser
- ESC: Quit
- Mouse: Select files, adjust settings

## 📋 Requirements

### System Requirements
- **OS**: macOS 12.0+ (Monterey or later)
- **Processor**: Apple Silicon (M1/M2/M3) or Intel Mac
- **Memory**: 4GB+ RAM
- **Storage**: 1GB+ free space
- **Development**: Xcode 14.0+ with Command Line Tools

### Dependencies
- **OpenCV**: Computer vision library
- **GLFW**: Window management and OpenGL context
- **OpenGL**: Graphics rendering
- **C++17**: Modern C++ features

**No Python Dependencies**: Pure C++ implementation for simplicity and performance.

## 🎯 Use Cases

- **Video Analysis**: Professional video processing and annotation display
- **Dataset Review**: Browse and analyze UA-DETRAC and other annotated datasets
- **Research**: Visualize detection results and performance metrics
- **Education**: Learn about computer vision and video processing
- **Development**: Test and debug video processing pipelines

## 🏗️ Architecture

### Application Structure

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   File Browser  │───▶│  Video Loading   │───▶│   Annotation    │
│   (GLFW/OpenGL) │    │   (OpenCV)       │    │   (XML Parser)  │
└─────────────────┘    └──────────────────┘    └─────────────────┘
                                                         │
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   GUI Display   │◀───│   Rendering      │◀───│   Performance   │
│   (OpenGL)      │    │   (OpenGL)       │    │   Monitoring    │
└─────────────────┘    └──────────────────┘    └─────────────────┘
                                                         │
                       ┌──────────────────┐    ┌─────────────────┐
                       │   Playback       │◀───│   Controls      │
                       │   Controls       │    │   (Interactive) │
                       └──────────────────┘    └─────────────────┘
```

### Key Components

1. **File Browser**: Browse and select video files
2. **Video Loader**: OpenCV-based video processing
3. **Annotation Parser**: UA-DETRAC XML format support
4. **Rendering Engine**: OpenGL-based visualization
5. **Performance Monitor**: Real-time metrics tracking
6. **Interactive Controls**: User interface elements

## 🔧 Configuration

### Application Settings

The application supports various configuration options:

- **Confidence Threshold**: Adjust detection sensitivity (0.0 - 1.0)
- **Annotation Display**: Toggle bounding boxes and labels
- **Performance Overlay**: Show/hide FPS and latency metrics
- **File Browser**: Enable/disable file selection panel
- **Playback Speed**: Control video playback rate

### Supported Formats

- **Video**: MP4, AVI, MOV, MKV
- **Annotations**: UA-DETRAC XML format
- **Output**: JPEG frame capture

## 📈 Performance Features

### Real-Time Processing

- **OpenGL Rendering**: Hardware-accelerated graphics display
- **OpenCV Optimization**: Efficient video frame processing
- **Memory Management**: Optimized texture and buffer handling
- **Multi-threading**: Background processing for smooth UI

### Performance Monitoring

- **FPS Tracking**: Real-time frame rate monitoring
- **Latency Measurement**: Processing time per frame
- **Memory Usage**: Current and peak memory consumption
- **CPU Utilization**: System resource monitoring

## 🎮 GUI Features

### Real-Time Visualization

- **Video Display**: High-quality video playback with annotations
- **Bounding Boxes**: Color-coded detection boxes by vehicle type
- **Track IDs**: Unique identifiers for tracked objects
- **Performance Metrics**: Real-time FPS and latency display
- **File Browser**: Easy video file selection

### Interactive Controls

- **Playback Controls**: Play, pause, step through frames
- **Annotation Toggles**: Show/hide detection boxes and labels
- **Performance Panels**: Real-time monitoring widgets
- **Confidence Sliders**: Adjust detection sensitivity
- **File Selection**: Browse and load video files

## 🧪 Testing

### Manual Testing

```bash
# Test simple demo
./scripts/build_and_run.sh camera

# Test GUI version
./scripts/build_and_run.sh camera gui

# Test professional version
./scripts/build_and_run.sh pro
```

### Video File Testing

```bash
# Test with sample video
./scripts/build_and_run.sh /path/to/sample.mp4

# Test with UA-DETRAC annotations
./scripts/build_and_run.sh pro
```

## 📊 Dataset Support

### UA-DETRAC Dataset

The application supports the UA-DETRAC traffic surveillance dataset:

- **Video Format**: MP4, AVI, MOV, MKV
- **Annotation Format**: XML files with frame-by-frame detections
- **Vehicle Types**: Car, bus, van, truck, others
- **Auto-loading**: Automatically finds matching annotation files

### Sample Data

A sample annotation file is included in `data/sample_annotations/sample_video.xml` to demonstrate the format.

## 🔍 Troubleshooting

### Common Issues

1. **Build Errors**
   - Make sure OpenCV and GLFW are installed: `brew install opencv glfw`
   - Check Xcode Command Line Tools are installed: `xcode-select --install`
   - Verify you're in the correct directory

2. **Video Not Loading**
   - Check file path is correct
   - Verify video format is supported (MP4, AVI, MOV, MKV)
   - Ensure video file exists and is not corrupted

3. **Annotations Not Showing**
   - Check if XML annotation file exists with same name as video
   - Verify XML format matches UA-DETRAC specification
   - Try adjusting confidence threshold

### Performance Tips

- Close other applications to free up memory
- Use lower resolution videos for better performance
- Adjust confidence threshold to reduce processing load

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

### Development Setup

```bash
# Clone the repository
git clone https://github.com/yourusername/realtime-car-vision.git

# Install dependencies
brew install opencv glfw cmake

# Build the project
./scripts/build_and_run.sh pro
```

### Code Style

- Follow the existing C++ style guide
- Use clang-format for code formatting
- Test all three versions (simple, GUI, professional)
- Update documentation for new features

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **UA-DETRAC Dataset**: Traffic surveillance dataset
- **OpenCV**: Computer vision library
- **GLFW**: Window management and OpenGL context
- **Apple**: macOS development platform

## 📚 Documentation

- [Quick Start Guide](QUICK_START.md)
- [Running Guide](RUNNING_GUIDE.md)
- [Project Summary](PROJECT_SUMMARY.md)

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/realtime-car-vision/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/realtime-car-vision/discussions)

---

**Made with ❤️ for Video Analysis** 