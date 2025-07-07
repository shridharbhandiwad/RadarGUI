# 🔧 Compilation, Building, and Execution Guide

## 📋 Quick Start

### 🚀 Fastest Way to Build and Run

**Linux/macOS:**
```bash
./build_qmake.sh && ./release/RadarVisualization
```

**Windows:**
```cmd
build_qmake.bat
```

---

## 🏗️ Build System Options

The project supports **two build systems**:

1. **qmake** (Recommended) - Qt's traditional build system
2. **CMake** - Modern cross-platform build system

## 📦 Prerequisites

### 🐧 Linux (Ubuntu/Debian)
```bash
# Qt6 (Recommended)
sudo apt-get update
sudo apt-get install qt6-base-dev qt6-base-dev-tools build-essential

# OR Qt5 (Alternative)
sudo apt-get install qtbase5-dev qtbase5-dev-tools build-essential
```

### 🎩 Linux (CentOS/RHEL/Fedora)
```bash
# Qt6
sudo dnf install qt6-qtbase-devel gcc-c++

# OR Qt5
sudo yum install qt5-qtbase-devel gcc-c++
```

### 🍎 macOS
```bash
# Using Homebrew
brew install qt

# Using MacPorts
sudo port install qt6-qtbase +universal
```

### 🪟 Windows
1. **Download Qt**: https://www.qt.io/download
2. **Install Qt Creator** with MSVC 2019/2022 or MinGW compiler
3. **Add Qt to PATH**: `C:\Qt\6.x.x\msvcXXXX_64\bin`

---

## 🎯 Method 1: qmake Build (Recommended)

### 🐧🍎 Linux/macOS

#### Option A: Automated Script
```bash
# Make script executable (first time only)
chmod +x build_qmake.sh

# Build and get detailed output
./build_qmake.sh
```

#### Option B: Manual Steps
```bash
# Clean previous build
rm -rf debug release *.o moc_* Makefile*

# Generate Makefile
qmake RadarVisualization.pro

# Build (using all CPU cores)
make -j$(nproc)

# Run
./release/RadarVisualization
```

### 🪟 Windows

#### Option A: Automated Script
```cmd
build_qmake.bat
```

#### Option B: Manual Steps (Command Prompt)
```cmd
# Clean previous build
rmdir /s /q debug release
del *.o moc_* Makefile*

# Generate Makefile
qmake RadarVisualization.pro

# Build (MSVC)
nmake

# OR Build (MinGW)
mingw32-make

# Run
release\RadarVisualization.exe
```

#### Option C: Qt Creator IDE
1. **Open** `RadarVisualization.pro` in Qt Creator
2. **Configure** project with desired Qt kit
3. **Build** → Build Project (Ctrl+B)
4. **Run** → Run (Ctrl+R)

---

## 🏗️ Method 2: CMake Build

### 🐧🍎 Linux/macOS

#### Option A: Automated Script
```bash
# Make script executable (first time only)
chmod +x build.sh

# Build
./build.sh
```

#### Option B: Manual Steps
```bash
# Create build directory
mkdir -p build && cd build

# Configure
cmake ..

# Build
make -j$(nproc)

# Run
./RadarVisualization
```

### 🪟 Windows

#### Command Prompt
```cmd
# Create build directory
mkdir build && cd build

# Configure (adjust Qt path)
cmake .. -DCMAKE_PREFIX_PATH="C:\Qt\6.7.0\msvc2022_64"

# Build
cmake --build . --config Release

# Run
Release\RadarVisualization.exe
```

#### Visual Studio
```cmd
# Generate Visual Studio solution
cmake .. -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH="C:\Qt\6.7.0\msvc2022_64"

# Open solution in Visual Studio
start RadarVisualization.sln
```

---

## 🎯 Execution Instructions

### 🚀 Running the Application

Once built successfully, the executable will be located at:

- **Linux/macOS qmake**: `./release/RadarVisualization`
- **Linux/macOS CMake**: `./build/RadarVisualization`
- **Windows qmake**: `release\RadarVisualization.exe`
- **Windows CMake**: `build\Release\RadarVisualization.exe`

### 🎮 Application Controls

1. **Launch the application**
2. **Simulation Mode** (enabled by default):
   - Random targets appear on PPI display
   - Real-time FFT spectrum of simulated ADC data
   - Toggle simulation with "Enable/Disable Simulation" button

