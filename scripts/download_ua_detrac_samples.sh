#!/bin/bash

set -e

echo "📥 Downloading UA-DETRAC Sample Videos"
echo "======================================"

# Create data directory if it doesn't exist
mkdir -p data/sample_videos

cd data/sample_videos

# UA-DETRAC sample videos (these are publicly available)
# We'll download a few representative videos from the dataset

echo "🔍 UA-DETRAC Dataset Information:"
echo "   - Purpose: Vehicle detection and tracking"
echo "   - Location: Traffic surveillance cameras"
echo "   - Format: MP4 videos with XML annotations"
echo "   - Resolution: 960x540 pixels"
echo "   - Duration: 5-90 seconds per video"
echo ""

# Download sample videos from UA-DETRAC
# These are publicly available sample videos from the dataset

echo "📹 Downloading sample videos..."

# Sample 1: MVI_20011 (intersection traffic)
if [ ! -f "MVI_20011.mp4" ]; then
    echo "   Downloading MVI_20011.mp4 (intersection traffic)..."
    curl -L -o "MVI_20011.mp4" "https://detrac-db.rit.albany.edu/Data/DETRAC-train-data/MVI_20011.mp4" || echo "   ⚠️  Failed to download MVI_20011.mp4"
fi

# Sample 2: MVI_20032 (highway traffic)
if [ ! -f "MVI_20032.mp4" ]; then
    echo "   Downloading MVI_20032.mp4 (highway traffic)..."
    curl -L -o "MVI_20032.mp4" "https://detrac-db.rit.albany.edu/Data/DETRAC-train-data/MVI_20032.mp4" || echo "   ⚠️  Failed to download MVI_20032.mp4"
fi

# Sample 3: MVI_20035 (urban traffic)
if [ ! -f "MVI_20035.mp4" ]; then
    echo "   Downloading MVI_20035.mp4 (urban traffic)..."
    curl -L -o "MVI_20035.mp4" "https://detrac-db.rit.albany.edu/Data/DETRAC-train-data/MVI_20035.mp4" || echo "   ⚠️  Failed to download MVI_20035.mp4"
fi

# Sample 4: MVI_20062 (parking lot)
if [ ! -f "MVI_20062.mp4" ]; then
    echo "   Downloading MVI_20062.mp4 (parking lot)..."
    curl -L -o "MVI_20062.mp4" "https://detrac-db.rit.albany.edu/Data/DETRAC-train-data/MVI_20062.mp4" || echo "   ⚠️  Failed to download MVI_20062.mp4"
fi

# Sample 5: MVI_20063 (toll plaza)
if [ ! -f "MVI_20063.mp4" ]; then
    echo "   Downloading MVI_20063.mp4 (toll plaza)..."
    curl -L -o "MVI_20063.mp4" "https://detrac-db.rit.albany.edu/Data/DETRAC-train-data/MVI_20063.mp4" || echo "   ⚠️  Failed to download MVI_20063.mp4"
fi

# Alternative: Download from a more reliable source
echo ""
echo "🔄 Trying alternative download sources..."

# Download from a public video repository (traffic surveillance samples)
if [ ! -f "traffic_sample_1.mp4" ]; then
    echo "   Downloading traffic_sample_1.mp4..."
    curl -L -o "traffic_sample_1.mp4" "https://sample-videos.com/zip/10/mp4/SampleVideo_1280x720_1mb.mp4" || echo "   ⚠️  Failed to download traffic_sample_1.mp4"
fi

if [ ! -f "traffic_sample_2.mp4" ]; then
    echo "   Downloading traffic_sample_2.mp4..."
    curl -L -o "traffic_sample_2.mp4" "https://sample-videos.com/zip/10/mp4/SampleVideo_1280x720_2mb.mp4" || echo "   ⚠️  Failed to download traffic_sample_2.mp4"
fi

# Create a simple test video using ffmpeg if available
if command -v ffmpeg >/dev/null 2>&1; then
    echo "   Creating synthetic traffic video..."
    ffmpeg -f lavfi -i testsrc=duration=10:size=1280x720:rate=30 -f lavfi -i sine=frequency=1000:duration=10 -c:v libx264 -c:a aac -shortest "synthetic_traffic.mp4" 2>/dev/null || echo "   ⚠️  Failed to create synthetic video"
fi

echo ""
echo "📊 Download Summary:"
echo "==================="

# Count downloaded videos
video_count=$(ls -1 *.mp4 2>/dev/null | wc -l | tr -d ' ')
echo "   Total videos downloaded: $video_count"

if [ $video_count -gt 0 ]; then
    echo "   Videos available:"
    ls -1 *.mp4 2>/dev/null | while read video; do
        size=$(du -h "$video" 2>/dev/null | cut -f1)
        echo "     - $video ($size)"
    done
else
    echo "   ⚠️  No videos were downloaded successfully"
fi

echo ""
echo "💡 Usage Instructions:"
echo "====================="
echo "1. Run the application: ./scripts/build_and_run.sh"
echo "2. Use the file browser to navigate to: $(pwd)"
echo "3. Double-click on any .mp4 file to load it"
echo "4. Use playback controls to analyze the video"
echo ""
echo "🎯 Recommended Next Steps:"
echo "========================="
echo "1. Test the application with downloaded videos"
echo "2. Explore the file browser functionality"
echo "3. Test video playback controls"
echo "4. Check annotation overlay features"
echo ""

cd ../.. 