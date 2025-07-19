#!/bin/bash

set -e

echo "📥 Downloading Better Sample Videos"
echo "==================================="

# Create data directory if it doesn't exist
mkdir -p data/sample_videos

cd data/sample_videos

echo "🔍 Downloading traffic surveillance videos..."
echo ""

# Download from reliable public video sources
echo "📹 Downloading sample videos..."

# Sample 1: Traffic intersection (from Pexels)
if [ ! -f "traffic_intersection.mp4" ]; then
    echo "   Downloading traffic_intersection.mp4..."
    curl -L -o "traffic_intersection.mp4" "https://player.vimeo.com/external/434045526.sd.mp4?s=c27eecc69a27dbc4ff2b87d38afc35f1a9e7c02d&profile_id=164&oauth2_token_id=57447761" || echo "   ⚠️  Failed to download traffic_intersection.mp4"
fi

# Sample 2: Highway traffic (from Pixabay)
if [ ! -f "highway_traffic.mp4" ]; then
    echo "   Downloading highway_traffic.mp4..."
    curl -L -o "highway_traffic.mp4" "https://cdn.pixabay.com/vimeo/3287147/traffic-23867.mp4?width=1280&hash=8c8c8c8c8c8c8c8c8c8c8c8c8c8c8c8c8c8c8c8c" || echo "   ⚠️  Failed to download highway_traffic.mp4"
fi

# Create synthetic videos using ffmpeg if available
echo ""
echo "🎬 Creating synthetic test videos..."

if command -v ffmpeg >/dev/null 2>&1; then
    # Create a synthetic traffic video with moving rectangles
    if [ ! -f "synthetic_traffic_1.mp4" ]; then
        echo "   Creating synthetic_traffic_1.mp4 (moving vehicles)..."
        ffmpeg -f lavfi -i "testsrc=duration=15:size=1280x720:rate=30" \
               -vf "drawbox=x=100:y=200:w=80:h=40:color=red:t=fill:enable='between(t,0,15)',drawbox=x=300:y=300:w=100:h=50:color=blue:t=fill:enable='between(t,2,12)',drawbox=x=500:y=150:w=90:h=45:color=green:t=fill:enable='between(t,5,15)'" \
               -c:v libx264 -preset fast -crf 23 -y "synthetic_traffic_1.mp4" 2>/dev/null || echo "   ⚠️  Failed to create synthetic_traffic_1.mp4"
    fi

    # Create a synthetic video with more complex patterns
    if [ ! -f "synthetic_traffic_2.mp4" ]; then
        echo "   Creating synthetic_traffic_2.mp4 (complex patterns)..."
        ffmpeg -f lavfi -i "testsrc=duration=20:size=1280x720:rate=30" \
               -vf "drawbox=x='100+50*sin(t)':y='200+30*cos(t)':w=60:h=30:color=red:t=fill,drawbox=x='400+40*cos(t)':y='300+20*sin(t)':w=80:h=40:color=blue:t=fill,drawbox=x='700+60*sin(t*0.5)':y='250+25*cos(t*0.5)':w=70:h=35:color=green:t=fill" \
               -c:v libx264 -preset fast -crf 23 -y "synthetic_traffic_2.mp4" 2>/dev/null || echo "   ⚠️  Failed to create synthetic_traffic_2.mp4"
    fi

    # Create a simple test pattern
    if [ ! -f "test_pattern.mp4" ]; then
        echo "   Creating test_pattern.mp4 (simple pattern)..."
        ffmpeg -f lavfi -i "testsrc=duration=10:size=1280x720:rate=30" \
               -c:v libx264 -preset fast -crf 23 -y "test_pattern.mp4" 2>/dev/null || echo "   ⚠️  Failed to create test_pattern.mp4"
    fi
else
    echo "   ⚠️  ffmpeg not found. Install with: brew install ffmpeg"
fi

# Download some sample videos from sample-videos.com
echo ""
echo "📥 Downloading sample videos from sample-videos.com..."

if [ ! -f "sample_1280x720_1mb.mp4" ]; then
    echo "   Downloading sample_1280x720_1mb.mp4..."
    curl -L -o "sample_1280x720_1mb.mp4" "https://sample-videos.com/zip/10/mp4/SampleVideo_1280x720_1mb.mp4" || echo "   ⚠️  Failed to download sample_1280x720_1mb.mp4"
fi

if [ ! -f "sample_1280x720_2mb.mp4" ]; then
    echo "   Downloading sample_1280x720_2mb.mp4..."
    curl -L -o "sample_1280x720_2mb.mp4" "https://sample-videos.com/zip/10/mp4/SampleVideo_1280x720_2mb.mp4" || echo "   ⚠️  Failed to download sample_1280x720_2mb.mp4"
fi

if [ ! -f "sample_1280x720_5mb.mp4" ]; then
    echo "   Downloading sample_1280x720_5mb.mp4..."
    curl -L -o "sample_1280x720_5mb.mp4" "https://sample-videos.com/zip/10/mp4/SampleVideo_1280x720_5mb.mp4" || echo "   ⚠️  Failed to download sample_1280x720_5mb.mp4"
fi

echo ""
echo "📊 Download Summary:"
echo "==================="

# Count downloaded videos
video_count=$(ls -1 *.mp4 2>/dev/null | wc -l | tr -d ' ')
echo "   Total videos available: $video_count"

if [ $video_count -gt 0 ]; then
    echo "   Videos available:"
    ls -1 *.mp4 2>/dev/null | while read video; do
        size=$(du -h "$video" 2>/dev/null | cut -f1)
        duration=$(ffprobe -v quiet -show_entries format=duration -of csv=p=0 "$video" 2>/dev/null | cut -d. -f1 || echo "unknown")
        echo "     - $video ($size, ${duration}s)"
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
echo "🎯 Recommended Testing Order:"
echo "============================"
echo "1. test_pattern.mp4 - Simple test pattern"
echo "2. synthetic_traffic_1.mp4 - Moving rectangles (simulated vehicles)"
echo "3. synthetic_traffic_2.mp4 - Complex moving patterns"
echo "4. sample_1280x720_1mb.mp4 - Standard sample video"
echo "5. Other downloaded videos"
echo ""
echo "🔧 If you need more videos:"
echo "=========================="
echo "- Install ffmpeg: brew install ffmpeg"
echo "- Run this script again to create more synthetic videos"
echo "- Download from: https://sample-videos.com/"
echo "- Use your own traffic surveillance videos"
echo ""

cd ../.. 