3. **Range Control**:
   - Adjust PPI display range (1-50 km) using spinbox
   - Targets outside range are automatically filtered

4. **Network Mode**:
   - Send UDP data to port 5000
   - Application automatically receives and displays real data
   - Status bar shows connection status

### 📊 Features in Action

- **🎯 PPI Display**: Semi-circular radar plot with color-coded targets
  - Red: Approaching targets (positive radial speed)
  - Blue: Receding targets (negative radial speed)
  - Green: Stationary targets
  
- **📈 FFT Spectrum**: Real-time frequency domain analysis
  - -80 to +20 dB magnitude range
  - Grid overlay with frequency labels
  
- **📋 Track Table**: Complete target information
  - 8 columns with all target parameters
  - Auto-sizing and real-time updates

---

## 🔍 Troubleshooting

### ❌ Common Build Issues

#### "qmake not found"
```bash
# Linux: Install Qt development packages
sudo apt-get install qt6-base-dev-tools

# macOS: Add Qt to PATH
export PATH="/opt/homebrew/bin:$PATH"

# Windows: Add Qt bin directory to PATH
set PATH=%PATH%;C:\Qt\6.7.0\msvc2022_64\bin
```

#### "Qt5/Qt6 not found" (CMake)
```bash
# Set Qt path explicitly
export Qt6_DIR="/usr/lib/x86_64-linux-gnu/cmake/Qt6"

# OR for Windows
set Qt6_DIR=C:\Qt\6.7.0\msvc2022_64\lib\cmake\Qt6
```

#### Compiler Errors
```bash
# Ensure C++17 support
g++ --version    # Should be 7.0+
clang --version  # Should be 5.0+

# Windows: Use Visual Studio 2019+ or recent MinGW
```

#### Missing Network Module
```bash
# Linux
sudo apt-get install qt6-base-dev

# Windows: Reinstall Qt with Network module checked
```

### 🐞 Runtime Issues

#### "Cannot bind to UDP port 5000"
- Check if port is already in use: `netstat -an | grep 5000`
- Try running as administrator/root (not recommended for production)
- Check firewall settings

#### Black/Empty Display
- Update graphics drivers
- Try running with software rendering: `QT_QUICK_BACKEND=software`

#### Performance Issues
- Close other applications
- Reduce update rate if needed (modify `UPDATE_INTERVAL_MS`)
- Check CPU usage with task manager

---

## 📁 Project Structure

```
RadarVisualization/
├── 🔧 Build Files
│   ├── CMakeLists.txt          # CMake configuration
│   ├── RadarVisualization.pro  # qmake project file
│   ├── build.sh               # CMake build script (Linux/macOS)
│   ├── build_qmake.sh         # qmake build script (Linux/macOS)
│   └── build_qmake.bat        # qmake build script (Windows)
├── 💻 Source Code
│   ├── main.cpp               # Application entry point
│   ├── MainWindow.h/.cpp      # Main window implementation
│   ├── PPIWidget.h/.cpp       # PPI radar display widget
│   ├── FFTWidget.h/.cpp       # FFT spectrum widget
│   └── DataStructures.h       # Data type definitions
├── 📚 Documentation
│   ├── README.md              # User guide
│   ├── COMPILATION_GUIDE.md   # This file
│   └── IMPLEMENTATION_SUMMARY.md # Technical details
└── 🏗️ Generated (after build)
    ├── build/                 # CMake build artifacts
    ├── debug/                 # qmake debug build
    └── release/               # qmake release build
```

---

## 🚀 Performance Notes

- **Update Rate**: 50ms (20 FPS) for real-time visualization
- **Memory Usage**: ~10-50 MB depending on data size
- **CPU Usage**: ~5-15% on modern systems
- **Network**: UDP port 5000 for data reception

## 🎯 Next Steps

1. **Customize Data Sources**: Modify UDP message parsing for your specific radar data format
2. **Extend FFT Analysis**: Add windowing functions, spectrum averaging, peak detection
3. **Enhance PPI Display**: Add track history trails, range-azimuth grid overlays
4. **Add Export Features**: Save screenshots, data logging, configuration files

---

## 💡 Tips for Developers

- **IDE Integration**: Use Qt Creator for best development experience
- **Debugging**: Build debug version and use Qt Creator's debugger
- **Profiling**: Use Qt Creator's performance analyzer for optimization
- **Testing**: Simulation mode allows testing without real radar hardware