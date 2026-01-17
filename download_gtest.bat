@echo off
echo Downloading Google Test source code...
echo.

cd /d %~dp0

if not exist third_party mkdir third_party
cd third_party

echo Cloning GoogleTest v1.14.0...
git clone --depth 1 --branch v1.14.0 https://github.com/google/googletest.git

if errorlevel 1 (
    echo.
    echo ERROR: Failed to download
    echo.
    echo Manual download:
    echo 1. Visit: https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    echo 2. Download and extract to: third_party\googletest
    echo.
    pause
    exit /b 1
)

echo.
echo Download complete!
echo Now run: update_cmake_for_gtest.bat
pause
