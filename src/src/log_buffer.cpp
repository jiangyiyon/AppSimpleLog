// Ring buffer implementation with atomic operations

#include "speckit/log/log_buffer.h"
#include <cstring>
#include <atomic>



RingBuffer::RingBuffer(size_t capacity)
    : head_(0), tail_(0), size_(0), capacity_(capacity) {
    // Pre-allocate buffer to avoid runtime allocation
    buffer_ = std::make_unique<LogEntry[]>(capacity);
}

bool RingBuffer::tryPush(const LogEntry& entry) {
    // Get current head and size atomically
    size_t current_head = head_.load(std::memory_order_acquire);
    size_t current_size = size_.load(std::memory_order_acquire);
    
    // Check if buffer is full
    if (current_size >= capacity_) {
        return false;  // Buffer full, cannot push
    }
    
    // Move entry into buffer at head position
    buffer_[current_head] = std::move(const_cast<LogEntry&>(entry));
    
    // Advance head with wrap-around
    size_t new_head = (current_head + 1) % capacity_;
    head_.store(new_head, std::memory_order_release);
    
    // Increment size
    size_.fetch_add(1, std::memory_order_acq_rel);
    
    return true;
}

std::vector<LogEntry> RingBuffer::popAll() {
    // Get current tail and size atomically
    size_t current_tail = tail_.load(std::memory_order_acquire);
    size_t current_size = size_.load(std::memory_order_acquire);
    
    // Collect all entries from tail to head
    std::vector<LogEntry> entries;
    entries.reserve(current_size);
    
    for (size_t i = 0; i < current_size; ++i) {
        size_t index = (current_tail + i) % capacity_;
        entries.push_back(std::move(buffer_[index]));
    }
    
    // Update tail to head and reset size
    size_t new_tail = (current_tail + current_size) % capacity_;
    tail_.store(new_tail, std::memory_order_release);
    size_.store(0, std::memory_order_release);
    
    return entries;
}

bool RingBuffer::isEmpty() const {
    return size_.load(std::memory_order_acquire) == 0;
}

size_t RingBuffer::size() const {
    return size_.load(std::memory_order_acquire);
}

size_t RingBuffer::capacity() const {
    return capacity_;
}


