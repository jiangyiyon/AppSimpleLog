@echo off
REM Visual Studio Build Script for AppSimpleLog P1 MVP
REM This script creates build directory, configures CMake, and builds with MSVC

echo ========================================
echo Visual Studio Build - AppSimpleLog P1 MVP
echo ========================================

REM Step 1: Create build directory
if not exist build mkdir build
cd build

REM Step 2: Configure CMake for Visual Studio
echo [1/4] Configuring CMake for Visual Studio...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
if errorlevel 1 (
    echo ERROR: CMake configuration failed
    echo.
    echo Common issues:
    echo   1. CMake not installed or not in PATH
    echo   2. Visual Studio 2022 not installed
    echo.
    echo To fix:
    echo   - Install CMake: https://cmake.org/download/
    echo   - Install Visual Studio 2022: https://visualstudio.microsoft.com/downloads/
    echo.
    echo Alternative: Use CMake GUI (cmake-gui.exe) if available
    pause
    exit /b 1
)

echo [2/4] CMake configuration completed
echo.

REM Step 3: Build solution with MSBuild
echo [3/4] Building Visual Studio solution...
cmake --build . --config Release
if errorlevel 1 (
    echo ERROR: Build failed
    echo.
    echo This may be due to:
    echo   1. Compilation errors in source code
    echo   2. Missing dependencies (GoogleTest)
    echo   3. Linker errors
    echo.
    echo Check build output above for specific errors
    echo.
    echo To open in Visual Studio for debugging:
    echo   - Double-click: AppSimpleLog.sln in build directory
    echo   - Or: devenv build\AppSimpleLog.sln
    pause
    exit /b 1
)

echo [4/4] Build completed successfully
echo.

REM Step 4: Report build status
echo ========================================
echo Build Summary
echo ========================================
echo.
echo Configuration: Release
echo Compiler: MSVC (Visual Studio 2022)
echo Platform: Windows x64
echo.
echo Binaries location: build\Release\
echo.
echo Libraries:
echo   - SpeckitLog.lib (static library)
echo   - SpeckitBridge.dll (shared library)
echo.
echo Test executables:
echo   - unit_tests.exe
echo   - integration_tests.exe
echo   - performance_tests.exe
echo.
echo ========================================

REM Step 5: Provide test execution instructions
echo.
echo To run tests, execute from build\Release\ directory:
echo.
echo   Unit tests:
echo     cd build\Release
echo     unit_tests.exe
echo.
echo   Integration tests:
echo     cd build\Release
echo     integration_tests.exe
echo.
echo   Performance tests:
echo     cd build\Release
echo     performance_tests.exe
echo.
echo ========================================
echo.
echo Build complete! Ready to run tests.
echo.
pause
