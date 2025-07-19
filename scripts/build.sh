#!/bin/bash

# Real-time Car Vision Pipeline Build Script
# Optimized for Apple Silicon (M2+)

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
PROJECT_NAME="realtime-car-vision"
BUILD_DIR="build"
INSTALL_DIR="install"
CMAKE_BUILD_TYPE="Release"
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Print colored output
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

# Check if we're on macOS
check_platform() {
    if [[ "$OSTYPE" != "darwin"* ]]; then
        print_error "This project is designed for macOS with Apple Silicon"
        exit 1
    fi
    
    # Check for Apple Silicon
    if [[ $(uname -m) != "arm64" ]]; then
        print_warning "This project is optimized for Apple Silicon (M2+). Performance may vary on Intel Macs."
    fi
    
    print_success "Platform check passed: $(uname -s) $(uname -m)"
}

# Check dependencies
check_dependencies() {
    print_status "Checking dependencies..."
    
    local missing_deps=()
    
    # Check for Homebrew
    if ! command -v brew &> /dev/null; then
        missing_deps+=("Homebrew")
    fi
    
    # Check for Xcode Command Line Tools
    if ! xcode-select -p &> /dev/null; then
        missing_deps+=("Xcode Command Line Tools")
    fi
    
    # Check for CMake
    if ! command -v cmake &> /dev/null; then
        missing_deps+=("CMake")
    fi
    
    # Check for Python (optional - only needed for advanced model conversion)
if ! command -v python3 &> /dev/null; then
    print_warning "Python 3 not found - using C++ model converter instead"
fi
    
    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        print_status "Please run: ./scripts/setup_environment.sh"
        exit 1
    fi
    
    print_success "All dependencies found"
}

# Install missing dependencies
install_dependencies() {
    print_status "Installing dependencies..."
    
    # Install Homebrew packages
    brew_packages=(
        "cmake"
        "opencv"
        "eigen"
        "glfw"
        "boost"
        "fmt"
        "spdlog"
    )
    
    for package in "${brew_packages[@]}"; do
        if ! brew list "$package" &> /dev/null; then
            print_status "Installing $package..."
            brew install "$package"
        else
            print_status "$package already installed"
        fi
    done
    
    # Install Python packages (optional - for advanced model conversion)
if command -v python3 &> /dev/null; then
    python_packages=(
        "torch"
        "torchvision"
        "coremltools"
        "ultralytics"
        "numpy"
        "opencv-python"
    )
    
    for package in "${python_packages[@]}"; do
        if ! python3 -c "import ${package//-/_}" &> /dev/null; then
            print_status "Installing Python package: $package..."
            pip3 install "$package"
        else
            print_status "Python package $package already installed"
        fi
    done
else
    print_status "Skipping Python package installation - using C++ model converter"
fi
    
    print_success "Dependencies installed"
}

# Create build directory
setup_build() {
    print_status "Setting up build directory..."
    
    # Remove old build directory
    if [[ -d "$BUILD_DIR" ]]; then
        print_status "Removing old build directory..."
        rm -rf "$BUILD_DIR"
    fi
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    print_success "Build directory ready"
}

# Configure CMake
configure_cmake() {
    print_status "Configuring CMake..."
    
    # CMake configuration options
    cmake_options=(
        "-DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE"
        "-DCMAKE_OSX_ARCHITECTURES=arm64"
        "-DCMAKE_OSX_DEPLOYMENT_TARGET=13.0"
        "-DCMAKE_CXX_STANDARD=20"
        "-DCMAKE_CXX_STANDARD_REQUIRED=ON"
        "-DCMAKE_CXX_FLAGS_RELEASE=-O3 -DNDEBUG -march=native"
        "-DCMAKE_CXX_FLAGS_DEBUG=-g -O0 -DDEBUG"
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
        "-DBUILD_SHARED_LIBS=ON"
        "-DUSE_METAL=ON"
        "-DUSE_CORE_ML=ON"
        "-DUSE_NEURAL_ENGINE=ON"
        "-DUSE_ACCELERATE=ON"
        "-DUSE_VIDEO_TOOLBOX=ON"
        "-DUSE_AV_FOUNDATION=ON"
        "-DUSE_IMGUI=ON"
        "-DUSE_GLFW=ON"
        "-DUSE_OPENCV=ON"
        "-DUSE_EIGEN=ON"
        "-DUSE_SPDLOG=ON"
        "-DUSE_FMT=ON"
        "-DUSE_BOOST=ON"
        "-DENABLE_TESTS=ON"
        "-DENABLE_BENCHMARKS=ON"
        "-DENABLE_EXAMPLES=ON"
        "-DENABLE_DOCS=OFF"
    )
    
    # Run CMake configuration
    cmake "${cmake_options[@]}" ..
    
    if [[ $? -eq 0 ]]; then
        print_success "CMake configuration completed"
    else
        print_error "CMake configuration failed"
        exit 1
    fi
}

