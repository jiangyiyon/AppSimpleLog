// Async logger for high-performance logging
// Uses background thread with lock-free queue

#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include <stop_token>
#include <semaphore>
#include "log_level.h"
#include "log_entry.h"
#include "file_manager.h"
#include "async_queue.h"
#include "log_buffer.h"
#include "crash_handler.h"
#include "platform.h"



/// Async logger with background writer thread
/// Provides <1ms log call latency with non-blocking operations
class AsyncLogger {
public:
    /// Create async logger instance. Returns nullptr on failure.
    /// @param base_name Base name for log files (supports absolute/relative paths, UTF-8 encoding)
    /// @param queue_size Size of async queue (default: 10000)
    static std::unique_ptr<AsyncLogger> create(const std::string& base_name,
                                               size_t queue_size = 10000);
    
    /// Destructor - ensures proper cleanup
    ~AsyncLogger();

    /// Initialize logger components. Returns true on success.
    bool initialize(const std::string& base_name, size_t queue_size);

    /// Deinitialize and stop background thread. Safe to call multiple times.
    void deinitialize();
    
    /// Set minimum log level
    /// @param level Minimum log level to output
    void setLogLevel(LogLevel level);
    
    /// Get current minimum log level
    /// @return Current minimum log level
    LogLevel getLogLevel() const;
    
    /// Log a message asynchronously
    /// @param level Log severity level
    /// @param tag Log category tag
    /// @param message User message to log
    void log(LogLevel level, const std::string& tag, const std::string& message);

    /// Flush all buffered logs to disk
    void flush();

private:
    AsyncLogger();
    
    /// Background writer thread function. The std::stop_token is provided by std::jthread.
    void writerThread(std::stop_token stop_token);

    /// Helper to format and write a collection of log entries.
    void writeEntries(const std::vector<LogEntry>& entries);

    /// Initialize components (extracted from constructor to support testing)
    /// Returns true on success.
    bool initializeComponents(const std::string& base_name, size_t queue_size);
    
    /// Emergency flush callback for crash handler
    void emergencyFlush();
    
    /// Get current process ID
    ProcessIdType getProcessId() const;
    
    /// Get current thread ID
    ThreadIdType getThreadId() const;
    
    /// Get current timestamp in milliseconds
    int64_t getTimestamp() const;

    std::unique_ptr<FileManager> file_manager_;  ///< File operations manager
    std::unique_ptr<AsyncQueue> async_queue_;    ///< Lock-free async queue
    std::unique_ptr<RingBuffer> ring_buffer_;     ///< Crash-safe ring buffer
    std::unique_ptr<CrashHandler> crash_handler_; ///< Crash handler
    std::jthread writer_thread_;                   ///< Background writer thread
    std::counting_semaphore<1> sem_;               ///< Semaphore for thread synchronization (C++20)
    LogLevel min_level_ = LogLevel::kLogLevelDebug;       ///< Minimum log level
    ProcessIdType process_id_;                 ///< Cached process ID
    bool initialized_ = false;                   ///< Initialization state
    size_t queue_size_;                          ///< Async queue size
};


