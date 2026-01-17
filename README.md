# AppSimpleLog

A high-performance, asynchronous, cross-platform logging library for Windows and macOS (x64/arm64) written in C++20 with C-Bridge pattern for Objective-C/Swift compatibility.

## Features

- **High Performance**: <1ms log call latency with async background writing
- **Cross-Platform**: Support for Windows (x64/arm64) and macOS (x64/arm64)
- **Process Isolation**: Each process maintains independent log files
- **Log Levels**: DEBUG, INFO, WARNING, ERROR with dynamic level control
- **Tag Filtering**: Categorize and filter logs by tags with per-tag level control
- **File Rotation**: Automatic log file rotation with configurable retention policy
- **Crash Safety**: Ring buffer with emergency flush to preserve 90%+ logs on crash
- **Zero Dependencies**: Built entirely with C++20 standard library
- **C-Bridge API**: ABI-stable C interface for easy integration with any language
- **ZIP Archiving**: Optional log archiving for long-term storage and transport

## Performance

- **Latency**: <1ms average log call time
- **Throughput**: 100,000+ logs without data loss
- **Concurrency**: 10+ concurrent threads with <20% performance degradation
- **Memory**: Stable memory footprint over 24-hour operation
- **File Rotation**: <100ms rotation time
- **Archiving**: <5s for 10MB log file compression

## Requirements

- **Compiler**: C++20 compatible compiler
  - Windows: MSVC 2022 or later
  - macOS: Clang 14 or later (Xcode 14+)
- **Build System**: CMake 3.20 or later
- **Testing**: GoogleTest (optional, for running tests)

## Building

### Prerequisites

**Windows:**
```bash
# Install CMake
winget install Kitware.CMake

# Install Visual Studio 2022 or later with C++20 support
# Ensure C++ workload and CMake components are installed
```

**macOS:**
```bash
# Install CMake
brew install cmake

# Install Xcode 14+ or Command Line Tools
xcode-select --install
```

### Build Steps

```bash
# Clone repository
git clone <repository-url>
cd AppSimpleLog

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
cmake --build . --config Release

# (Optional) Run tests
ctest --output-on-failure
```

### Build Options

```bash
# Build without tests
cmake .. -DBUILD_TESTING=OFF

# Build with Debug symbols
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build with Release optimizations
cmake .. -DCMAKE_BUILD_TYPE=Release
```

## Installation

```bash
# Install to system directories (requires admin/sudo)
cmake --install .

# Or use cpack to create distribution packages
cpack
```

## Project Structure

```
src/
├── include/speckit/log/    # Public C++ API headers
├── src/                      # C++ implementation
└── tests/                    # Unit, integration, and performance tests

cbridge/                        # C API interface
└── speckit_logger.h          # Public C header
```

## Quick Start

### C++ Usage

```cpp
#include "speckit/log/logger.h"

int main() {
    // Create logger
    auto logger = speckit::log::Logger::create("MyApp");
    
    // Set log level
    logger->setLogLevel(speckit::log::LogLevel::kLogLevelInfo);

    // Log messages
    logger->log(speckit::log::LogLevel::kLogLevelInfo, "Network", "Connected to server");
    logger->log(speckit::log::LogLevel::kLogLevelError, "Database", "Connection failed");
    
    return 0;
}
```

### C API Usage

```c
#include "speckit_logger.h"

int main() {
    // Create logger
    SpeckitLogger* logger = speckit_logger_create("MyApp");
    
    // Set log level
    speckit_logger_set_log_level(logger, SPECKIT_LOG_LEVEL_INFO);
    
    // Log messages
    speckit_logger_log(logger, SPECKIT_LOG_LEVEL_INFO, "Network", "Connected to server");
    speckit_logger_log(logger, SPECKIT_LOG_LEVEL_ERROR, "Database", "Connection failed");
    
    // Clean up
    speckit_logger_destroy(logger);
    return 0;
}
```

### Objective-C/Swift Usage

```objc
#import "speckit_logger.h"

// Works seamlessly in Objective-C files
SpeckitLogger* logger = speckit_logger_create("MyApp");
speckit_logger_log(logger, SPECKIT_LOG_LEVEL_INFO, "UI", "Button clicked");
```

```swift
import Foundation

// Works in Swift via C bridging header
let logger = speckit_logger_create("MyApp")
speckit_logger_log(logger, SPECKIT_LOG_LEVEL_INFO, "UI", "Button clicked")
```

## Configuration

### Log Levels

```cpp
enum class LogLevel {
    kLogLevelDebug = 0,
    kLogLevelInfo = 1,
    kLogLevelWarning = 2,
    kLogLevelError = 3
};
```

### File Rotation

```cpp
// Set max file size (default: 10MB)
logger->setMaxFileSize(10 * 1024 * 1024);

// Set retention count (default: 3 files)
logger->setRetentionCount(3);
```

### Tag Filtering

```cpp
// Enable/disable tags
logger->setTagEnabled("Database", false);  // Disable Database logs
logger->setTagEnabled("Network", true);   // Enable Network logs

// Set per-tag level
logger->setTagLevel("UI", LogLevel::kLogLevelWarning);  // UI logs must be WARNING or higher
```

### Archiving

```cpp
// Manual archive
logger->archive();

// Auto-archive on exit
logger->setAutoArchive(true);
```

## Log Format

Each log entry follows this format:

```
[INFO] 2023-10-27 14:35:12.453 [1234, 5678] [Network]: Socket connected successfully.
```

Format breakdown:
- `[INFO]`: Log level
- `2023-10-27 14:35:12.453`: Timestamp (YYYY-MM-DD HH:MM:SS.mmm)
- `[1234, 5678]`: Process ID and Thread ID
- `[Network]`: Tag
- `Socket connected successfully.`: User message

## Testing

### Running Tests

```bash
cd build

# Run all tests
ctest

# Run specific test suites
./unit_tests
./integration_tests
./performance_tests
```

### Test Coverage

- **Unit Tests**: Core components (LogEntry, TagFilter, LogBuffer, FileManager)
- **Integration Tests**: Async logging, file rotation, multi-process, archiving, crash recovery
- **Performance Tests**: Latency (<1ms), throughput (100K logs), stress testing

## Performance Validation

The library is validated against these performance targets:

- ✅ <1ms average log call latency
- ✅ 100,000+ logs without data loss
- ✅ <100ms file rotation time
- ✅ <5s archiving for 10MB files
- ✅ Stable memory over 24-hour operation
- ✅ 90%+ data preservation on crash

## API Reference

### C++ API

See `src/include/speckit/log/logger.h` for complete C++ API reference.

### C API

See `cbridge/speckit_logger.h` for complete C API reference.

## Architecture

The library follows these core principles:

1. **C-Bridge Pattern**: ABI-stable C interface for language interoperability
2. **Zero Dependencies**: Pure C++20 standard library
3. **RAII Ownership**: std::unique_ptr for all resources
4. **Async Architecture**: Background writer thread with lock-free queue
5. **Crash Safety**: Ring buffer with emergency flush
6. **String Views**: Minimize copying with std::string_view

## Platform Support

| Platform | Architecture | Status |
|----------|-------------|---------|
| Windows 10/11 | x64, ARM64 | ✅ Supported |
| macOS 12+ | x64, ARM64 | ✅ Supported |

## License

[Add your license here]

## Contributing

[Add contribution guidelines here]

## Support

[Add support links here]

## Changelog

### Version 1.0.0 (2025-01-16)

- Initial release
- Core logging functionality
- Async writing with <1ms latency
- Process isolation
- Tag filtering
- File rotation
- Crash-safe flushing
- ZIP archiving
