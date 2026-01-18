// Ring buffer for crash-safe logging
// Pre-allocated circular buffer with atomic operations

#pragma once

#include <atomic>
#include <mutex>
#include <cstdint>
#include <memory>
#include <vector>
#include "log_entry.h"

/// Thread-safe ring buffer for crash-safe log storage
/// Pre-allocated during initialization to avoid runtime allocation
class RingBuffer {
public:
    /// Constructor with fixed capacity
    /// @param capacity Maximum number of log entries to store
    explicit RingBuffer(size_t capacity);

    /// Destructor
    ~RingBuffer() = default;

    /// Add a log entry to the buffer (non-blocking)
    /// @param entry Log entry to add (will be moved)
    /// @return true if added successfully, false if buffer full
    bool tryPush(LogEntry&& entry);

    /// Get all log entries and clear buffer
    /// @return Vector of all log entries currently in buffer
    std::vector<LogEntry> popAll();

    /// Check if buffer is empty
    /// @return true if buffer has no entries
    bool isEmpty() const;

    /// Get current number of entries in buffer
    /// @return Number of entries in buffer
    size_t size() const;

    /// Get buffer capacity
    /// @return Maximum number of entries buffer can hold
    size_t capacity() const;

private:
    std::unique_ptr<LogEntry[]> buffer_;              ///< Pre-allocated buffer
    std::atomic<size_t> head_;                        ///< Index where next entry will be written
    std::atomic<size_t> tail_;                        ///< Index where oldest entry is located
    std::atomic<size_t> size_;                        ///< Current number of entries
    const size_t capacity_;                           ///< Total capacity of buffer
};