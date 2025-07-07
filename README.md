# Radar Data Visualization - Qt C++ Application

A modern Qt C++ GUI application for real-time radar data visualization that receives UDP data on port 5000 and displays both target tracking data and raw ADC frequency spectrum data.

## Features

### 1. PPI (Plan Position Indicator) Display
- **Semi-circular radar plot** covering -90° to +90° azimuth
- **Color-coded targets** based on radial speed:
  - 🔴 **Red**: Approaching targets (positive radial speed)
  - 🔵 **Blue**: Receding targets (negative radial speed)  
  - 🟢 **Green**: Stationary targets (near-zero speed)
- **Range rings** and azimuth lines for easy reading
- **Target markers** with ID labels and size based on signal level
- **Adjustable range scale** (1-50 km)

### 2. FFT Spectrum Display
- **Real-time frequency domain plot** of raw ADC data
- **Magnitude spectrum in dB** with configurable range
- **Grid lines** and proper axis labeling
- **Built-in FFT implementation** (Cooley-Tukey algorithm)

### 3. Target Track Table
- **Comprehensive target information** in tabular format
- **Real-time updates** showing all target parameters:
  - Target ID, Level (dB), Range (m), Azimuth (°)
  - Elevation (°), Radial Speed (m/s)
  - Azimuth Speed (°/s), Elevation Speed (°/s)

### 4. Network & Data Handling
- **UDP receiver** listening on port 5000
- **50ms update rate** for real-time performance
- **Data simulation mode** for testing and demonstration
- **Modern C++17** with Qt best practices

## Data Structures

### Target Track Data
```cpp
struct TargetTrack {
    uint32_t target_id;
    float level;
    float radius;
    float azimuth;        // -90° to +90°
    float elevation;
    float radial_speed;   // m/s (+ approaching, - receding)
    float azimuth_speed;  // deg/s
    float elevation_speed; // deg/s
};
```

### Raw ADC Frame
```cpp
struct RawADCFrame {
    std::vector<float> sample_data;
    uint32_t frame_number;
    uint32_t num_chirps;
    uint8_t num_rx_antennas;
    uint32_t num_samples_per_chirp;
    uint8_t rx_mask;
    uint8_t adc_resolution;
    uint8_t interleaved_rx;
    Rx_Data_Format_t data_format;
};
```

## Build Requirements

- **Qt 5.12+ or Qt 6.x** (Core, Widgets, Network modules)
- **CMake 3.16+**
- **C++17 compatible compiler**
  - GCC 7+, Clang 5+, or MSVC 2019+

### Windows Build
```bash
# Install Qt and CMake
# Clone/download the project
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvcXXXX_64"
cmake --build . --config Release
```

### Linux Build
```bash
# Install Qt development packages
sudo apt-get install qt6-base-dev qt6-base-dev-tools cmake build-essential
# Or for Qt5: sudo apt-get install qtbase5-dev qtbase5-dev-tools

mkdir build && cd build
cmake ..
make -j$(nproc)
```

### macOS Build
```bash
# Install Qt via Homebrew
brew install qt cmake

mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
make -j$(sysctl -n hw.ncpu)
```

## Usage

1. **Launch the application**:
   ```bash
   ./RadarVisualization
   ```

2. **Simulation Mode** (default):
   - Application starts with simulated data enabled
   - Random targets and ADC data generated every 50ms
   - Toggle simulation on/off using the "Enable/Disable Simulation" button

3. **Network Mode**:
   - Send UDP data to port 5000
   - Application will automatically receive and display real data
   - Simulation can be disabled when receiving real data

4. **Controls**:
   - **Range Control**: Adjust PPI display range (1-50 km)
   - **Simulation Toggle**: Enable/disable simulated data
   - **Resizable Interface**: All panels auto-resize with window

## UDP Message Format

For real data integration, send UDP messages to port 5000 with the following format:

```cpp
struct MessageHeader {
    MessageType type;        // TARGET_TRACK_DATA = 1, RAW_ADC_DATA = 2
    uint32_t data_size;     // Size of following data in bytes
    uint64_t timestamp;     // Timestamp (optional)
};
```

Followed by the serialized target track data or raw ADC data.

## Architecture

- **MainWindow**: Main application window with layout management
- **PPIWidget**: Custom radar plot widget with polar coordinate display
- **FFTWidget**: Frequency spectrum display widget with built-in FFT
- **DataStructures**: Type definitions for radar data
- **CMake build system**: Cross-platform compilation support

## Key Features Implementation

- **Modern C++17** with smart pointers and RAII
- **Qt signals/slots** for event-driven architecture  
- **Custom painting** with QPainter for high-performance graphics
- **Efficient FFT** implementation for real-time spectrum analysis
- **Responsive UI** with QSplitter-based layout management
- **Thread-safe** UDP reception and data handling

## Performance

- **50ms update rate** (20 FPS) for smooth real-time visualization
- **Optimized rendering** with double buffering and minimal redraws
- **Memory efficient** data structures and algorithms
- **Scalable** to hundreds of targets and large FFT sizes

## License

This radar visualization application demonstrates modern Qt C++ development practices for real-time data visualization systems.
