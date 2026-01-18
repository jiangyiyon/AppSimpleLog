# libzip Installation Guide

This document provides instructions for installing libzip library on different platforms for the AppSimpleLog project.

## What is libzip?

libzip is a C library for reading, creating, and modifying zip archives. It is:
- **Lightweight**: Pure C library with minimal dependencies
- **Cross-platform**: Supports Windows, macOS, Linux
- **Mature**: Stable and widely used
- **Standard compliant**: Creates standard ZIP files compatible with any tool

## Installation by Platform

### Windows

#### Option 1: Using vcpkg (Recommended)

```bash
# Install vcpkg if not already installed
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install libzip
.\vcpkg install libzip:x64-windows
```

Then configure CMake to use vcpkg:
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake ..
```

#### Option 2: Using NuGet

```bash
# Install via NuGet
nuget install libzip.redist -Version 1.10.1
```

#### Option 3: Manual Build

1. Download source from https://libzip.org/
2. Extract and open in Visual Studio
3. Build for Release x64
4. Add include and lib paths to CMake

### macOS

#### Using Homebrew (Recommended)

```bash
# Install libzip
brew install libzip

# Verify installation
brew info libzip
```

CMake will automatically find libzip via pkg-config.

#### Using MacPorts

```bash
# Install libzip
sudo port install libzip
```

### Linux (Ubuntu/Debian)

```bash
# Install development package
sudo apt-get update
sudo apt-get install libzip-dev zlib1g-dev

# Verify installation
pkg-config --modversion libzip
```

### Linux (CentOS/RHEL/Fedora)

```bash
# Install development package
sudo yum install libzip-devel zlib-devel

# Or for Fedora
sudo dnf install libzip-devel zlib-devel
```

## Verification

After installation, verify that libzip is available:

```bash
# Check for pkg-config (Linux/macOS)
pkg-config --cflags --libs libzip

# Or check in CMake
cmake .. -DCMAKE_BUILD_TYPE=Release
# Should find libzip automatically
```

## Building the Project

After installing libzip, build the project:

### Windows

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### macOS/Linux

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Troubleshooting

### Error: "libzip not found"

**Windows**: Ensure libzip is installed and vcpkg toolchain is configured.

**macOS**: Run `brew install libzip` and try again.

**Linux**: Run `sudo apt-get install libzip-dev` (Ubuntu) or `sudo yum install libzip-devel` (CentOS).

### Error: "undefined reference to zip_open"

This indicates linking failure. Ensure:
- libzip is installed correctly
- CMake found the library during configuration
- Build is using Release configuration (Debug builds may have issues)

### CMake Warnings

You may see warnings like:
```
WARNING: libzip not found. Archive functionality may not work.
```

This is non-fatal - the project will build but archive operations won't work.

## Advanced Configuration

### Custom libzip Path

If libzip is installed in a non-standard location:

```bash
cmake .. -DLIBZIP_ROOT=/custom/path/to/libzip
```

### Static Linking

To link libzip statically (if static library is available):

```bash
cmake .. -DBUILD_SHARED_LIBS=OFF
```

## Additional Resources

- [libzip Official Website](https://libzip.org/)
- [libzip Documentation](https://libzip.org/documentation/)
- [libzip GitHub Repository](https://github.com/nih-at/libzip)
- [AppSimpleLog README](./README.md)

## License

libzip is licensed under BSD 3-Clause. See the libzip repository for details.
