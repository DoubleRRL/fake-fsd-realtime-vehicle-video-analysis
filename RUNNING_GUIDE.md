# Running Guide: Real-time Vehicle Detection

This guide will help you get the project running quickly and generate performance benchmarks for recruiters.

## 🚀 Quick Start

### 1. Build the Project

```bash
# Clone the repository
git clone https://github.com/DoubleRRL/fake-fsd-realtime-vehicle-video-analysis.git
cd fake-fsd-realtime-vehicle-video-analysis

# Install dependencies
brew install opencv eigen glfw

# Build project
mkdir build && cd build
cmake .. && make -j$(nproc)

# Convert model (C++ converter, no Python needed)
./ModelConverter --output models/yolov8n_optimized.mlmodel --neural-engine
```

### 2. Get Sample Videos

```bash
# Option 1: Download sample videos
./scripts/download_sample_videos.sh

# Option 2: Create test video with ffmpeg
ffmpeg -f lavfi -i testsrc=duration=10:size=960x540:rate=30 \
       -c:v libx264 -preset fast -crf 23 \
       data/sample_videos/test_540p.mp4

# Option 3: Use your own video
# Place your video file in data/sample_videos/
```

### 3. Run Quick Performance Test

```bash
# Quick test (100 frames, ~3 seconds)
./Benchmark --video data/sample_videos/test_540p.mp4 --quick
```

## 📊 Benchmarking for Recruiters

### Generate Demo Video

Create an annotated video showing real-time detection performance:

```bash
# Generate 10-second demo video
./Benchmark --video data/sample_videos/test_540p.mp4 --demo --output demo.mp4

# Generate high-quality demo
./Benchmark --video data/sample_videos/test_540p.mp4 --demo --quality high --output demo_hq.mp4
```

### Run Full Benchmark

```bash
# Full benchmark (300 frames, ~10 seconds)
./Benchmark --video data/sample_videos/test_540p.mp4 --frames 300 --quality high

# This generates:
# - benchmark_report.json (detailed results)
# - performance_summary.md (formatted for README)
# - benchmark_output.mp4 (annotated video)
```

### Performance Summary

The benchmark will output a performance summary like this:

```
=== QUICK PERFORMANCE TEST ===
System: MacBook-Pro.local - macOS 14.0
Hardware: Apple M2 Pro - 16GB RAM
Average Latency: 18.45 ms
Average FPS: 54.2
Total Detections: 156
Memory Usage: 1.2 MB
CPU Usage: 45.2%

=== PERFORMANCE ASSESSMENT ===
✅ EXCELLENT: Latency < 20ms target
✅ EXCELLENT: FPS >= 50 target
```

## 🎯 Where to Get 540p Videos

### Free Sample Videos

1. **Pexels**: https://www.pexels.com/videos/
   - Search for "traffic", "cars", "highway"
   - Download in 540p or 720p

2. **Pixabay**: https://pixabay.com/videos/
   - Free traffic videos
   - Download in various resolutions

3. **Sample Videos**: https://sample-videos.com/
   - Free sample videos in different formats

### Create Test Videos

```bash
# Create synthetic test video
ffmpeg -f lavfi -i testsrc=duration=10:size=960x540:rate=30 \
       -c:v libx264 -preset fast -crf 23 \
       data/sample_videos/synthetic_540p.mp4

# Create traffic simulation
ffmpeg -f lavfi -i testsrc=duration=10:size=960x540:rate=30 \
       -f lavfi -i testsrc=duration=10:size=960x540:rate=30 \
       -filter_complex "[0:v][1:v]overlay=10:10" \
       -c:v libx264 -preset fast -crf 23 \
       data/sample_videos/traffic_sim.mp4
```

### Convert Your Videos

```bash
# Convert to 540p
ffmpeg -i your_video.mp4 -vf scale=960:540 -c:v libx264 -preset fast -crf 23 \
       data/sample_videos/converted_540p.mp4

# Convert to 30fps
ffmpeg -i your_video.mp4 -r 30 -c:v libx264 -preset fast -crf 23 \
       data/sample_videos/converted_30fps.mp4
```

## 🏃‍♂️ Running the Main Application

### Command Line Mode

```bash
# Process video file
./RealTimeVideoAnalysis --video data/sample_videos/test_540p.mp4

# Use camera input
./RealTimeVideoAnalysis --camera

# Adjust quality and performance
./RealTimeVideoAnalysis --video test_540p.mp4 --quality high --fps 60
```

