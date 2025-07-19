# 🎬 Running Guide

## How to Run the Demo

### Option 1: Super Easy (Recommended)
```bash
./scripts/build_and_run.sh camera
```

### Option 2: Manual Steps
```bash
cd standalone_demo
cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/opencv .
make -j$(nproc)
./SimpleVideoDemo camera
```

## Input Sources

### Webcam
```bash
./SimpleVideoDemo camera
```

### Video File
```bash
./SimpleVideoDemo /path/to/your/video.mp4
```

### Sample Video (if available)
```bash
./SimpleVideoDemo data/sample_videos/test.mp4
```

## Controls

### Keyboard Controls
- **q**: Quit the program
- **s**: Save current frame as JPEG
- **Any other key**: Continue playing

### What You'll See
- Real-time video display
- FPS counter (frames per second)
- Frame number
- Processing time per frame

## Performance Tips

### For Better Performance
- Close other applications
- Use lower resolution videos
- Ensure good lighting for camera

### For Best Quality
- Use high-resolution videos
- Ensure stable camera position
- Good lighting conditions

## Troubleshooting

### Common Issues

**"Camera not found"**
- Make sure your camera isn't being used by another app
- Check camera permissions in System Preferences
- Try using a video file instead

**"Video file not found"**
- Check the file path is correct
- Make sure the video file exists
- Try using an absolute path

**"Build failed"**
- Make sure OpenCV is installed: `brew install opencv`
- Check you're in the right directory
- Try the super easy script: `./scripts/build_and_run.sh camera`

**"Permission denied"**
- Make sure the script is executable: `chmod +x scripts/build_and_run.sh`

## Example Usage

```bash
# Start with webcam
./scripts/build_and_run.sh camera

# Use a specific video file
./scripts/build_and_run.sh ~/Videos/my_video.mp4

# Use a sample video
./scripts/build_and_run.sh data/sample_videos/test.mp4
```

## What's Happening

The demo shows:
1. **Video Input**: Captures frames from camera or video file
2. **Real-time Processing**: Processes each frame as it comes
3. **Display**: Shows the processed video with performance metrics
4. **Performance Monitoring**: Displays FPS and frame timing

This is a simplified version of the full real-time video analysis pipeline, optimized for easy demonstration and testing.

Enjoy exploring real-time video processing! 🎥✨ 