// Async logger implementation with background thread

#include "speckit/log/async_logger.h"
#include "speckit/log/log_level.h"
#include "speckit/log/log_entry.h"
#include <chrono>
#include <format>
#include <thread>



std::unique_ptr<AsyncLogger> AsyncLogger::create(const std::string& base_name, 
                                                     size_t queue_size) {
    return std::unique_ptr<AsyncLogger>(new AsyncLogger(base_name, queue_size));
}

AsyncLogger::AsyncLogger(const std::string& base_name, size_t queue_size)
    : process_id_(getProcessId()), queue_size_(queue_size), sem_(0) {
    
    // Create components
    file_manager_ = std::make_unique<FileManager>(base_name);
    async_queue_ = std::make_unique<AsyncQueue>(queue_size);
    ring_buffer_ = std::make_unique<RingBuffer>(queue_size);  // Same size for crash safety
    crash_handler_ = std::make_unique<CrashHandler>();
    
    // Initialize file manager
    if (!file_manager_->initialize(process_id_)) {
        initialized_ = false;
        return;
    }
    
    // Set crash handler callback
    crash_handler_->setFlushCallback([this]() { this->emergencyFlush(); });
    
    // Start background writer thread
    running_.store(true);
    shutdown_requested_.store(false);
    writer_thread_ = std::jthread([this]() { this->writerThread(); });
    
    initialized_ = true;
}

AsyncLogger::~AsyncLogger() {
    if (initialized_) {
        // Request shutdown
        shutdown_requested_.store(true);
        sem_.release();  // Signal writer thread to wake up
        
        // Wait for writer thread to finish
        if (writer_thread_.joinable()) {
            writer_thread_.join();
        }
        
        // Flush any remaining entries
        flush();
    }
}

void AsyncLogger::setLogLevel(LogLevel level) {
    min_level_ = level;
}

LogLevel AsyncLogger::getLogLevel() const {
    return min_level_;
}

void AsyncLogger::log(LogLevel level, const std::string& tag,
                const std::string& message) {
    // Check if message should be logged (fast path - no locking)
    if (!::shouldLog(level, min_level_)) {
        return;  // Filtered out - no queuing needed
    }
    
    if (!initialized_) {
        return;  // Logger not initialized
    }
    
    // Create log entry
    LogEntry entry(
        level,
        getTimestamp(),
        process_id_,
        getThreadId(),
        tag,
        message
    );
    
    // Try to push to async queue (non-blocking)
    if (async_queue_->tryPush(entry)) {
        // Success - notify writer thread
        sem_.release();
    } else {
        // Queue full - try ring buffer for crash safety
        if (!ring_buffer_->tryPush(entry)) {
            // Ring buffer also full - silently drop to prevent blocking
            return;
        }
        // Flush ring buffer to free up space
        flush();
    }
}

void AsyncLogger::flush() {
    // Get all entries from async queue
    auto queue_entries = async_queue_->popAll();
    
    // Get all entries from ring buffer
    auto ring_entries = ring_buffer_->popAll();
    
    // Write all entries to file
    for (const auto& entry : queue_entries) {
        std::string formatted = formatLogEntry(entry);
        file_manager_->write(formatted);
    }
    
    for (const auto& entry : ring_entries) {
        std::string formatted = formatLogEntry(entry);
        file_manager_->write(formatted);
    }
    
    // Force file flush
    if (file_manager_) {
        // FileManager handles periodic flushing
    }
}

void AsyncLogger::writerThread() {
    while (running_.load()) {
        // Wait for entries or shutdown request using semaphore
        sem_.acquire();

        // Check for shutdown request
        if (shutdown_requested_.load()) {
            // Process remaining entries before exiting
            flush();
            break;
        }

        // Flush all queued entries
        flush();
    }

    running_.store(false);
}

void AsyncLogger::emergencyFlush() {
    // Emergency flush on crash - write all buffers
    
    // Flush async queue
    auto queue_entries = async_queue_->popAll();
    for (const auto& entry : queue_entries) {
        std::string formatted = formatLogEntry(entry);
        file_manager_->write(formatted);
    }
    
    // Flush ring buffer
    auto ring_entries = ring_buffer_->popAll();
    for (const auto& entry : ring_entries) {
        std::string formatted = formatLogEntry(entry);
        file_manager_->write(formatted);
    }
    
    // Force file flush immediately
    // Note: This requires FileManager interface to support immediate flush
    // For now, FileManager handles periodic flushing
}

ProcessIdType AsyncLogger::getProcessId() const {
#ifdef SPECKIT_PLATFORM_WINDOWS
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}

ThreadIdType AsyncLogger::getThreadId() const {
#ifdef SPECKIT_PLATFORM_WINDOWS
    return GetCurrentThreadId();
#else
    static thread_local std::thread::id tid = std::this_thread::get_id();
    return tid;
#endif
}

int64_t AsyncLogger::getTimestamp() const {
    // Get timestamp in milliseconds since epoch
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}


