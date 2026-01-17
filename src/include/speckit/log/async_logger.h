// Async logger for high-performance logging
// Uses background thread with lock-free queue

#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <thread>
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
    /// Create async logger instance
    /// @param base_name Base name for log files (supports absolute/relative paths, UTF-8 encoding)
    /// @param queue_size Size of async queue (default: 10000)
    /// @return Unique pointer to async logger instance
    static std::unique_ptr<AsyncLogger> create(const std::string& base_name,
                                                   size_t queue_size = 10000);
    
    /// Destructor - ensures proper cleanup
    ~AsyncLogger();
    
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
    AsyncLogger(const std::string& base_name, size_t queue_size);
    
    /// Background writer thread function
    void writerThread();
    
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
    std::atomic<bool> running_;                  ///< Thread running flag
    std::atomic<bool> shutdown_requested_;         ///< Shutdown request flag
    LogLevel min_level_ = LogLevel::kLogLevelDebug;       ///< Minimum log level
    ProcessIdType process_id_;                 ///< Cached process ID
    bool initialized_ = false;                   ///< Initialization state
    size_t queue_size_;                          ///< Async queue size
};


