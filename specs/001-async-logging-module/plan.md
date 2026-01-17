# Implementation Plan: 高性能异步跨平台日志模块

**Branch**: `001-async-logging-module` | **Date**: 2025-01-16 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `/specs/001-async-logging-module/spec.md`

## Summary

Implement a high-performance, asynchronous, cross-platform logging library for Windows and macOS (x64/arm64) using C++20 with C-Bridge pattern for Objective-C/Swift compatibility. The system provides four log levels (DEBUG, INFO, WARNING, ERROR), process isolation, async file rotation, tag-based filtering, and ZIP archiving. Key technical constraints: zero third-party dependencies, <1ms log call latency, crash-safe flush with ring buffer, and 100K+ log throughput without data loss.

## Technical Context

**Language/Version**: C++20 (standard library only)
**Primary Dependencies**: Zero third-party dependencies (C++20 stdlib: std::format, std::source_location, std::atomic, std::jthread, std::condition_variable_any)
**Storage**: Local text files with ZIP compression for archiving
**Testing**: GoogleTest framework for unit/integration testing
**Target Platform**: Windows (x64/arm64) and macOS (x64/arm64)
**Project Type**: Single project (C++ library with C bridge)
**Performance Goals**: <1ms log call latency, 10+ concurrent threads, 100K+ logs without loss, <100ms file rotation, <5s archiving 10MB
**Constraints**: <200MB memory footprint, crash-safe with 90% data preservation, ABI-stable C interface, no C++ exceptions in public API
**Scale/Scope**: Single application instance, up to 3 concurrent processes per executable, 24-hour continuous operation

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

### I. Core Technology Decisions

**API Expose**: ✅ C-Bridge pattern required - Export headers with extern "C" for ABI compatibility
- **Requirement**: FR-002 requires pure C++ API, FR-003 requires Objective-C compatibility
- **Plan**: Implement opaque pointer pattern with SpeckitLogger struct and prefixed C functions

**Dependency Control**: ✅ Zero third-party dependencies required
- **Requirement**: FR-001 cross-platform support
- **Plan**: Use only C++20 standard library features with compile-time feature detection

**Reliability**: ✅ Crash-Safe Flush required
- **Requirement**: FR-029 (normal exit), FR-030 (crash recovery), SC-009/SC-010
- **Plan**: Register signal handlers (SIGSEGV, SIGABRT) and emergency sync writes via ring buffer

### II. C-Bridge Design

**Opaque Pointer Pattern**: ✅ Required
- **Plan**: `typedef struct SpeckitLogger SpeckitLogger;` with factory pattern

**Swift Friendly**: ✅ Optional but recommended
- **Plan**: Use NS_ENUM/CF_ENUM macros for C enums if Swift integration is needed

### III. C++20 Implementation Constraints

**Formatting Engine**: ✅ std::format with feature detection
- **Requirement**: FR-010 precise formatting with timestamp
- **Plan**: Use std::format with conditional compilation for older compilers

**Source Location Capture**: ✅ std::source_location required
- **Requirement**: FR-010 includes timestamp, process ID, thread ID
- **Plan**: Capture at macro level, avoid passing through C bridge

**Async Architecture**: ✅ std::jthread with std::condition_variable_any
- **Requirement**: FR-027 (<1ms latency), FR-028 (background thread)
- **Plan**: Single background writer thread with lock-free queue for submissions

**Crash Handling**: ✅ Atomic flag + Ring Buffer required
- **Requirement**: SC-010 (90% crash preservation)
- **Plan**: Emergency flush handler on crash signals

### IV. Google C++ Standards

**Ownership**: ✅ std::unique_ptr required
- **Plan**: Manage file streams and buffers with RAII

**Error Handling**: ✅ No exceptions in C layer
- **Requirement**: SC-009 (100% data preservation on normal exit)
- **Plan**: Return integer error codes from C functions

**Naming**: ✅ speckit::log:: (C++) and Speckit* (C export)
- **Plan**: Follow specified naming convention

### V. Memory & Performance

**Pre-allocation**: ✅ Required during initialization
- **Requirement**: SC-006 (stable memory over 24 hours)
- **Plan**: Pre-allocate ring buffer and log entry pool

**String Views**: ✅ Use std::string_view internally
- **Requirement**: FR-027 (<1ms latency)
- **Plan**: Minimize copying, pass views to background thread

### Gate Status: ✅ PASSED - All constitutional requirements satisfied

## Project Structure

### Documentation (this feature)

```text
specs/001-async-logging-module/
├── spec.md              # Feature specification
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (/speckit.plan command)
├── data-model.md        # Phase 1 output (/speckit.plan command)
├── quickstart.md        # Phase 1 output (/speckit.plan command)
├── contracts/           # Phase 1 output (/speckit.plan command)
│   └── c-api.h        # C API interface contract
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code (repository root)

```text
src/
├── include/
│   └── speckit/
│       └── log/
│           ├── logger.h          # C++ internal logger class
│           ├── log_entry.h       # Log entry data structure
│           ├── log_buffer.h      # Ring buffer for crash safety
│           ├── file_manager.h    # File rotation and management
│           ├── tag_filter.h      # Tag filtering logic
│           └── archive.h        # ZIP archiving
├── src/
│   ├── logger.cpp
│   ├── log_entry.cpp
│   ├── log_buffer.cpp
│   ├── file_manager.cpp
│   ├── tag_filter.cpp
│   ├── archive.cpp
│   └── crash_handler.cpp       # Signal handlers
├── cbridge/
│   ├── speckit_logger.h         # C API header (extern "C")
│   └── speckit_logger.cpp       # C implementation layer
└── tests/
    ├── unit/
    │   ├── test_log_entry.cpp
    │   ├── test_tag_filter.cpp
    │   ├── test_log_buffer.cpp
    │   └── test_file_manager.cpp
    ├── integration/
    │   ├── test_async_logging.cpp
    │   ├── test_file_rotation.cpp
    │   ├── test_multi_process.cpp
    │   ├── test_archive.cpp
    │   └── test_crash_recovery.cpp
    └── performance/
        ├── test_latency.cpp
        ├── test_throughput.cpp
        └── test_stress.cpp

CMakeLists.txt                # Build configuration
README.md                     # Project documentation
```

**Structure Decision**: Single project structure with C++ library and C bridge layer. The C bridge directory provides the public API, while src/ contains all C++ implementation. Tests are organized by type (unit, integration, performance) to support different testing strategies.

## Complexity Tracking

> No constitutional violations requiring justification.
