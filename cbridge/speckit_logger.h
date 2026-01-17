// Opaque pointer for C-Bridge pattern
// Enables ABI-stable C interface for language interoperability

#pragma once

// Export macro for Windows DLL
#ifdef _WIN32
    #ifdef SPECKITBRIDGE_EXPORTS
        #define SPECKITBRIDGE_API __declspec(dllexport)
    #else
        #define SPECKITBRIDGE_API __declspec(dllimport)
    #endif
#else
    #define SPECKITBRIDGE_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration for C++ Logger class
typedef struct SpeckitLogger SpeckitLogger;

// Log level enumeration for C API
typedef enum {
    SPECKIT_LOG_LEVEL_DEBUG = 0,
    SPECKIT_LOG_LEVEL_INFO = 1,
    SPECKIT_LOG_LEVEL_WARNING = 2,
    SPECKIT_LOG_LEVEL_ERROR = 3
} SpeckitLogLevel;

// Return codes for C API
typedef enum {
    SPECKIT_SUCCESS = 0,
    SPECKIT_ERROR_INVALID_ARGUMENT = -1,
    SPECKIT_ERROR_NULL_POINTER = -2,
    SPECKIT_ERROR_ALREADY_INITIALIZED = -3,
    SPECKIT_ERROR_NOT_INITIALIZED = -4,
    SPECKIT_ERROR_FILE_IO = -5,
    SPECKIT_ERROR_MEMORY = -6,
    SPECKIT_ERROR_THREAD = -7
} SpeckitErrorCode;

// Create a logger instance
// @param config Logger configuration (e.g., base filename)
// @return Logger handle or nullptr on error
SPECKITBRIDGE_API SpeckitLogger* speckit_logger_create(const char* config);

// Log a message
// @param logger Logger handle
// @param level Log level (SpeckitLogLevel)
// @param tag Log category tag
// @param message Log message
// @return SPECKIT_SUCCESS on success, error code otherwise
SPECKITBRIDGE_API SpeckitErrorCode speckit_logger_log(SpeckitLogger* logger, int level,
                                     const char* tag, const char* message);

// Set minimum log level
// @param logger Logger handle
// @param level Minimum log level
// @return SPECKIT_SUCCESS on success, error code otherwise
SPECKITBRIDGE_API SpeckitErrorCode speckit_logger_set_log_level(SpeckitLogger* logger, int level);

// Destroy logger instance
// @param logger Logger handle
// @return SPECKIT_SUCCESS on success, error code otherwise
SPECKITBRIDGE_API SpeckitErrorCode speckit_logger_destroy(SpeckitLogger* logger);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include "speckit/log/log_level.h"

    // Convert C log level to C++ log level
    inline LogLevel toCppLogLevel(SpeckitLogLevel level) {
        return static_cast<LogLevel>(level);
    }

    // Convert C++ log level to C log level
    inline SpeckitLogLevel toCLogLevel(LogLevel level) {
        return static_cast<SpeckitLogLevel>(level);
    }
#endif
