// Ring buffer implementation with atomic operations

#include "speckit/log/log_buffer.h"
#include <cstring>
#include <atomic>



RingBuffer::RingBuffer(size_t capacity)
    : head_(0), tail_(0), size_(0), capacity_(capacity) {
    // Pre-allocate buffer to avoid runtime allocation
    buffer_ = std::make_unique<LogEntry[]>(capacity);
}

bool RingBuffer::tryPush(LogEntry&& entry) {
    // Load current values atomically
    size_t current_size = size_.load(std::memory_order_acquire);

    // Check if buffer is full
    if (current_size >= capacity_) {
        return false;  // Buffer full, cannot push
    }

    // Use fetch_add to safely increment size
    size_t old_size = size_.fetch_add(1, std::memory_order_acq_rel);

    // Double-check that we didn't exceed capacity after incrementing
    if (old_size >= capacity_) {
        // If we exceeded capacity, decrement size back
        size_.fetch_sub(1, std::memory_order_acq_rel);
        return false;
    }

    // Now safely get and advance head
    size_t current_head = head_.fetch_add(1, std::memory_order_acq_rel);
    size_t index = current_head % capacity_;

    // Move entry to buffer at calculated index
    buffer_[index] = std::move(entry);

    return true;
}

std::vector<LogEntry> RingBuffer::popAll() {
    // Atomically swap size to 0 and get old value
    size_t old_size = size_.exchange(0, std::memory_order_acq_rel);

    if (old_size == 0) {
        return std::vector<LogEntry>();  // Return empty vector if no entries
    }

    // Get current tail and calculate new tail
    size_t current_tail = tail_.load(std::memory_order_acquire);
    size_t new_tail = (current_tail + old_size) % capacity_;

    // Collect all entries from tail to head
    std::vector<LogEntry> entries;
    entries.reserve(old_size);

    for (size_t i = 0; i < old_size; ++i) {
        size_t index = (current_tail + i) % capacity_;
        entries.emplace_back(std::move(buffer_[index]));
    }

    // Update tail atomically
    tail_.store(new_tail, std::memory_order_release);

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