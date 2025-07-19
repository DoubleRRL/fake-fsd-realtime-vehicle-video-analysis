#!/bin/bash

# Real-Time Video Analysis Pipeline - Dataset Setup Script
# This script downloads and prepares the UA-DETRAC dataset

set -e  # Exit on any error

echo "📊 Setting up UA-DETRAC dataset for Real-Time Video Analysis Pipeline..."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
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

# Check if required tools are available
check_dependencies() {
    print_status "Checking dependencies..."
    
    if ! command -v wget &> /dev/null; then
        print_error "wget is required but not installed. Please install it first."
        exit 1
    fi
    
    if ! command -v ffmpeg &> /dev/null; then
        print_error "ffmpeg is required but not installed. Please install it first."
        exit 1
    fi
    
    print_success "All dependencies are available"
}

# Create data directories
create_directories() {
    print_status "Creating data directories..."
    
    mkdir -p data/ua_detrac/{videos,annotations,processed}
    mkdir -p data/sample_videos
    
    print_success "Data directories created"
}

# Download sample videos from UA-DETRAC
download_sample_videos() {
    print_status "Downloading sample videos from UA-DETRAC..."
    
    cd data/ua_detrac/videos
    
    # Sample videos from UA-DETRAC (small subset for testing)
    VIDEOS=(
        "MVI_20011.mp4"
        "MVI_20012.mp4"
        "MVI_20032.mp4"
        "MVI_20033.mp4"
    )
    
    for video in "${VIDEOS[@]}"; do
        if [[ ! -f "$video" ]]; then
            print_status "Downloading $video..."
            wget -q --show-progress "https://detrac-db.rit.albany.edu/Data/DETRAC-train-data/Insight-MVT_Annotation_Train/$video" || {
                print_warning "Failed to download $video, skipping..."
                continue
            }
        else
            print_status "$video already exists, skipping..."
        fi
    done
    
    cd ../../..
    print_success "Sample videos downloaded"
}

# Download annotations
download_annotations() {
    print_status "Downloading annotations..."
    
    cd data/ua_detrac/annotations
    
    # Sample annotations
    ANNOTATIONS=(
        "MVI_20011_v3.xml"
        "MVI_20012_v3.xml"
        "MVI_20032_v3.xml"
        "MVI_20033_v3.xml"
    )
    
    for annotation in "${ANNOTATIONS[@]}"; do
        if [[ ! -f "$annotation" ]]; then
            print_status "Downloading $annotation..."
            wget -q --show-progress "https://detrac-db.rit.albany.edu/Data/DETRAC-train-data/DETRAC-Train-Annotations-XML/$annotation" || {
                print_warning "Failed to download $annotation, skipping..."
                continue
            }
        else
            print_status "$annotation already exists, skipping..."
        fi
    done
    
    cd ../../..
    print_success "Annotations downloaded"
}

# Convert videos to 540p
convert_videos() {
    print_status "Converting videos to 540p..."
    
    cd data/ua_detrac/videos
    
    for video in *.mp4; do
        if [[ -f "$video" ]]; then
            filename=$(basename "$video" .mp4)
            output_path="../processed/${filename}_540p.mp4"
            
            if [[ ! -f "$output_path" ]]; then
                print_status "Converting $video to 540p..."
                ffmpeg -i "$video" \
                       -vf scale=960:540 \
                       -c:v libx264 \
                       -crf 23 \
                       -preset medium \
                       -c:a copy \
                       "$output_path" \
                       -y -loglevel error
            else
                print_status "$output_path already exists, skipping..."
            fi
        fi
    done
    
    cd ../../..
    print_success "Videos converted to 540p"
}

# Create dataset configuration
create_dataset_config() {
    print_status "Creating dataset configuration..."
    
    cat > data/ua_detrac/dataset_config.json << 'EOF'
{
    "dataset": {
        "name": "UA-DETRAC",
        "description": "Traffic surveillance dataset optimized for 540p processing",
        "version": "1.0",
        "resolution": {
            "width": 960,
            "height": 540,
            "fps": 25
        },
        "classes": [
            {"id": 0, "name": "car", "color": [255, 0, 0]},
            {"id": 1, "name": "bus", "color": [0, 255, 0]},
            {"id": 2, "name": "van", "color": [0, 0, 255]},
            {"id": 3, "name": "others", "color": [255, 255, 0]}
        ],
        "detection_config": {
            "confidence_threshold": 0.5,
            "nms_threshold": 0.4,
            "max_detections": 100
        },
        "tracking_config": {
            "max_age": 30,
            "min_hits": 3,
            "iou_threshold": 0.3
        },
        "videos": [
            {
                "name": "MVI_20011",
                "filename": "MVI_20011_540p.mp4",
                "annotation": "MVI_20011_v3.xml",
                "duration": 0,
                "frame_count": 0
            },
            {
                "name": "MVI_20012",
                "filename": "MVI_20012_540p.mp4",
                "annotation": "MVI_20012_v3.xml",
                "duration": 0,
                "frame_count": 0
            },
            {
                "name": "MVI_20032",
                "filename": "MVI_20032_540p.mp4",
                "annotation": "MVI_20032_v3.xml",
                "duration": 0,
                "frame_count": 0
            },
            {
                "name": "MVI_20033",
                "filename": "MVI_20033_540p.mp4",
                "annotation": "MVI_20033_v3.xml",
                "duration": 0,
                "frame_count": 0
            }
        ]
    }
}
EOF

    print_success "Dataset configuration created"
}

