@echo off
setlocal enabledelayedexpansion

REM Radar Visualization qmake Build Script for Windows
REM This script uses qmake (Qt's traditional build system)

echo 🎯 Building Radar Data Visualization with qmake on Windows...
echo.

REM Function to check if command exists
where qmake >nul 2>&1
if %errorlevel% equ 0 (
    set QMAKE_CMD=qmake
    goto :found_qmake
)

where qmake6 >nul 2>&1
if %errorlevel% equ 0 (
    set QMAKE_CMD=qmake6
    goto :found_qmake
)

where qmake-qt6 >nul 2>&1
if %errorlevel% equ 0 (
    set QMAKE_CMD=qmake-qt6
    goto :found_qmake
)

REM Check common Qt installation paths
set QT_PATHS=C:\Qt\6.7.0\msvc2022_64\bin;C:\Qt\6.6.0\msvc2022_64\bin;C:\Qt\6.5.0\msvc2022_64\bin;C:\Qt\5.15.2\msvc2019_64\bin

for %%P in (%QT_PATHS%) do (
    if exist "%%P\qmake.exe" (
        set QMAKE_CMD="%%P\qmake.exe"
        echo 📦 Found qmake at: %%P
        goto :found_qmake
    )
)

echo ❌ Error: qmake not found!
echo.
echo Please install Qt and ensure qmake is in your PATH:
echo.
echo 🪟 Windows Installation:
echo   1. Download Qt from https://www.qt.io/download
echo   2. Install Qt Creator and Qt libraries
echo   3. Add Qt bin directory to PATH, for example:
echo      C:\Qt\6.7.0\msvc2022_64\bin
echo   4. Or use Qt Command Prompt
echo.
echo 🔧 Alternative: Use Qt Creator
echo   1. Open RadarVisualization.pro in Qt Creator
echo   2. Configure project with desired Qt kit
echo   3. Build and run from IDE
echo.
pause
exit /b 1

:found_qmake
echo 📦 Using qmake: %QMAKE_CMD%

REM Check Qt version
for /f "tokens=*" %%i in ('%QMAKE_CMD% -query QT_VERSION 2^>nul') do set QT_VERSION=%%i
if defined QT_VERSION (
    echo 🔧 Qt Version: %QT_VERSION%
) else (
    echo ⚠️  Warning: Could not determine Qt version
)

echo.

REM Clean previous build
echo 🧹 Cleaning previous build...
if exist debug rmdir /s /q debug
if exist release rmdir /s /q release
del /q *.o moc_* ui_* qrc_* Makefile* 2>nul

REM Generate Makefiles
echo 🔨 Generating Makefiles with qmake...
%QMAKE_CMD% RadarVisualization.pro

if %errorlevel% neq 0 (
    echo ❌ qmake failed!
    echo.
    echo 🔍 Troubleshooting:
    echo   • Make sure all source files are present
    echo   • Check that Qt development packages are properly installed
    echo   • Verify that qmake can find Qt libraries
    echo   • Try running: %QMAKE_CMD% -query
    echo   • Make sure you're using the correct compiler kit
    pause
    exit /b 1
)

echo ✅ Makefile generated successfully

REM Build the project
echo.
echo ⚙️  Building project...

REM Check for nmake (Visual Studio) or mingw32-make
where nmake >nul 2>&1
if %errorlevel% equ 0 (
    set MAKE_CMD=nmake
    echo 🚀 Building with nmake...
) else (
    where mingw32-make >nul 2>&1
    if %errorlevel% equ 0 (
        set MAKE_CMD=mingw32-make
        echo 🚀 Building with mingw32-make...
    ) else (
        where make >nul 2>&1
        if %errorlevel% equ 0 (
            set MAKE_CMD=make
            echo 🚀 Building with make...
        ) else (
            echo ❌ Error: No suitable make tool found!
            echo.
            echo Please ensure one of the following is in your PATH:
            echo   • nmake (Visual Studio)
            echo   • mingw32-make (MinGW)
            echo   • make (MSYS2/Cygwin)
            pause
            exit /b 1
        )
    )
)

%MAKE_CMD%

if %errorlevel% equ 0 (
    echo.
    echo 🎉 Build successful!
    echo.
    
    REM Find the executable
    if exist "release\RadarVisualization.exe" (
        set EXECUTABLE=release\RadarVisualization.exe
    ) else if exist "debug\RadarVisualization.exe" (
        set EXECUTABLE=debug\RadarVisualization.exe
    ) else if exist "RadarVisualization.exe" (
        set EXECUTABLE=RadarVisualization.exe
    ) else (
        echo ⚠️  Warning: Could not find executable
        set EXECUTABLE=
    )
    
    if defined EXECUTABLE (
        echo 📍 Executable location: !EXECUTABLE!
        echo.
        echo 🚀 Run the application:
        echo    !EXECUTABLE!
        echo.
        echo    Or double-click the executable in Windows Explorer
    )
    
    echo.
    echo ✨ Features included:
    echo   🎯 Semi-circular PPI radar display (-90° to +90°)
    echo   📊 Real-time FFT spectrum analysis
    echo   📋 Target tracking table with all parameters
    echo   🌐 UDP data reception on port 5000
    echo   🎭 Built-in data simulation for testing
    echo   ⚡ 50ms update rate for real-time performance
    echo.
    echo 💡 Tips:
    echo   • Use the range control to adjust PPI display (1-50 km)
    echo   • Toggle simulation on/off with the button
    echo   • Send UDP data to port 5000 for real operation
    echo   • All panels are resizable using splitters
    
) else (
    echo.
    echo ❌ Build failed!
    echo.
    echo 🔍 Common issues and solutions:
    echo   • Missing Qt modules: Reinstall Qt with Network module
    echo   • Compiler errors: Check C++17 support in your compiler
    echo   • Linker errors: Verify Qt libraries are properly installed
    echo   • Path issues: Make sure Qt bin directory is in PATH
    echo.
    echo 📋 For detailed error analysis:
    echo   %MAKE_CMD% clean
    echo   %MAKE_CMD%
)

echo.
echo Press any key to continue...
pause >nul