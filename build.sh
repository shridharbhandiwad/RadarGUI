#!/bin/bash

# Radar Visualization Build Script
# Make sure Qt5 or Qt6 is installed before running this script

echo "Building Radar Data Visualization Application..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
if [ -n "$Qt6_DIR" ]; then
    echo "Using Qt6 from: $Qt6_DIR"
    cmake .. -DCMAKE_PREFIX_PATH="$Qt6_DIR"
elif [ -n "$Qt5_DIR" ]; then
    echo "Using Qt5 from: $Qt5_DIR"
    cmake .. -DCMAKE_PREFIX_PATH="$Qt5_DIR"
else
    echo "Attempting to find Qt automatically..."
    cmake ..
fi

# Build
if [ $? -eq 0 ]; then
    echo "Building application..."
    make -j$(nproc)
    
    if [ $? -eq 0 ]; then
        echo ""
        echo "✅ Build successful!"
        echo "Run the application with: ./RadarVisualization"
        echo ""
        echo "Features:"
        echo "  • Semi-circular PPI radar display (-90° to +90°)"
        echo "  • Real-time FFT spectrum analysis"
        echo "  • Target tracking table with all parameters"
        echo "  • UDP data reception on port 5000"
        echo "  • Built-in data simulation for testing"
        echo "  • 50ms update rate for real-time performance"
    else
        echo "❌ Build failed during compilation"
        exit 1
    fi
else
    echo "❌ CMake configuration failed"
    echo ""
    echo "Make sure Qt5 or Qt6 development packages are installed:"
    echo ""
    echo "Ubuntu/Debian:"
    echo "  sudo apt-get install qt6-base-dev qt6-base-dev-tools cmake build-essential"
    echo "  # or for Qt5:"
    echo "  sudo apt-get install qtbase5-dev qtbase5-dev-tools cmake build-essential"
    echo ""
    echo "CentOS/RHEL:"
    echo "  sudo yum install qt5-qtbase-devel cmake gcc-c++"
    echo ""
    echo "macOS:"
    echo "  brew install qt cmake"
    echo ""
    echo "Windows:"
    echo "  Install Qt from https://www.qt.io/download"
    echo "  Install Visual Studio or Build Tools"
    echo "  Run: cmake .. -DCMAKE_PREFIX_PATH=\"C:/Qt/6.x.x/msvcXXXX_64\""
    exit 1
fi