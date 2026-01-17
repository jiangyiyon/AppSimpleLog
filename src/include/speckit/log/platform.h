// Platform detection header for cross-platform support
// Supports Windows (x64/arm64) and macOS (x64/arm64)

#pragma once

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
    #define SPECKIT_PLATFORM_WINDOWS
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_MAC
        #define SPECKIT_PLATFORM_MACOS
    #endif
#else
    #error "Unsupported platform"
#endif

// Architecture detection
#if defined(_M_X64) || defined(__x86_64__)
    #define SPECKIT_ARCH_X64
#elif defined(_M_ARM64) || defined(__aarch64__)
    #define SPECKIT_ARCH_ARM64
#else
    #error "Unsupported architecture"
#endif

// Platform-specific includes and definitions
#ifdef SPECKIT_PLATFORM_WINDOWS
    #define SPECKIT_EXPORT __declspec(dllexport)
    #define SPECKIT_IMPORT __declspec(dllimport)
#else
    #define SPECKIT_EXPORT __attribute__((visibility("default")))
    #define SPECKIT_IMPORT
#endif

// Process ID and thread ID types
#ifdef SPECKIT_PLATFORM_WINDOWS
    #include <windows.h>
    #include <thread>
    using ProcessIdType = DWORD;
    using ThreadIdType = DWORD;
#else
    #include <unistd.h>
    #include <thread>
    using ProcessIdType = pid_t;
    using ThreadIdType = std::thread::id;  // Use C++20 std::thread::id
#endif
