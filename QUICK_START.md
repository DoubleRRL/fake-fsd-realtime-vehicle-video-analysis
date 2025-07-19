# 🚀 Quick Start Guide

Want to see real-time video processing in action? Follow these simple steps:

## What You Need
- Mac with Apple Silicon (M1/M2/M3)
- macOS 12.0 or newer
- Terminal app

## Super Quick Start (3 steps)

### 1. Install OpenCV
Open Terminal and run:
```bash
brew install opencv
```

### 2. Run the Demo
From the project folder, run:
```bash
./scripts/build_and_run.sh camera
```

### 3. Enjoy!
- A window will open showing your camera feed
- Press 'q' to quit, 's' to save a frame
- That's it! 🎉

## Alternative: Use a Video File
```bash
./scripts/build_and_run.sh /path/to/your/video.mp4
```

## What You'll See
- Real-time video processing
- FPS counter (frames per second)
- Frame number display
- Smooth video playback

## Troubleshooting
- **"Command not found"**: Make sure you're in the project folder
- **"Camera not found"**: Try using a video file instead
- **Build errors**: Make sure OpenCV is installed with `brew install opencv`

That's it! Enjoy your real-time video processing demo! 🎬✨ 