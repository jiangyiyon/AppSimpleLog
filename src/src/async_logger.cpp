// Async logger implementation with background thread

#include "speckit/log/async_logger.h"
#include "speckit/log/log_level.h"
#include "speckit/log/log_entry.h"
#include <chrono>
#include <format>
#include <stop_token>
#include <thread>



std::unique_ptr<AsyncLogger> AsyncLogger::create(const std::string& base_name,
                                                  size_t queue_size) {
    auto logger = std::unique_ptr<AsyncLogger>(new AsyncLogger());
    if (!logger->initialize(base_name, queue_size)) {
        return nullptr;
    }
    return logger;
}

bool AsyncLogger::initializeComponents(const std::string& base_name, size_t queue_size) {
    // Create components
    file_manager_ = std::make_unique<FileManager>(base_name);
    async_queue_ = std::make_unique<AsyncQueue>(queue_size);
    ring_buffer_ = std::make_unique<RingBuffer>(queue_size);  // Same size for crash safety
    crash_handler_ = std::make_unique<CrashHandler>();

    // Initialize file manager
    if (!file_manager_->Initialize(process_id_)) {
        return false;
    }

    // Set crash handler callback
    crash_handler_->setFlushCallback([this]() { this->emergencyFlush(); });

    // Start background writer thread with stop token support
    writer_thread_ = std::jthread([this](std::stop_token st) { this->writerThread(st); });

    return true;
}

void AsyncLogger::writeEntries(const std::vector<LogEntry>& entries) {
    if (!file_manager_) return;
    for (const auto& entry : entries) {
        std::string formatted = formatLogEntry(entry);
        file_manager_->Write(formatted);
    }
}

AsyncLogger::AsyncLogger()
    : process_id_(getProcessId()), queue_size_(0), sem_(0) {
    // Default ctor. Call initialize(...) to set up components.
}

bool AsyncLogger::initialize(const std::string& base_name, size_t queue_size) {
    if (initialized_) return true;
    queue_size_ = queue_size;
    if (!initializeComponents(base_name, queue_size)) {
        initialized_ = false;
        return false;
    }
    initialized_ = true;
    return true;
}

void AsyncLogger::deinitialize() {
    if (!initialized_) return;

    // Request thread stop and wake it up
    writer_thread_.request_stop();
    sem_.release();  // Signal writer thread to wake up

    if (writer_thread_.joinable()) {
        writer_thread_.join();
    }

    // Flush any remaining entries
    flush();

    initialized_ = false;
}

AsyncLogger::~AsyncLogger() {
    // Ensure resources are released
    deinitialize();
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
    if (async_queue_->tryPush(std::move(entry))) {
        // Success - notify writer thread
        sem_.release();
    } else {
        // Queue full - try ring buffer for crash safety
        if (!ring_buffer_->tryPush(std::move(entry))) {
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
    writeEntries(queue_entries);
    writeEntries(ring_entries);
    
    // Force file flush
    if (file_manager_) {
        // FileManager handles periodic flushing
    }
}

void AsyncLogger::writerThread(std::stop_token stop_token) {
    while (!stop_token.stop_requested()) {
        // Wait for entries or stop request using semaphore
        sem_.acquire();

        if (stop_token.stop_requested()) {
            // Process remaining entries before exiting
            flush();
            break;
        }

        // Flush all queued entries
        flush();
    }
}

void AsyncLogger::emergencyFlush() {
    // Emergency flush on crash - write all buffers
    
    // Flush async queue
    auto queue_entries = async_queue_->popAll();
    writeEntries(queue_entries);
    
    // Flush ring buffer
    auto ring_entries = ring_buffer_->popAll();
    writeEntries(ring_entries);
    
    // Force file flush immediately
    // Note: This requires FileManager interface to support immediate flush
    // For now, FileManager handles periodic flushing
    if (file_manager_) {
        file_manager_->Flush();
    }
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


