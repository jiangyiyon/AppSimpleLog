@echo off
cd /d %~dp0
cmake -S . -B build\debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
if errorlevel 1 (
    echo CMake configuration failed
    exit /b 1
)
cmake --build build\debug
if errorlevel 1 (
    echo Build failed
    exit /b 1
)
echo Build successful!