# Create sample video for testing
create_sample_video() {
    print_status "Creating sample test video..."
    
    cd data/sample_videos
    
    # Create a simple test video using ffmpeg
    if [[ ! -f "test_540p.mp4" ]]; then
        print_status "Generating test video..."
        ffmpeg -f lavfi -i testsrc=duration=10:size=960x540:rate=25 \
               -f lavfi -i sine=frequency=1000:duration=10 \
               -c:v libx264 -c:a aac \
               test_540p.mp4 \
               -y -loglevel error
    else
        print_status "Test video already exists"
    fi
    
    cd ../..
    print_success "Sample test video created"
}

# Validate dataset
validate_dataset() {
    print_status "Validating dataset..."
    
    cd data/ua_detrac
    
    # Check if videos exist
    video_count=$(ls processed/*_540p.mp4 2>/dev/null | wc -l)
    annotation_count=$(ls annotations/*.xml 2>/dev/null | wc -l)
    
    print_status "Found $video_count processed videos"
    print_status "Found $annotation_count annotation files"
    
    if [[ $video_count -eq 0 ]]; then
        print_warning "No processed videos found"
    fi
    
    if [[ $annotation_count -eq 0 ]]; then
        print_warning "No annotation files found"
    fi
    
    cd ../..
    
    print_success "Dataset validation completed"
}

# Create dataset info
create_dataset_info() {
    print_status "Creating dataset information..."
    
    cat > data/ua_detrac/README.md << 'EOF'
# UA-DETRAC Dataset

This directory contains a subset of the UA-DETRAC (UA-DETRAC Benchmark Suite) dataset, optimized for real-time video analysis.

## Dataset Information

- **Source**: UA-DETRAC Benchmark Suite
- **Resolution**: 540p (960x540)
- **Frame Rate**: 25 FPS
- **Format**: MP4 (H.264)
- **Classes**: Car, Bus, Van, Others

## Files

### Videos
- `MVI_20011_540p.mp4`: Traffic surveillance video 1
- `MVI_20012_540p.mp4`: Traffic surveillance video 2
- `MVI_20032_540p.mp4`: Traffic surveillance video 3
- `MVI_20033_540p.mp4`: Traffic surveillance video 4

### Annotations
- `MVI_20011_v3.xml`: Annotations for video 1
- `MVI_20012_v3.xml`: Annotations for video 2
- `MVI_20032_v3.xml`: Annotations for video 3
- `MVI_20033_v3.xml`: Annotations for video 4

## Usage

```bash
# Run analysis on a specific video
./RealTimeVideoAnalysis processed/MVI_20011_540p.mp4

# Run with custom configuration
./RealTimeVideoAnalysis --config dataset_config.json processed/MVI_20011_540p.mp4
```

## Citation

If you use this dataset, please cite:

```
@inproceedings{detrac2017,
  title={UA-DETRAC: A new benchmark and protocol for multi-object detection and tracking},
  author={Wen, Longyin and Du, Dawei and Cai, Zhen and Lei, Zhen and Chang, Ming-Ching and Qi, Honggang and Lim, Jongwoo and Yang, Ming-Hsuan and Lyu, Siwei},
  booktitle={Computer Vision and Image Understanding},
  year={2017}
}
```
EOF

    print_success "Dataset information created"
}

# Main execution
main() {
    print_status "Starting dataset setup..."
    
    check_dependencies
    create_directories
    download_sample_videos
    download_annotations
    convert_videos
    create_dataset_config
    create_sample_video
    validate_dataset
    create_dataset_info
    
    print_success "Dataset setup completed successfully!"
    
    echo ""
    echo "📊 Dataset Summary:"
    echo "  ✅ Sample videos downloaded"
    echo "  ✅ Annotations downloaded"
    echo "  ✅ Videos converted to 540p"
    echo "  ✅ Configuration files created"
    echo "  ✅ Test video generated"
    
    echo ""
    echo "📋 Available videos:"
    ls -la data/ua_detrac/processed/*_540p.mp4 2>/dev/null || echo "  No processed videos found"
    
    echo ""
    echo "🚀 Next Steps:"
    echo "  1. Build the project: ./scripts/build.sh"
    echo "  2. Run analysis: ./RealTimeVideoAnalysis data/ua_detrac/processed/MVI_20011_540p.mp4"
    echo "  3. Test with sample: ./RealTimeVideoAnalysis data/sample_videos/test_540p.mp4"
    
    echo ""
    echo "📚 Dataset Information:"
    echo "  - Location: data/ua_detrac/"
    echo "  - Configuration: data/ua_detrac/dataset_config.json"
    echo "  - Documentation: data/ua_detrac/README.md"
}

# Run main function
main "$@" 