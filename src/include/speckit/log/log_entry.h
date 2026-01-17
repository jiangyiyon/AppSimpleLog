// Log entry data structure
// Represents a single log record with all required fields

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include "log_level.h"
#include "platform.h"

/// Log entry structure containing all required information
/// Uses string_view to avoid copying until formatting
struct LogEntry {
    LogLevel level;              ///< Log severity level
    int64_t timestamp_ms;       ///< Timestamp in milliseconds since epoch
    ProcessIdType process_id;     ///< Process ID
    ThreadIdType thread_id;       ///< Thread ID
    std::string_view tag;      ///< Log category tag
    std::string_view message;   ///< User message

    /// Constructor with all fields
    LogEntry(LogLevel lvl, int64_t ts, ProcessIdType pid, ThreadIdType tid,
            std::string_view tg, std::string_view msg)
        : level(lvl), timestamp_ms(ts), process_id(pid), thread_id(tid),
          tag(tg), message(msg) {}

    /// Default constructor
    LogEntry() = default;

    /// Move constructor
    LogEntry(LogEntry&& other) noexcept
        : level(other.level),
          timestamp_ms(other.timestamp_ms),
          process_id(other.process_id),
          thread_id(other.thread_id),
          tag(other.tag),
          message(other.message) {}

    /// Move assignment
    LogEntry& operator=(LogEntry&& other) noexcept {
        if (this != &other) {
            level = other.level;
            timestamp_ms = other.timestamp_ms;
            process_id = other.process_id;
            thread_id = other.thread_id;
            tag = other.tag;
            message = other.message;
        }
        return *this;
    }

    /// Disable copy operations (use move semantics)
    LogEntry(const LogEntry&) = delete;
    LogEntry& operator=(const LogEntry&) = delete;
};

/// Format a log entry as a string
/// @param entry The log entry to format
/// @return Formatted log entry string
std::string formatLogEntry(const LogEntry& entry);
