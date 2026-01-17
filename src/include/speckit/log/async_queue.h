// Lock-free queue for async logging
// Thread-safe queue with atomic operations for high-performance

#pragma once

#include <atomic>
#include <memory>
#include <vector>
#include "log_entry.h"



/// Lock-free queue for async log submission
/// Uses atomic ring buffer for thread-safe operations
class AsyncQueue {
public:
    /// Constructor with fixed capacity
    /// @param capacity Maximum number of log entries
    explicit AsyncQueue(size_t capacity);
    
    /// Destructor
    ~AsyncQueue();
    
    /// Add log entry to queue (non-blocking)
    /// @param entry Log entry to add
    /// @return true if added successfully, false if queue full
    bool tryPush(const LogEntry& entry);
    
    /// Get all log entries from queue
    /// @return Vector of all entries currently in queue
    std::vector<LogEntry> popAll();
    
    /// Check if queue is empty
    /// @return true if queue has no entries
    bool isEmpty() const;
    
    /// Get current number of entries in queue
    /// @return Number of entries in queue
    size_t size() const;
    
    /// Get queue capacity
    /// @return Maximum number of entries queue can hold
    size_t capacity() const;

private:
    std::unique_ptr<LogEntry[]> buffer_;  ///< Pre-allocated buffer
    std::atomic<size_t> write_index_;    ///< Index where next entry will be written
    std::atomic<size_t> read_index_;     ///< Index where oldest entry is located
    std::atomic<size_t> size_;           ///< Current number of entries
    size_t capacity_;                     ///< Total capacity of queue
};


