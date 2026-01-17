// Async queue implementation with atomic operations

#include "speckit/log/async_queue.h"



AsyncQueue::AsyncQueue(size_t capacity)
    : write_index_(0), read_index_(0), size_(0), capacity_(capacity) {
    // Pre-allocate buffer to avoid runtime allocation
    buffer_ = std::make_unique<LogEntry[]>(capacity);
}

AsyncQueue::~AsyncQueue() = default;

bool AsyncQueue::tryPush(const LogEntry& entry) {
    // Get current write index and size atomically
    size_t current_write = write_index_.load(std::memory_order_acquire);
    size_t current_size = size_.load(std::memory_order_acquire);
    
    // Check if queue is full
    if (current_size >= capacity_) {
        return false;  // Queue full, cannot push
    }
    
    // Move entry into buffer at write position
    buffer_[current_write] = std::move(const_cast<LogEntry&>(entry));
    
    // Advance write index with wrap-around
    size_t new_write = (current_write + 1) % capacity_;
    write_index_.store(new_write, std::memory_order_release);
    
    // Increment size
    size_.fetch_add(1, std::memory_order_acq_rel);
    
    return true;
}

std::vector<LogEntry> AsyncQueue::popAll() {
    // Get current read and write indices atomically
    size_t current_read = read_index_.load(std::memory_order_acquire);
    size_t current_write = write_index_.load(std::memory_order_acquire);
    size_t current_size = size_.load(std::memory_order_acquire);
    
    // Collect all entries from read index to write index
    std::vector<LogEntry> entries;
    entries.reserve(current_size);
    
    for (size_t i = 0; i < current_size; ++i) {
        size_t index = (current_read + i) % capacity_;
        entries.push_back(std::move(buffer_[index]));
    }
    
    // Update read index to write index and reset size
    read_index_.store(current_write, std::memory_order_release);
    size_.store(0, std::memory_order_release);
    
    return entries;
}

bool AsyncQueue::isEmpty() const {
    return size_.load(std::memory_order_acquire) == 0;
}

size_t AsyncQueue::size() const {
    return size_.load(std::memory_order_acquire);
}

size_t AsyncQueue::capacity() const {
    return capacity_;
}


