# Build Instructions for AppSimpleLog P1 MVP

**Date**: 2025-01-16
**Status**: Build scripts ready

---

## Quick Start

### Windows
```batch
build.bat
```

### macOS/Linux
```bash
chmod +x build.sh
./build.sh
```

---

## Manual Build Steps

If automated scripts don't work, follow these manual steps:

### 1. Create Build Directory
```bash
mkdir build
cd build
```

### 2. Configure CMake
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
```

**Expected Output**:
```
-- Configuring done
-- Generating done
-- Build files have been written to: ...
```

### 3. Build Project
```bash
cmake --build . --config Release
```

**Expected Output**:
```
[100%] Building CXX object...
[100%] Linking CXX executable...
```

---

## Troubleshooting

### Issue: CMake not found
**Solution**: Install CMake 3.20 or later
- Windows: `winget install Kitware.CMake`
- macOS: `brew install cmake`

### Issue: C++20 not supported
**Solution**: Upgrade compiler
- Windows: Install Visual Studio 2022
- macOS: Install Xcode 14+

### Issue: GoogleTest not found
**Solution**: Install GoogleTest
- Via vcpkg: `vcpkg install gtest`
- Via conda: `conda install -c conda-forge gtest`
- From source: Download and build GoogleTest

---

## Build Artifacts

After successful build, expect these files:

```
build/
├── CMakeCache.txt
├── CMakeFiles/
├── Makefile (or .sln for Visual Studio)
└── Release/ (or Debug/)
    ├── SpeckitLog.lib (or .a)
    ├── SpeckitBridge.dll (or .dylib)
    ├── unit_tests.exe
    ├── integration_tests.exe
    └── performance_tests.exe
```

---

## Running Tests

### Unit Tests
```bash
cd build
./unit_tests
```

### Integration Tests
```bash
cd build
./integration_tests
```

### Performance Tests
```bash
cd build
./performance_tests
```

---

## Known Issues

### Bash Execution Issues
Current environment has issues with bash execution through Git Bash. Use:
- **Windows**: Run `build.bat` directly
- **macOS/Linux**: Run `build.sh` directly

### Platform-Specific Build
The CMakeLists.txt is configured for both Windows and Unix systems. CMake will automatically detect the platform and use appropriate generators.

---

## Notes

All source files are ready for building. The build process will:
1. Configure CMake with Release optimizations
2. Enable testing support (GoogleTest)
3. Compile all C++ source files
4. Link libraries (SpeckitLog, SpeckitBridge)
5. Build test executables

Once built, you can run the test suites to validate P1 MVP functionality.
