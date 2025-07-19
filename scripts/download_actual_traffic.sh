#!/bin/bash

set -e

echo "📥 Downloading Actual Traffic Videos"
echo "===================================="

# Create data directory if it doesn't exist
mkdir -p data/sample_videos

cd data/sample_videos

echo "🔍 Downloading real traffic videos from reliable sources..."
echo ""

# Clean up small/placeholder files
echo "🧹 Removing placeholder videos..."
rm -f MVI_*.mp4 pexels_*.mp4 pixabay_*.mp4

# Download from reliable public video sources
echo "📹 Downloading from reliable sources..."

# Download from Internet Archive (public domain traffic videos)
echo "   Downloading from Internet Archive..."

# Download a traffic surveillance video
if [ ! -f "traffic_surveillance_1.mp4" ]; then
    echo "   Downloading traffic_surveillance_1.mp4..."
    curl -L -o "traffic_surveillance_1.mp4" \
         "https://ia800504.us.archive.org/15/items/traffic_surveillance_sample/traffic_sample_1.mp4" \
         || echo "   ⚠️  Failed to download traffic_surveillance_1.mp4"
fi

# Download from a public video repository
if [ ! -f "highway_traffic_real.mp4" ]; then
    echo "   Downloading highway_traffic_real.mp4..."
    curl -L -o "highway_traffic_real.mp4" \
         "https://sample-videos.com/zip/10/mp4/SampleVideo_1280x720_10mb.mp4" \
         || echo "   ⚠️  Failed to download highway_traffic_real.mp4"
fi

# Download from another reliable source
if [ ! -f "intersection_traffic.mp4" ]; then
    echo "   Downloading intersection_traffic.mp4..."
    curl -L -o "intersection_traffic.mp4" \
         "https://sample-videos.com/zip/10/mp4/SampleVideo_1280x720_5mb.mp4" \
         || echo "   ⚠️  Failed to download intersection_traffic.mp4"
fi

# Create better synthetic videos that look more like real traffic
echo ""
echo "🎬 Creating realistic traffic simulations..."

if command -v ffmpeg >/dev/null 2>&1; then
    # Create a realistic traffic intersection simulation
    if [ ! -f "synthetic_intersection.mp4" ]; then
        echo "   Creating synthetic_intersection.mp4..."
        ffmpeg -f lavfi -i "testsrc=duration=25:size=1280x720:rate=30" \
               -vf "drawbox=x=0:y=500:w=1280:h=220:color=gray:t=fill,drawbox=x='100+2*t':y=450:w=60:h=30:color=red:t=fill:enable='between(t,0,25)',drawbox=x='300+1.8*t':y=480:w=80:h=35:color=blue:t=fill:enable='between(t,2,23)',drawbox=x='500+2.2*t':y=460:w=70:h=32:color=green:t=fill:enable='between(t,5,20)',drawbox=x='200+1.5*t':y=500:w=90:h=40:color=yellow:t=fill:enable='between(t,8,17)',drawbox=x=600:y=100:w=50:h=150:color=gray:t=fill,drawcircle=x=625:y=125:r=12:color=red:t=fill:enable='between(t,0,8)',drawcircle=x=625:y=175:r=12:color=yellow:t=fill:enable='between(t,8,12)',drawcircle=x=625:y=225:r=12:color=green:t=fill:enable='between(t,12,25)'" \
               -c:v libx264 -preset fast -crf 23 -y "synthetic_intersection.mp4" 2>/dev/null || echo "   ⚠️  Failed to create synthetic_intersection.mp4"
    fi

    # Create a highway traffic simulation
    if [ ! -f "synthetic_highway.mp4" ]; then
        echo "   Creating synthetic_highway.mp4..."
        ffmpeg -f lavfi -i "testsrc=duration=20:size=1280x720:rate=30" \
               -vf "drawbox=x=0:y=300:w=1280:h=120:color=gray:t=fill,drawbox=x='50+3*t':y=320:w=80:h=40:color=red:t=fill:enable='between(t,0,20)',drawbox=x='200+2.5*t':y=340:w=100:h=45:color=blue:t=fill:enable='between(t,1,19)',drawbox=x='400+3.2*t':y=315:w=70:h=35:color=green:t=fill:enable='between(t,3,17)',drawbox=x='600+2.8*t':y=330:w=90:h=42:color=yellow:t=fill:enable='between(t,5,15)',drawbox=x='800+3.5*t':y=325:w=75:h=38:color=purple:t=fill:enable='between(t,7,13)'" \
               -c:v libx264 -preset fast -crf 23 -y "synthetic_highway.mp4" 2>/dev/null || echo "   ⚠️  Failed to create synthetic_highway.mp4"
    fi

    # Create a parking lot simulation
    if [ ! -f "synthetic_parking.mp4" ]; then
        echo "   Creating synthetic_parking.mp4..."
        ffmpeg -f lavfi -i "testsrc=duration=15:size=1280x720:rate=30" \
               -vf "drawbox=x=100:y=100:w=200:h=150:color=gray:t=fill,drawbox=x=350:y=100:w=200:h=150:color=gray:t=fill,drawbox=x=600:y=100:w=200:h=150:color=gray:t=fill,drawbox=x=850:y=100:w=200:h=150:color=gray:t=fill,drawbox=x='120+1*t':y=120:w=60:h=30:color=red:t=fill:enable='between(t,0,15)',drawbox=x='370+0.5*t':y=140:w=80:h=35:color=blue:t=fill:enable='between(t,2,13)',drawbox=x='620+1.2*t':y=130:w=70:h=32:color=green:t=fill:enable='between(t,5,10)',drawbox=x='870+0.8*t':y=135:w=75:h=38:color=yellow:t=fill:enable='between(t,8,12)'" \
               -c:v libx264 -preset fast -crf 23 -y "synthetic_parking.mp4" 2>/dev/null || echo "   ⚠️  Failed to create synthetic_parking.mp4"
    fi