# Build the project
build_project() {
    print_status "Building project with $JOBS jobs..."
    
    # Build the project
    make -j"$JOBS"
    
    if [[ $? -eq 0 ]]; then
        print_success "Build completed successfully"
    else
        print_error "Build failed"
        exit 1
    fi
}

# Run tests
run_tests() {
    print_status "Running tests..."
    
    # Run unit tests
    if [[ -f "tests/unit_tests" ]]; then
        ./tests/unit_tests
    fi
    
    # Run integration tests
    if [[ -f "tests/integration_tests" ]]; then
        ./tests/integration_tests
    fi
    
    print_success "Tests completed"
}

# Install the project
install_project() {
    print_status "Installing project..."
    
    # Create install directory
    mkdir -p "../$INSTALL_DIR"
    
    # Install binaries
    if [[ -f "src/realtime_car_vision" ]]; then
        cp "src/realtime_car_vision" "../$INSTALL_DIR/"
    fi
    
    # Install libraries
    if [[ -d "lib" ]]; then
        cp -r "lib" "../$INSTALL_DIR/"
    fi
    
    # Install headers
    if [[ -d "include" ]]; then
        cp -r "include" "../$INSTALL_DIR/"
    fi
    
    # Install models
    if [[ -d "models" ]]; then
        cp -r "models" "../$INSTALL_DIR/"
    fi
    
    # Install scripts
    if [[ -d "scripts" ]]; then
        cp -r "scripts" "../$INSTALL_DIR/"
    fi
    
    # Install documentation
    if [[ -f "README.md" ]]; then
        cp "README.md" "../$INSTALL_DIR/"
    fi
    
    print_success "Installation completed"
}

# Generate Metal shaders
generate_shaders() {
    print_status "Generating Metal shaders..."
    
    # Check if Metal shader compiler is available
    if ! command -v xcrun &> /dev/null; then
        print_warning "Metal shader compiler not found, skipping shader generation"
        return
    fi
    
    # Compile Metal shaders
    if [[ -f "../shaders/preprocessing.metal" ]]; then
        xcrun metal -c ../shaders/preprocessing.metal -o preprocessing.air
        xcrun metallib preprocessing.air -o preprocessing.metallib
        print_success "Preprocessing shaders compiled"
    fi
    
    if [[ -f "../shaders/rendering.metal" ]]; then
        xcrun metal -c ../shaders/rendering.metal -o rendering.air
        xcrun metallib rendering.air -o rendering.metallib
        print_success "Rendering shaders compiled"
    fi
    
    # Clean up intermediate files
    rm -f *.air
}

# Convert YOLOv8 model
convert_model() {
    print_status "Converting YOLOv8 model to Core ML..."
    
    # Create models directory
    mkdir -p "models"
    
    # Check if C++ model converter exists
    if [[ -f "ModelConverter" ]]; then
        print_status "Using C++ model converter..."
        cd "models"
        ../ModelConverter --output yolov8n_optimized.mlmodel --size 640 --neural-engine
        
        if [[ $? -eq 0 ]]; then
            print_success "C++ model conversion completed"
        else
            print_warning "C++ model conversion failed, trying Python fallback..."
            # Fallback to Python if available
            if command -v python3 &> /dev/null && [[ -f "../../models/convert_model.py" ]]; then
                python3 ../../models/convert_model.py
                if [[ $? -eq 0 ]]; then
                    print_success "Python model conversion completed"
                else
                    print_warning "Model conversion failed, continuing without model"
                fi
            else
                print_warning "Model conversion failed, continuing without model"
            fi
        fi
        cd ..
    else
        print_warning "C++ model converter not found, trying Python fallback..."
        # Fallback to Python if available
        if command -v python3 &> /dev/null && [[ -f "../models/convert_model.py" ]]; then
            cd "models"
            python3 ../../models/convert_model.py
            if [[ $? -eq 0 ]]; then
                print_success "Python model conversion completed"
            else
                print_warning "Model conversion failed, continuing without model"
            fi
            cd ..
        else
            print_warning "No model converter available, continuing without model"
        fi
    fi
}

