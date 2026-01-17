// Async queue implementation with atomic operations

#include "speckit/log/async_queue.h"
#include <algorithm>

AsyncQueue::AsyncQueue(size_t capacity)
    : write_index_(0), read_index_(0), size_(0), capacity_(capacity) {
    // Pre-allocate buffer to avoid runtime allocation
    buffer_ = std::make_unique<LogEntry[]>(capacity);
}

AsyncQueue::~AsyncQueue() = default;

bool AsyncQueue::tryPush(const LogEntry& entry) {
    size_t current_read, current_write;

    do {
        current_read = read_index_.load(std::memory_order_acquire);
        current_write = write_index_.load(std::memory_order_acquire);

        // Calculate size based on current read/write positions
        size_t current_size = (current_write - current_read + capacity_) % capacity_;

        if (current_size >= capacity_ - 1) { // Queue is full
            return false;
        }

        // Calculate new write index
        size_t new_write = (current_write + 1) % capacity_;

        // Attempt to atomically update write index
        if (write_index_.compare_exchange_strong(current_write, new_write,
            std::memory_order_acq_rel, std::memory_order_acquire)) {
            
            // Successfully acquired write position, store the entry
            // Use move assignment since LogEntry copy is deleted
            buffer_[current_write] = std::move(const_cast<LogEntry&>(entry));
            
            // Ensure the write is visible before updating size
            std::atomic_thread_fence(std::memory_order_release);
            
            // Update size atomically
            size_.fetch_add(1, std::memory_order_relaxed);
            
            return true;
        }
        
        // If CAS failed, retry with updated values
    } while (true);
}

std::vector<LogEntry> AsyncQueue::popAll() {
    std::vector<LogEntry> result;
    
    while (true) {
        // Get current read and write indices
        size_t current_read = read_index_.load(std::memory_order_acquire);
        size_t current_write = write_index_.load(std::memory_order_acquire);
        
        // Calculate number of available entries
        size_t available_count = (current_write - current_read + capacity_) % capacity_;
        
        if (available_count == 0) {
            return result; // No entries to pop
        }
        
        // Prepare result vector
        result.clear();
        result.reserve(available_count);
        
        // Copy all available entries
        for (size_t i = 0; i < available_count; ++i) {
            size_t index = (current_read + i) % capacity_;
            // Use move semantics to avoid copying
            result.push_back(std::move(buffer_[index]));
        }
        
        // Attempt to atomically update read index
        if (read_index_.compare_exchange_strong(current_read, current_write,
            std::memory_order_acq_rel, std::memory_order_acquire)) {
            
            // Update size atomically
            size_.fetch_sub(available_count, std::memory_order_relaxed);
            
            // Success - return the entries
            return result;
        }
        
        // If CAS failed, another thread popped entries first, try again
    }
}

bool AsyncQueue::isEmpty() const {
    size_t current_read = read_index_.load(std::memory_order_acquire);
    size_t current_write = write_index_.load(std::memory_order_acquire);
    return current_read == current_write;
}

size_t AsyncQueue::size() const {
    // Use the atomic size_ for consistency with tryPush and popAll
    return size_.load(std::memory_order_acquire);
}

size_t AsyncQueue::capacity() const {
    return capacity_;
}