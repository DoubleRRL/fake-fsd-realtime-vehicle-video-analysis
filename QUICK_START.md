# Quick Start Guide

## Real-time Car Vision Pipeline
### Optimized for Apple Silicon (M2+)

This guide will help you get the real-time car vision pipeline up and running quickly on your Apple Silicon Mac.

## Prerequisites

- macOS 13.0 or later
- Apple Silicon Mac (M1/M2/M3) for optimal performance
- Xcode Command Line Tools
- Homebrew

## Installation

### 1. Clone the Repository
```bash
git clone <repository-url>
cd realtime-car-vision
```

### 2. Setup Environment
```bash
# Make scripts executable
chmod +x scripts/*.sh

# Setup environment and dependencies
./scripts/setup_environment.sh
```

### 3. Build the Project
```bash
# Build the entire project
./scripts/build.sh

# Or build with custom options
./scripts/build.sh clean    # Clean build
./scripts/build.sh deps     # Install dependencies only
./scripts/build.sh test     # Run tests only
```

## Usage

### Command Line Mode
```bash
# Run with camera input
./install/realtime_car_vision --camera --fps 50

# Run with video file
./install/realtime_car_vision --video data/sample_video.mp4

# Run with custom quality settings
./install/realtime_car_vision --camera --quality high --fps 60
```

### GUI Mode
```bash
# Run with GUI (recommended for interactive use)
./install/realtime_car_vision --camera --gui

# Run with custom window size
./install/realtime_car_vision --camera --gui --width 1920 --height 1080
```

### Using the Run Script
```bash
cd install
./run.sh --camera --gui
```

## Performance Targets

- **Latency**: <20ms per frame
- **Throughput**: 50+ FPS on 540p video
- **Resolution**: 960x540 (540p) optimized
- **Hardware**: Apple Silicon Neural Engine + Metal GPU

## Configuration Options

### Quality Levels
- `low`: 960x540, high confidence threshold, fewer detections
- `medium`: 1280x720, balanced settings
- `high`: 1920x1080, lower confidence threshold, more detections

### Input Sources
- `--camera`: Use built-in camera
- `--video <file>`: Use video file as input

### Performance Settings
- `--fps <value>`: Target FPS (default: 50)
- `--quality <level>`: Quality level (low/medium/high)

### GUI Options
- `--gui`: Enable GUI mode
- `--width <pixels>`: GUI window width
- `--height <pixels>`: GUI window height

## GUI Controls

### Keyboard Shortcuts
- `Space`: Start/Stop pipeline
- `P`: Toggle performance overlay
- `D`: Toggle detection overlay
- `C`: Toggle controls panel
- `F`: Toggle fullscreen
- `Escape`: Exit application

### Interactive Controls
- **Confidence Threshold**: Adjust detection sensitivity
- **NMS Threshold**: Control overlapping detection filtering
- **Max Detections**: Limit number of detections per frame
- **Zoom Level**: Zoom in/out of video display
- **Performance Graphs**: Real-time FPS and latency monitoring

## Troubleshooting

### Common Issues

1. **Build Fails**
   ```bash
   # Clean and rebuild
   ./scripts/build.sh clean
   ./scripts/build.sh
   ```

2. **Camera Not Found**
   ```bash
   # Check camera permissions
   # System Preferences > Security & Privacy > Camera
   ```

3. **Model Not Found**
   ```bash
   # Convert YOLOv8 model (C++ converter, no Python required)
   cd build
   ./ModelConverter --output models/yolov8n_optimized.mlmodel --neural-engine
   ```

4. **Performance Issues**
   - Ensure you're on Apple Silicon Mac
   - Check thermal throttling
   - Reduce quality settings
   - Close other applications

### Performance Optimization

1. **For Maximum Performance**
   ```bash
   ./install/realtime_car_vision --camera --quality low --fps 60
   ```

2. **For Best Quality**
   ```bash
   ./install/realtime_car_vision --camera --quality high --fps 30
   ```

3. **For Balanced Performance**
   ```bash
   ./install/realtime_car_vision --camera --quality medium --fps 50
   ```

## Dataset Setup

### UA-DETRAC Dataset
```bash
# Download and setup dataset
./scripts/setup_dataset.sh

# Use dataset video
./install/realtime_car_vision --video data/ua-detrac/sample_video.mp4 --gui
```

## Development

### Project Structure
```
realtime-car-vision/
├── src/
│   ├── core/           # Core pipeline components
│   ├── modules/        # Processing modules
│   └── main.cpp        # Main application
├── shaders/            # Metal compute shaders
├── models/             # ML models and conversion scripts
├── scripts/            # Build and setup scripts
├── data/               # Sample data and datasets
└── docs/               # Documentation
```

### Building for Development
```bash
# Debug build
mkdir build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# Run with debug output
./src/realtime_car_vision --camera --debug
```

### Adding New Features
1. Create module in `src/modules/`
2. Add to pipeline in `src/core/Pipeline.cpp`
3. Update CMakeLists.txt
4. Test with sample data

## Performance Monitoring

The application provides real-time performance metrics:

- **FPS**: Frames per second
- **Latency**: Processing time per frame
- **CPU Usage**: CPU utilization
- **GPU Usage**: GPU utilization
- **Memory Usage**: Memory consumption
- **Detection Count**: Objects detected per frame

## Support

For issues and questions:
1. Check the troubleshooting section
2. Review the full documentation
3. Check system requirements
4. Verify hardware compatibility

## License

This project is licensed under the MIT License - see the LICENSE file for details. 