# Create run script
create_run_script() {
    print_status "Creating run script..."
    
    cat > "../$INSTALL_DIR/run.sh" << 'EOF'
#!/bin/bash

# Real-time Car Vision Pipeline Runner
# Usage: ./run.sh [options]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BINARY="$SCRIPT_DIR/realtime_car_vision"

if [[ ! -f "$BINARY" ]]; then
    echo "Error: Binary not found at $BINARY"
    echo "Please run the build script first"
    exit 1
fi

# Default options
CAMERA="--camera"
GUI="--gui"
FPS="--fps 50"
QUALITY="--quality high"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --video)
            CAMERA="--video $2"
            shift 2
            ;;
        --camera)
            CAMERA="--camera"
            shift
            ;;
        --no-gui)
            GUI=""
            shift
            ;;
        --fps)
            FPS="--fps $2"
            shift 2
            ;;
        --quality)
            QUALITY="--quality $2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --video <file>     Use video file as input"
            echo "  --camera           Use camera as input (default)"
            echo "  --no-gui           Run without GUI"
            echo "  --fps <value>      Target FPS (default: 50)"
            echo "  --quality <level>  Quality level: low, medium, high (default: high)"
            echo "  --help             Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Run the binary
echo "Starting Real-time Car Vision Pipeline..."
echo "Options: $CAMERA $GUI $FPS $QUALITY"
echo "Press Ctrl+C to stop"
echo ""

"$BINARY" $CAMERA $GUI $FPS $QUALITY
EOF
    
    chmod +x "../$INSTALL_DIR/run.sh"
    print_success "Run script created"
}

# Main build process
main() {
    print_status "Starting build process for $PROJECT_NAME..."
    
    # Check platform and dependencies
    check_platform
    check_dependencies
    
    # Ask user if they want to install dependencies
    read -p "Do you want to install/update dependencies? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        install_dependencies
    fi
    
    # Setup build environment
    setup_build
    
    # Generate Metal shaders
    generate_shaders
    
    # Convert YOLOv8 model
    convert_model
    
    # Configure and build
    configure_cmake
    build_project
    
    # Run tests (optional)
    read -p "Do you want to run tests? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        run_tests
    fi
    
    # Install project
    install_project
    
    # Create run script
    create_run_script
    
    # Print success message
    print_success "Build process completed successfully!"
    print_status "Installation directory: $INSTALL_DIR"
    print_status "To run the application: cd $INSTALL_DIR && ./run.sh"
    print_status "For help: ./run.sh --help"
}

# Handle command line arguments
case "${1:-}" in
    "clean")
        print_status "Cleaning build artifacts..."
        rm -rf "$BUILD_DIR" "$INSTALL_DIR"
        print_success "Clean completed"
        ;;
    "deps")
        check_platform
        install_dependencies
        ;;
    "test")
        if [[ -d "$BUILD_DIR" ]]; then
            cd "$BUILD_DIR"
            run_tests
        else
            print_error "Build directory not found. Run build first."
            exit 1
        fi
        ;;
    "install")
        if [[ -d "$BUILD_DIR" ]]; then
            cd "$BUILD_DIR"
            install_project
            create_run_script
        else
            print_error "Build directory not found. Run build first."
            exit 1
        fi
        ;;
    "help"|"-h"|"--help")
        echo "Real-time Car Vision Pipeline Build Script"
        echo ""
        echo "Usage: $0 [command]"
        echo ""
        echo "Commands:"
        echo "  (no args)  Build the entire project"
        echo "  clean      Remove build artifacts"
        echo "  deps       Install dependencies only"
        echo "  test       Run tests only"
        echo "  install    Install project only"
        echo "  help       Show this help message"
        ;;
    "")
        main
        ;;
    *)
        print_error "Unknown command: $1"
        echo "Use '$0 help' for usage information"
        exit 1
        ;;
esac 