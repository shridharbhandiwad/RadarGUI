#!/bin/bash

# Radar Visualization qmake Build Script
# This script uses qmake (Qt's traditional build system)

echo "🎯 Building Radar Data Visualization with qmake..."
echo ""

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to find qmake
find_qmake() {
    # Try common qmake names
    for qmake_cmd in qmake6 qmake-qt6 qmake5 qmake-qt5 qmake; do
        if command_exists $qmake_cmd; then
            echo $qmake_cmd
            return 0
        fi
    done
    return 1
}

# Find qmake
QMAKE_CMD=$(find_qmake)

if [ $? -ne 0 ]; then
    echo "❌ Error: qmake not found!"
    echo ""
    echo "Please install Qt development packages:"
    echo ""
    echo "🐧 Ubuntu/Debian:"
    echo "  sudo apt-get install qt6-base-dev qt6-base-dev-tools"
    echo "  # or for Qt5:"
    echo "  sudo apt-get install qtbase5-dev qtbase5-dev-tools"
    echo ""
    echo "🎩 CentOS/RHEL/Fedora:"
    echo "  sudo dnf install qt6-qtbase-devel"
    echo "  # or for Qt5:"
    echo "  sudo yum install qt5-qtbase-devel"
    echo ""
    echo "🍎 macOS:"
    echo "  brew install qt"
    echo ""
    echo "🪟 Windows:"
    echo "  1. Download Qt from https://www.qt.io/download"
    echo "  2. Install Qt Creator and Qt libraries"
    echo "  3. Add Qt bin directory to PATH"
    echo "  4. Use Qt Creator or run qmake from Qt Command Prompt"
    exit 1
fi

echo "📦 Found qmake: $QMAKE_CMD"

# Check Qt version
QT_VERSION=$($QMAKE_CMD -query QT_VERSION 2>/dev/null)
if [ $? -eq 0 ]; then
    echo "🔧 Qt Version: $QT_VERSION"
else
    echo "⚠️  Warning: Could not determine Qt version"
fi

echo ""

# Clean previous build
echo "🧹 Cleaning previous build..."
rm -rf debug release *.o moc_* ui_* qrc_* Makefile*

# Generate Makefiles
echo "🔨 Generating Makefiles with qmake..."
$QMAKE_CMD RadarVisualization.pro

if [ $? -ne 0 ]; then
    echo "❌ qmake failed!"
    echo ""
    echo "🔍 Troubleshooting:"
    echo "  • Make sure all source files are present"
    echo "  • Check that Qt development packages are properly installed"
    echo "  • Verify that qmake can find Qt libraries"
    echo "  • Try running: $QMAKE_CMD -query"
    exit 1
fi

echo "✅ Makefile generated successfully"

# Build the project
echo ""
echo "⚙️  Building project..."

# Determine number of cores for parallel build
if command_exists nproc; then
    CORES=$(nproc)
elif command_exists sysctl; then
    CORES=$(sysctl -n hw.ncpu 2>/dev/null || echo 1)
else
    CORES=1
fi

echo "🚀 Building with $CORES parallel jobs..."

make -j$CORES

if [ $? -eq 0 ]; then
    echo ""
    echo "🎉 Build successful!"
    echo ""
    
    # Find the executable
    if [ -f "release/RadarVisualization" ]; then
        EXECUTABLE="release/RadarVisualization"
    elif [ -f "debug/RadarVisualization" ]; then
        EXECUTABLE="debug/RadarVisualization"
    elif [ -f "RadarVisualization" ]; then
        EXECUTABLE="RadarVisualization"
    else
        echo "⚠️  Warning: Could not find executable"
        EXECUTABLE=""
    fi
    
    if [ -n "$EXECUTABLE" ]; then
        echo "📍 Executable location: $EXECUTABLE"
        echo ""
        echo "🚀 Run the application:"
        echo "   ./$EXECUTABLE"
    fi
    
    echo ""
    echo "✨ Features included:"
    echo "  🎯 Semi-circular PPI radar display (-90° to +90°)"
    echo "  📊 Real-time FFT spectrum analysis"
    echo "  📋 Target tracking table with all parameters"
    echo "  🌐 UDP data reception on port 5000"
    echo "  🎭 Built-in data simulation for testing"
    echo "  ⚡ 50ms update rate for real-time performance"
    echo ""
    echo "💡 Tips:"
    echo "  • Use the range control to adjust PPI display (1-50 km)"
    echo "  • Toggle simulation on/off with the button"
    echo "  • Send UDP data to port 5000 for real operation"
    echo "  • All panels are resizable using splitters"
    
else
    echo ""
    echo "❌ Build failed!"
    echo ""
    echo "🔍 Common issues and solutions:"
    echo "  • Missing Qt modules: Install qt-network package"
    echo "  • Compiler errors: Check C++17 support"
    echo "  • Linker errors: Verify Qt libraries are properly installed"
    echo "  • Permission errors: Check write permissions in current directory"
    echo ""
    echo "📋 For detailed error analysis:"
    echo "  make clean && make VERBOSE=1"
    exit 1
fi