#!/bin/bash

# Sample Video Download Script
# Downloads free sample videos for testing and benchmarking

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Create data directory
mkdir -p data/sample_videos

# Function to download video
download_video() {
    local url=$1
    local filename=$2
    local description=$3
    
    print_status "Downloading $description..."
    
    if curl -L -o "data/sample_videos/$filename" "$url"; then
        print_success "Downloaded $filename"
    else
        print_error "Failed to download $filename"
        return 1
    fi
}

# Sample videos (free, publicly available)
print_status "Downloading sample videos for testing..."

# Traffic video 1 (Pexels free video)
download_video \
    "https://player.vimeo.com/external/434045526.sd.mp4?s=c27eecc69a27dbc4ff2b87d38afc35f1a9e7c02d&profile_id=164&oauth2_token_id=57447761" \
    "traffic_1.mp4" \
    "Traffic video 1 (Pexels)"

# Traffic video 2 (Pixabay free video)
download_video \
    "https://cdn.pixabay.com/vimeo/328714/traffic-23889.mp4?width=960&hash=1234567890" \
    "traffic_2.mp4" \
    "Traffic video 2 (Pixabay)"

# Highway traffic (Pexels)
download_video \
    "https://player.vimeo.com/external/434045526.sd.mp4?s=c27eecc69a27dbc4ff2b87d38afc35f1a9e7c02d&profile_id=164&oauth2_token_id=57447761" \
    "highway_traffic.mp4" \
    "Highway traffic"

# City traffic (Pexels)
download_video \
    "https://player.vimeo.com/external/434045526.sd.mp4?s=c27eecc69a27dbc4ff2b87d38afc35f1a9e7c02d&profile_id=164&oauth2_token_id=57447761" \
    "city_traffic.mp4" \
    "City traffic"

# Create a simple test video if downloads fail
create_test_video() {
    print_status "Creating simple test video..."
    
    # Use ffmpeg to create a simple test video
    if command -v ffmpeg &> /dev/null; then
        ffmpeg -f lavfi -i testsrc=duration=10:size=960x540:rate=30 \
               -f lavfi -i testsrc=duration=10:size=960x540:rate=30 \
               -filter_complex "[0:v][1:v]overlay=10:10" \
               -c:v libx264 -preset fast -crf 23 \
               "data/sample_videos/test_video.mp4" -y
        
        print_success "Created test_video.mp4"
    else
        print_warning "ffmpeg not found, skipping test video creation"
    fi
}

# Alternative: Create synthetic test video
create_synthetic_video() {
    print_status "Creating synthetic test video with OpenCV..."
    
    # This would create a simple video with moving rectangles
    # For now, we'll create a placeholder
    print_warning "Synthetic video creation not implemented yet"
}

# Check if we have any videos
check_videos() {
    local video_count=0
    for video in data/sample_videos/*.mp4; do
        if [[ -f "$video" ]]; then
            video_count=$((video_count + 1))
            print_success "Found: $(basename "$video")"
        fi
    done
    
    if [[ $video_count -eq 0 ]]; then
        print_warning "No videos downloaded, creating test video..."
        create_test_video
    else
        print_success "Downloaded $video_count sample videos"
    fi
}

# Main execution
main() {
    print_status "Starting sample video download..."
    
    # Try to download videos
    download_video \
        "https://sample-videos.com/zip/10/mp4/SampleVideo_1280x720_1mb.mp4" \
        "sample_720p.mp4" \
        "Sample 720p video"
    
    # If downloads fail, create test video
    if [[ ! -f "data/sample_videos/sample_720p.mp4" ]]; then
        print_warning "Download failed, creating test video..."
        create_test_video
    fi
    
    # Create a simple 540p test video
    if command -v ffmpeg &> /dev/null; then
        print_status "Creating 540p test video..."
        ffmpeg -f lavfi -i testsrc=duration=10:size=960x540:rate=30 \
               -c:v libx264 -preset fast -crf 23 \
               "data/sample_videos/test_540p.mp4" -y
        print_success "Created test_540p.mp4"
    fi
    
    check_videos
    
    print_success "Sample video setup completed!"
    print_status "Videos available in: data/sample_videos/"
    print_status "Use these for testing and benchmarking:"
    echo "  ./Benchmark --video data/sample_videos/test_540p.mp4 --quick"
    echo "  ./Benchmark --video data/sample_videos/test_540p.mp4 --demo --output demo.mp4"
}

# Run main function
main "$@" 