@echo off
REM Build script for AppSimpleLog P1 MVP

echo ========================================
echo Building AppSimpleLog P1 MVP
echo ========================================

REM Create build directory
if not exist build mkdir build
cd build

REM Configure CMake
echo [1/3] Configuring CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
if errorlevel 1 (
    echo ERROR: CMake configuration failed
    exit /b 1
)

REM Build project
echo [2/3] Building project...
cmake --build . --config Release
if errorlevel 1 (
    echo ERROR: Build failed
    exit /b 1
)

echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo Binaries are in: build\Release\
echo.
echo To run tests:
echo   - Unit tests: unit_tests.exe
echo   - Integration tests: integration_tests.exe
echo   - Performance tests: performance_tests.exe
echo.
echo ========================================
