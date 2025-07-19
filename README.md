# Real-Time Video Analysis Pipeline

[![Build Status](https://github.com/DoubleRRL/fake-fsd-realtime-vehicle-video-analysis/workflows/CI/badge.svg)](https://github.com/DoubleRRL/fake-fsd-realtime-vehicle-video-analysis/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Apple Silicon](https://img.shields.io/badge/Apple%20Silicon-M1%2FM2%2FM3-blue)](https://developer.apple.com/documentation/apple-silicon)

A professional video analysis application with multiple GUI interfaces for real-time video processing, annotation display, and performance monitoring. Features file browsing, UA-DETRAC dataset support, and interactive controls.

## 🚀 Features

- **Demo Version**: Simple video display with FPS counter
- **Professional Version**: File browser + annotations + playback controls
- **File Browser**: Browse and select video files from your computer
- **UA-DETRAC Support**: Automatic loading and display of annotation files
- **Real-Time Annotations**: Display detection boxes with vehicle type labels
- **Performance Monitoring**: Real-time FPS, latency, and processing metrics
- **Interactive Controls**: Adjust confidence thresholds, toggle features
- **Playback Controls**: Play, pause, step through frames
- **No Camera Access**: Focus on video file analysis, no permissions required

## 📊 Application Versions

| Version | Description | Features |
|---------|-------------|----------|
| **Demo** | Basic video display | Video playback, FPS counter, basic controls |
| **Professional** | Full-featured app | File browser, UA-DETRAC annotations, playback controls |

## 🛠️ Quick Start

### What You Need

- **Mac with Apple Silicon** (M1, M2, or M3 chip) or Intel Mac
- **macOS 12.0 or newer** (Monterey, Ventura, or Sonoma)
- **At least 4GB of RAM**
- **1GB of free storage space**

### Step 1: Install Required Tools

First, you need to install some tools. Open Terminal (press `Cmd + Space`, type "Terminal", press Enter) and run:

```bash
# Install Homebrew (if you don't have it)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install Xcode Command Line Tools
xcode-select --install

# Install OpenCV and GLFW (this might take a few minutes)
brew install opencv glfw
```

## 🚀 Super Simple Instructions (For Total Beginners)

### Step 1: Open Terminal
- Press `Cmd + Space` on your keyboard
- Type "Terminal" and press Enter

### Step 2: Install Required Software
Copy and paste these commands one by one:

```bash
# Install Homebrew (if you don't have it)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install required software
brew install opencv glfw
```

### Step 3: Download the Project
```bash
# Go to your Downloads folder
cd ~/Downloads

# Download the project (replace with your actual download method)
# If you have the files locally, just navigate to the folder
cd "/Users/RRL_1/realtime car vision"
```

### Step 4: Run the Application

**For Demo (Simple Video Display):**
```bash
./scripts/build_and_run.sh camera
```

**For Professional Version (File Browser + Annotations):**
```bash
./scripts/build_and_run.sh pro
```

**With a Video File:**
```bash
./scripts/build_and_run.sh /path/to/your/video.mp4
```

That's it! The script will automatically build and run the application for you.

### What You'll See

**Demo Version:**
- A window will open showing the video with real-time FPS counter
- Frame number and current FPS displayed on screen
- Press 'q' to quit, 's' to save current frame

**Professional Version:**
- **File Browser**: Browse and select video files from your computer
- **UA-DETRAC Support**: Automatic loading of annotation files (.xml)
- **Real-time Annotations**: Display detection boxes with vehicle types
- **Playback Controls**: Play, pause, step through frames
- **Performance Monitoring**: FPS, latency, and processing metrics
- **Interactive Controls**: Adjust confidence thresholds, toggle features

### Controls

**Demo Version:**
- **q**: Quit the program
- **s**: Save current frame as JPEG
- **Any other key**: Continue playing

**Professional Version:**
- **SPACE**: Play/Pause video
- **LEFT/RIGHT**: Step through frames
- **A**: Toggle annotations
- **P**: Toggle performance overlay
- **F**: Toggle file browser
- **ESC**: Quit
- **Mouse**: Select files, adjust settings

### Troubleshooting

**If you get OpenCV errors during build:**
```bash
# Try this alternative cmake command:
cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/opencv .
make -j$(nproc)
```

**If you get errors about missing files:**
- Make sure you're in the `standalone_demo` folder when running commands
- Check that all the build steps completed successfully

**If the program runs slowly:**
- Close other apps to free up memory
- Try using a lower resolution video file

**If you get permission errors:**
- Make sure you have Xcode Command Line Tools installed
- Try running `sudo xcode-select --reset` if needed

**If you get "camera not found" errors:**
- Make sure your camera is not being used by another application
- Try using a video file instead: `./SimpleVideoDemo video.mp4`
- Or use the professional version: `./scripts/build_and_run.sh pro`

### Basic Usage

```bash
# Demo with camera
./scripts/build_and_run.sh camera

# Demo with video file
./scripts/build_and_run.sh /path/to/video.mp4

# Professional version (file browser)
./scripts/build_and_run.sh pro
```

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