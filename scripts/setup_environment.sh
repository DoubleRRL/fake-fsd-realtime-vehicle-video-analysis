#!/bin/bash

# Real-Time Video Analysis Pipeline - Environment Setup Script
# This script installs all necessary dependencies for the project

set -e  # Exit on any error

echo "🚀 Setting up Real-Time Video Analysis Pipeline environment..."

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

# Check if running on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
    print_error "This script is designed for macOS only"
    exit 1
fi

# Check if Apple Silicon
if [[ $(uname -m) != "arm64" ]]; then
    print_warning "This project is optimized for Apple Silicon (M1/M2/M3). Performance may be suboptimal on Intel Macs."
fi

# Check macOS version
MACOS_VERSION=$(sw_vers -productVersion)
print_status "macOS version: $MACOS_VERSION"

if [[ $(echo "$MACOS_VERSION" | cut -d. -f1) -lt 12 ]]; then
    print_error "macOS 12.0 (Monterey) or later is required"
    exit 1
fi

# Check if Xcode Command Line Tools are installed
if ! xcode-select -p &> /dev/null; then
    print_status "Installing Xcode Command Line Tools..."
    xcode-select --install
    print_warning "Please complete the Xcode Command Line Tools installation and run this script again"
    exit 1
fi

print_success "Xcode Command Line Tools are installed"

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
    print_status "Installing Homebrew..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    
    # Add Homebrew to PATH for Apple Silicon
    if [[ $(uname -m) == "arm64" ]]; then
        echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zprofile
        eval "$(/opt/homebrew/bin/brew shellenv)"
    fi
fi

print_success "Homebrew is installed"

# Update Homebrew
print_status "Updating Homebrew..."
brew update

# Install C++ dependencies
print_status "Installing C++ dependencies..."

# Core libraries
brew install cmake
brew install ninja
brew install pkg-config

# Computer vision and math libraries
brew install opencv
brew install eigen
brew install glfw

# Install ImGui (if not available via Homebrew, we'll build from source)
if ! brew list imgui &> /dev/null; then
    print_status "ImGui not available via Homebrew, will build from source during CMake configuration"
fi

# Install Python dependencies for model conversion
print_status "Installing Python dependencies..."

# Check if Python 3.11 is installed
if ! brew list python@3.11 &> /dev/null; then
    brew install python@3.11
fi

# Create virtual environment
PYTHON_PATH=$(brew --prefix python@3.11)/bin/python3.11
print_status "Using Python: $PYTHON_PATH"

# Install Python packages
print_status "Installing Python packages for model conversion..."
$PYTHON_PATH -m pip install --upgrade pip
$PYTHON_PATH -m pip install coremltools
$PYTHON_PATH -m pip install torch torchvision
$PYTHON_PATH -m pip install ultralytics

# Install development tools
print_status "Installing development tools..."
brew install clang-format
brew install cppcheck
brew install valgrind

# Install additional tools
brew install ffmpeg  # For video processing
brew install wget    # For downloading datasets

# Create project directories
print_status "Creating project directories..."
mkdir -p data/{ua_detrac/{videos,annotations,processed},sample_videos}
mkdir -p models
mkdir -p resources/{fonts,icons,config}
mkdir -p tests/{unit_tests,performance_tests}
mkdir -p docs

# Create sample configuration files
print_status "Creating sample configuration files..."

# Create sample video processing configuration
cat > resources/config/video_config.json << 'EOF'
{
    "video": {
        "input": {
            "resolution": "HD540p",
            "fps": 25,
            "codec": "H.264"
        },
        "processing": {
            "enable_gpu_acceleration": true,
            "enable_neural_engine": true,
            "max_latency_ms": 20
        },
        "output": {
            "save_annotated": true,
            "save_metrics": true,
            "format": "MP4"
        }
    }
}
EOF

# Create detection configuration
cat > resources/config/detection_config.json << 'EOF'
{
    "detection": {
        "model": "yolov8n_optimized.mlmodel",
        "confidence_threshold": 0.5,
        "nms_threshold": 0.4,
        "max_detections": 100,
        "classes": [
            {"id": 0, "name": "car", "color": [255, 0, 0]},
            {"id": 1, "name": "bus", "color": [0, 255, 0]},
            {"id": 2, "name": "van", "color": [0, 0, 255]},
            {"id": 3, "name": "others", "color": [255, 255, 0]}
        ]
    }
}
EOF

# Create tracking configuration
cat > resources/config/tracking_config.json << 'EOF'
{
    "tracking": {
        "algorithm": "SORT",
        "max_age": 30,
        "min_hits": 3,
        "iou_threshold": 0.3,
        "max_tracks": 50,
        "prediction_horizon": 2.0
    }
}
EOF

# Create performance configuration
cat > resources/config/performance_config.json << 'EOF'
{
    "performance": {
        "target_fps": 50,
        "max_latency_ms": 20,
        "memory_limit_mb": 2048,
        "cpu_limit_percent": 60,
        "gpu_limit_percent": 80,
        "buffer_pool_size": 16,
        "buffer_size_mb": 64
    }
}
EOF

# Create .gitignore
cat > .gitignore << 'EOF'
# Build directories
build/
cmake-build-*/

# IDE files
.vscode/
.idea/
*.swp
*.swo

# OS files
.DS_Store
Thumbs.db

# Dependencies
third_party/
external/

