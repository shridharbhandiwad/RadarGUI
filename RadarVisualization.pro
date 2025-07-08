QT += core widgets network

CONFIG += c++17
CONFIG += debug_and_release
CONFIG += warn_on

TARGET = RadarVisualization
TEMPLATE = app

# Version info
VERSION = 1.0.0
QMAKE_TARGET_PRODUCT = "Radar Data Visualization"
QMAKE_TARGET_DESCRIPTION = "Real-time radar data visualization with PPI display and FFT spectrum"
QMAKE_TARGET_COPYRIGHT = "Radar Systems"

# Sources
SOURCES += \
    main.cpp \
    MainWindow.cpp \
    PPIWidget.cpp \
    FFTWidget.cpp

# Headers
HEADERS += \
    MainWindow.h \
    PPIWidget.h \
    FFTWidget.h \
    DataStructures.h

# Platform-specific configurations
win32 {
    #RC_ICONS = radar.ico
    QMAKE_TARGET_COMPANY = "Radar Systems"
}

unix:!macx {
    # Linux-specific settings
    target.path = /usr/local/bin
    INSTALLS += target
}

macx {
    # macOS-specific settings
    ICON = radar.icns
    QMAKE_INFO_PLIST = Info.plist
}

# Compiler flags
*-g++* {
    QMAKE_CXXFLAGS += -Wall -Wextra -Wpedantic
    QMAKE_CXXFLAGS_RELEASE += -O3
}

*-msvc* {
    QMAKE_CXXFLAGS += /W4
    QMAKE_CXXFLAGS_RELEASE += /O2
}

# Output directories
CONFIG(debug, debug|release) {
    DESTDIR = debug
    OBJECTS_DIR = debug/obj
    MOC_DIR = debug/moc
    RCC_DIR = debug/rcc
    UI_DIR = debug/ui
}

CONFIG(release, debug|release) {
    DESTDIR = release
    OBJECTS_DIR = release/obj
    MOC_DIR = release/moc
    RCC_DIR = release/rcc
    UI_DIR = release/ui
}