### GUI Mode

```bash
# Run with GUI (recommended for demos)
./RealTimeVideoAnalysis --video test_540p.mp4 --gui

# Custom window size
./RealTimeVideoAnalysis --video test_540p.mp4 --gui --width 1920 --height 1080
```

## 📈 Performance Optimization

### Quality Settings

- **Low**: 540p, high confidence, fewer detections (best performance)
- **Medium**: 720p, balanced settings (default)
- **High**: 1080p, low confidence, more detections (best quality)

### Performance Tuning

```bash
# For maximum performance
./RealTimeVideoAnalysis --video test_540p.mp4 --quality low --fps 60

# For best quality
./RealTimeVideoAnalysis --video test_540p.mp4 --quality high --fps 30

# For balanced performance
./RealTimeVideoAnalysis --video test_540p.mp4 --quality medium --fps 50
```

## 🔧 Troubleshooting

### Common Issues

1. **Build Fails**
   ```bash
   # Clean and rebuild
   cd build
   make clean
   cmake .. && make -j$(nproc)
   ```

2. **Model Not Found**
   ```bash
   # Convert model
   ./ModelConverter --output models/yolov8n_optimized.mlmodel --neural-engine
   ```

3. **Video Not Found**
   ```bash
   # Download sample videos
   ./scripts/download_sample_videos.sh
   
   # Or create test video
   ffmpeg -f lavfi -i testsrc=duration=10:size=960x540:rate=30 \
          -c:v libx264 -preset fast -crf 23 \
          data/sample_videos/test_540p.mp4
   ```

4. **Low Performance**
   - Check thermal throttling in Activity Monitor
   - Close other applications
   - Use lower quality settings

### Performance Monitoring

```bash
# Monitor system resources
top -pid $(pgrep RealTimeVideoAnalysis)

# Check GPU usage
sudo powermetrics --samplers gpu_power -n 1

# Monitor memory usage
vm_stat 1
```

## 📊 Benchmark Results Interpretation

### Performance Targets

| Metric | Target | Excellent | Good | Needs Work |
|--------|--------|-----------|------|------------|
| **Latency** | <20ms | <15ms | <25ms | >30ms |
| **FPS** | 50+ | 60+ | 40+ | <30 |
| **Memory** | <2GB | <1GB | <3GB | >4GB |
| **CPU** | <60% | <40% | <70% | >80% |

### Sample Results

```json
{
  "test_name": "Real-time Vehicle Detection Benchmark",
  "average_latency_ms": 18.45,
  "average_fps": 54.2,
  "total_detections": 156,
  "memory_usage_mb": 1200.5,
  "cpu_usage_percent": 45.2,
  "system_info": "MacBook-Pro.local - macOS 14.0",
  "hardware_info": "Apple M2 Pro - 16GB RAM"
}
```

## 🎬 Creating Demo Videos for Recruiters

### Quick Demo

```bash
# Generate 10-second demo with performance overlay
./Benchmark --video data/sample_videos/test_540p.mp4 --demo --output recruiter_demo.mp4
```

### Professional Demo

```bash
# High-quality demo with custom settings
./Benchmark --video data/sample_videos/test_540p.mp4 \
           --demo \
           --quality high \
           --frames 300 \
           --output professional_demo.mp4
```

### Demo Video Features

- **Real-time detection boxes** around vehicles
- **Performance overlay** showing FPS and latency
- **Frame counter** for timing reference
- **Detection confidence** percentages
- **Smooth playback** at 30fps

## 📁 Project Structure

```
fake-fsd-realtime-vehicle-video-analysis/
├── src/
│   ├── main.cpp                 # Main application
│   ├── utils/
│   │   ├── ModelConverter.cpp   # C++ model converter
│   │   └── BenchmarkRunner.cpp  # Performance testing
│   └── modules/                 # Processing modules
├── data/
│   └── sample_videos/           # Test videos
├── models/                      # ML models
├── scripts/
│   ├── build.sh                 # Build script
│   └── download_sample_videos.sh # Video downloader
└── build/                       # Build output
    ├── RealTimeVideoAnalysis    # Main executable
    ├── ModelConverter           # Model converter
    └── Benchmark                # Benchmark tool
```

## 🎯 Next Steps

1. **Run quick test** to verify performance
2. **Generate demo video** for recruiters
3. **Experiment with settings** to optimize for your use case
4. **Try different videos** to test robustness
5. **Monitor performance** during extended use

The project is now ready for demonstration and performance evaluation! 