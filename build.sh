#!/bin/bash
# Build script for AppSimpleLog P1 MVP

echo "========================================"
echo "Building AppSimpleLog P1 MVP"
echo "========================================"

# Create build directory
mkdir -p build
cd build

# Configure CMake
echo "[1/3] Configuring CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed"
    exit 1
fi

# Build project
echo "[2/3] Building project..."
cmake --build . --config Release
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    exit 1
fi

echo "========================================"
echo "Build completed successfully!"
echo "========================================"
echo ""
echo "Binaries are in: build/Release/"
echo ""
echo "To run tests:"
echo "  - Unit tests: ./unit_tests"
echo "  - Integration tests: ./integration_tests"
echo "  - Performance tests: ./performance_tests"
echo ""
echo "========================================"
