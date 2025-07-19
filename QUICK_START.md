# Quick Start

## 1. Install Dependencies

```bash
# Install Homebrew if you don't have it
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install OpenCV and GLFW
brew install opencv glfw
```

## 2. Run the App

```bash
cd /path/to/your/project/root
./scripts/build_and_run.sh
```

- The professional GUI will open. Use the file browser to select a video file.
- Or run with a specific video file:

```bash
./scripts/build_and_run.sh /path/to/video.mp4
```

That's it! Enjoy your real-time video analysis app! 🎬✨ 