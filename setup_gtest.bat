@echo off
echo ================================================
echo GTest Setup Script for AppSimpleLog
echo ================================================
echo.
echo This script will download and set up Google Test.
echo.

cd /d %~dp0

REM Check if GTest already exists
if exist "third_party\googletest" (
    echo GTest already exists in third_party\googletest
    echo.
    choice /C YN /M "Do you want to re-download and rebuild GTest"
    if errorlevel 2 (
        echo Keeping existing GTest installation
        goto :configure_cmake
    )
    echo Removing existing GTest installation...
    rmdir /s /q third_party\googletest
)

echo.
echo Step 1: Downloading Google Test...
echo.

if not exist third_party mkdir third_party
cd third_party

REM Clone GoogleTest repository
echo Cloning from GitHub...
git clone --depth 1 --branch v1.14.0 https://github.com/google/googletest.git googletest
if errorlevel 1 (
    echo.
    echo ERROR: Failed to clone GoogleTest repository
    echo.
    echo Possible causes:
    echo 1. Git is not installed or not in PATH
    echo 2. Network connection issues
    echo 3. GitHub is blocked
    echo.
    echo Alternative: Download manually from:
    echo https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    echo.
    pause
    exit /b 1
)

echo.
echo Step 2: Building Google Test...
echo.

cd googletest
if not exist build mkdir build

echo Configuring GTest with CMake...
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 ^
    -DCMAKE_INSTALL_PREFIX="%~dp0install" ^
    -DBUILD_SHARED_LIBS=ON ^
    -Dgtest_force_shared_crt=ON

if errorlevel 1 (
    echo.
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)

echo.
echo Building GTest (this may take a few minutes)...
cmake --build build --config Release --parallel

if errorlevel 1 (
    echo.
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo.
echo Step 3: Installing GTest...
echo.

cmake --install build --config Release

if errorlevel 1 (
    echo.
    echo ERROR: Installation failed
    pause
    exit /b 1
)

echo.
echo Step 4: Configuring AppSimpleLog with GTest...
echo.

cd ..\..

:configure_cmake
if exist build rmdir /s /q build
mkdir build\debug

echo Configuring CMake with GTest path...
cmake -S . -B build\debug -G "Visual Studio 18 2026" -A x64 ^
    -DGTEST_ROOT="%~dp0third_party\install" ^
    -DBUILD_TESTING=ON

if errorlevel 1 (
    echo.
    echo WARNING: CMake configuration completed with warnings
    echo GTest might not be properly detected
    echo.
    echo Try alternative approach: set CMAKE_PREFIX_PATH
    set CMAKE_PREFIX_PATH=%~dp0third_party\install
    cmake -S . -B build\debug -G "Visual Studio 18 2026" -A x64 -DBUILD_TESTING=ON
)

echo.
echo ================================================
echo Setup Complete!
echo ================================================
echo.
echo GTest has been installed to: %~dp0third_party\install
echo.
echo Next steps:
echo 1. Run: configure_vs2026.bat to configure the project
echo 2. Build: cmake --build build\debug --config Debug
echo 3. Run tests: build\debug\Debug\unit_tests.exe
echo.
pause