# Model files (large)
models/*.mlmodel
models/*.pt
models/*.onnx

# Data files (large)
data/ua_detrac/videos/*.mp4
data/ua_detrac/annotations/*.xml
data/sample_videos/*.mp4

# Output files
output/
results/
*.mp4
*.avi

# Logs
*.log
logs/

# Temporary files
*.tmp
*.temp

# Python
__pycache__/
*.pyc
*.pyo
*.pyd
.Python
env/
venv/
.venv/

# CMake
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
Makefile

# Compiled Metal shaders
*.air
*.metallib
EOF

# Create CMake presets
cat > CMakePresets.json << 'EOF'
{
    "version": 3,
    "configurePresets": [
        {
            "name": "default",
            "displayName": "Default Config",
            "description": "Default build using Ninja generator",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release",
                "CMAKE_CXX_STANDARD": "20",
                "CMAKE_CXX_STANDARD_REQUIRED": "ON"
            }
        },
        {
            "name": "debug",
            "displayName": "Debug Config",
            "description": "Debug build with symbols",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build-debug",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "CMAKE_CXX_STANDARD": "20",
                "CMAKE_CXX_STANDARD_REQUIRED": "ON"
            }
        }
    ],
    "buildPresets": [
        {
            "name": "default",
            "configurePreset": "default"
        },
        {
            "name": "debug",
            "configurePreset": "debug"
        }
    ]
}
EOF

# Set up pre-commit hooks (if git is available)
if command -v git &> /dev/null; then
    print_status "Setting up Git hooks..."
    
    # Install pre-commit if not already installed
    if ! command -v pre-commit &> /dev/null; then
        $PYTHON_PATH -m pip install pre-commit
    fi
    
    # Create pre-commit config
    cat > .pre-commit-config.yaml << 'EOF'
repos:
  - repo: https://github.com/pre-commit/pre-commit-hooks
    rev: v4.4.0
    hooks:
      - id: trailing-whitespace
      - id: end-of-file-fixer
      - id: check-yaml
      - id: check-added-large-files
      - id: check-merge-conflict

  - repo: https://github.com/psf/black
    rev: 23.3.0
    hooks:
      - id: black
        language_version: python3.11

  - repo: https://github.com/pre-commit/mirrors-clang-format
    rev: v16.0.6
    hooks:
      - id: clang-format
        types: [c++, c]
EOF

    # Install pre-commit hooks
    pre-commit install
    print_success "Git hooks configured"
fi

# Create build script
cat > scripts/build.sh << 'EOF'
#!/bin/bash

# Build script for Real-Time Video Analysis Pipeline

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

print_status() {
    echo -e "${BLUE}[BUILD]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

# Parse arguments
BUILD_TYPE=${1:-Release}
BUILD_DIR="build"

if [[ "$BUILD_TYPE" == "debug" ]]; then
    BUILD_DIR="build-debug"
    CMAKE_BUILD_TYPE="Debug"
else
    CMAKE_BUILD_TYPE="Release"
fi

print_status "Building with type: $CMAKE_BUILD_TYPE"

# Create build directory
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure with CMake
print_status "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE

# Build
print_status "Building project..."
cmake --build . --parallel $(nproc)

print_success "Build completed successfully!"
print_status "Executable location: $BUILD_DIR/RealTimeVideoAnalysis"
EOF

chmod +x scripts/build.sh

# Create run script
cat > scripts/run.sh << 'EOF'
#!/bin/bash

# Run script for Real-Time Video Analysis Pipeline

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_status() {
    echo -e "${BLUE}[RUN]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Check if executable exists
EXECUTABLE="build/RealTimeVideoAnalysis"

if [[ ! -f "$EXECUTABLE" ]]; then
    print_warning "Executable not found. Building project first..."
    ./scripts/build.sh
fi

# Run with provided arguments
print_status "Running Real-Time Video Analysis Pipeline..."
print_status "Arguments: $@"

./$EXECUTABLE "$@"
EOF

chmod +x scripts/run.sh

# Create test script
cat > scripts/test.sh << 'EOF'
#!/bin/bash

# Test script for Real-Time Video Analysis Pipeline

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_status() {
    echo -e "${BLUE}[TEST]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Build project first
print_status "Building project for testing..."
./scripts/build.sh debug

# Run unit tests
print_status "Running unit tests..."
cd build-debug
ctest --output-on-failure

print_success "All tests passed!"
EOF

chmod +x scripts/test.sh

print_success "Environment setup completed successfully!"

echo ""
echo "🎉 Setup Summary:"
echo "  ✅ Xcode Command Line Tools"
echo "  ✅ Homebrew and dependencies"
echo "  ✅ Python 3.11 and ML packages"
echo "  ✅ Development tools"
echo "  ✅ Project structure created"
echo "  ✅ Configuration files created"
echo "  ✅ Build and run scripts created"
echo "  ✅ Git hooks configured"

echo ""
echo "📋 Next Steps:"
echo "  1. Build the project: ./scripts/build.sh"
echo "  2. Convert YOLO model: cd models && python convert_model.py"
echo "  3. Run the application: ./scripts/run.sh video.mp4"
echo "  4. Run tests: ./scripts/test.sh"

echo ""
echo "📚 Documentation:"
echo "  - README.md: Project overview and usage"
echo "  - docs/: Detailed documentation"
echo "  - scripts/: Build and utility scripts"

print_success "Real-Time Video Analysis Pipeline is ready for development!" 