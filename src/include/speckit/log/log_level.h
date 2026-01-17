// Log level enumeration for filtering log output
// Levels: DEBUG < INFO < WARNING < ERROR

#pragma once

#include <cstdint>

namespace SpeckitLog {

/// Log severity levels
/// Higher numbers indicate higher severity
enum class LogLevel : int8_t {
    kLogLevelDebug = 0,      ///< Detailed debugging information
    kLogLevelInfo = 1,       ///< General informational messages
    kLogLevelWarning = 2,     ///< Warning messages for potentially harmful situations
    kLogLevelError = 3         ///< Error messages for serious issues
};

/// Check if a log level meets the minimum required level
/// @param level The log level to check
/// @param minimum The minimum required level
/// @return true if level >= minimum
inline bool shouldLog(LogLevel level, LogLevel minimum) {
    return static_cast<int8_t>(level) >= static_cast<int8_t>(minimum);
}

/// Convert log level to string representation
/// @param level The log level
/// @return String representation of the level
constexpr const char* levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::kLogLevelDebug:   return "DEBUG";
        case LogLevel::kLogLevelInfo:    return "INFO";
        case LogLevel::kLogLevelWarning:  return "WARNING";
        case LogLevel::kLogLevelError:    return "ERROR";
        default:                          return "UNKNOWN";
    }
}

} // namespace SpeckitLog

// Bring symbols into global scope for convenience
using SpeckitLog::LogLevel;
using SpeckitLog::shouldLog;
using SpeckitLog::levelToString;
