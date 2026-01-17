// C bridge implementation for language interoperability
// Provides ABI-stable C interface

#include "speckit_logger.h"
#include "speckit/log/async_logger.h"
#include "speckit/log/log_level.h"
#include <memory>

// Forward declaration for opaque pointer
struct SpeckitLogger {
    std::unique_ptr<AsyncLogger> logger;
};

extern "C" {

SpeckitLogger* speckit_logger_create(const char* config) {
    if (!config) {
        return nullptr;  // Invalid argument
    }

    try {
        // Create C++ async logger instance
        auto logger = AsyncLogger::create(config);

        // Wrap in opaque pointer
        SpeckitLogger* handle = new SpeckitLogger();
        handle->logger = std::move(logger);

        return handle;
    } catch (...) {
        return nullptr;  // Exception occurred
    }
}

SpeckitErrorCode speckit_logger_log(SpeckitLogger* logger, int level, 
                                      const char* tag, const char* message) {
    if (!logger || !logger->logger) {
        return SPECKIT_ERROR_NOT_INITIALIZED;
    }
    
    if (!tag || !message) {
        return SPECKIT_ERROR_NULL_POINTER;
    }
    
    if (level < SPECKIT_LOG_LEVEL_DEBUG || 
        level > SPECKIT_LOG_LEVEL_ERROR) {
        return SPECKIT_ERROR_INVALID_ARGUMENT;
    }
    
    try {
        // Convert C types to C++ types and log
        logger->logger->log(
            toCppLogLevel(static_cast<SpeckitLogLevel>(level)),
            std::string(tag),
            std::string(message)
        );
        
        return SPECKIT_SUCCESS;
    } catch (...) {
        return SPECKIT_ERROR_MEMORY;  // Exception occurred
    }
}

SpeckitErrorCode speckit_logger_set_log_level(SpeckitLogger* logger, int level) {
    if (!logger || !logger->logger) {
        return SPECKIT_ERROR_NOT_INITIALIZED;
    }
    
    if (level < SPECKIT_LOG_LEVEL_DEBUG || 
        level > SPECKIT_LOG_LEVEL_ERROR) {
        return SPECKIT_ERROR_INVALID_ARGUMENT;
    }
    
    try {
        // Convert and set log level
        logger->logger->setLogLevel(
            toCppLogLevel(static_cast<SpeckitLogLevel>(level))
        );
        
        return SPECKIT_SUCCESS;
    } catch (...) {
        return SPECKIT_ERROR_MEMORY;
    }
}

SpeckitErrorCode speckit_logger_destroy(SpeckitLogger* logger) {
    if (!logger) {
        return SPECKIT_ERROR_NULL_POINTER;
    }
    
    // Delete C++ logger instance via unique_ptr destructor
    delete logger;
    
    return SPECKIT_SUCCESS;
}

} // extern "C"
