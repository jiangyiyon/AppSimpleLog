@echo off
echo Configuring for Visual Studio 2026...
echo.
cd /d %~dp0
echo Current directory: %CD%
echo.

if exist build rmdir /s /q build
mkdir build
mkdir build\debug

echo Running CMake...
cmake -S . -B build\debug -G "Visual Studio 18 2026" -A x64 -DBUILD_TESTING=ON

if errorlevel 1 (
    echo.
    echo CMake configuration FAILED!
    echo.
    pause
    exit /b 1
)

echo.
echo CMake configuration SUCCESSFUL!
echo Solution file: build\debug\AppSimpleLog.sln
echo.
pause
