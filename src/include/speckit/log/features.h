// Compile-time feature detection for C++20 features
// Provides feature detection macros for conditional compilation

#pragma once



// C++20 feature detection using __cpp_* macros

// std::format support
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
    #define SPECKIT_HAS_FORMAT 1
#else
    #define SPECKIT_HAS_FORMAT 0
#endif

// std::jthread support
#if defined(__cpp_lib_jthread)
    #define SPECKIT_HAS_JTHREAD 1
#else
    #define SPECKIT_HAS_JTHREAD 0
#endif

// std::source_location support
#if defined(__cpp_lib_source_location)
    #define SPECKIT_HAS_SOURCE_LOCATION 1
#else
    #define SPECKIT_HAS_SOURCE_LOCATION 0
#endif

// std::atomic_ref support (for lock-free queues)
#if defined(__cpp_lib_atomic_ref)
    #define SPECKIT_HAS_ATOMIC_REF 1
#else
    #define SPECKIT_HAS_ATOMIC_REF 0
#endif

// MSVC specific feature detection
#if defined(_MSC_VER)
    #if _MSC_VER >= 1930
        #define SPECKIT_MSVC_2022_OR_LATER 1
    #else
        #define SPECKIT_MSVC_2022_OR_LATER 0
    #endif
#endif

// Clang/Apple Clang specific feature detection
#if defined(__clang__)
    #if __clang_major__ >= 14
        #define SPECKIT_CLANG_14_OR_LATER 1
    #else
        #define SPECKIT_CLANG_14_OR_LATER 0
    #endif
#endif