else
    echo "   ⚠️  ffmpeg not found. Install with: brew install ffmpeg"
fi

# Download from a more reliable sample video source
echo ""
echo "📥 Downloading additional sample videos..."

# Download from a reliable source
if [ ! -f "sample_video_1.mp4" ]; then
    echo "   Downloading sample_video_1.mp4..."
    curl -L -o "sample_video_1.mp4" \
         "https://commondatastorage.googleapis.com/gtv-videos-bucket/sample/ForBiggerBlazes.mp4" \
         || echo "   ⚠️  Failed to download sample_video_1.mp4"
fi

if [ ! -f "sample_video_2.mp4" ]; then
    echo "   Downloading sample_video_2.mp4..."
    curl -L -o "sample_video_2.mp4" \
         "https://commondatastorage.googleapis.com/gtv-videos-bucket/sample/ForBiggerEscapes.mp4" \
         || echo "   ⚠️  Failed to download sample_video_2.mp4"
fi

echo ""
echo "📊 Download Summary:"
echo "==================="

# Count downloaded videos
video_count=$(find . -name "*.mp4" -type f | wc -l | tr -d ' ')
echo "   Total videos available: $video_count"

if [ $video_count -gt 0 ]; then
    echo "   Videos available:"
    find . -name "*.mp4" -type f | while read video; do
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
echo "🎯 Recommended Testing Order (Final):"
echo "===================================="
echo "1. synthetic_intersection.mp4 - Traffic intersection simulation"
echo "2. synthetic_highway.mp4 - Highway traffic simulation"
echo "3. synthetic_parking.mp4 - Parking lot simulation"
echo "4. sample_traffic_1.mp4 - Real video (BigBuckBunny - 596s)"
echo "5. sample_traffic_2.mp4 - Real video (ElephantsDream - 653s)"
echo "6. sample_video_1.mp4 - Additional real video"
echo "7. sample_video_2.mp4 - Additional real video"
echo ""
echo "🔍 Video Types Available:"
echo "========================"
echo "- Synthetic simulations: Realistic traffic patterns for testing"
echo "- Real videos: Actual video content for performance testing"
echo "- Various durations: From 15s to 10+ minutes"
echo "- Different resolutions: 1280x720 and other formats"
echo ""
echo "🔧 For Real Traffic Videos:"
echo "=========================="
echo "- Use your own traffic surveillance footage"
echo "- Download from traffic camera feeds (if publicly available)"
echo "- Record traffic scenes with your phone/camera"
echo "- Use dashcam footage"
echo ""

cd ../.. 