#include "speckit/log/async_queue.h"
#include <cassert>
#include <thread>

AsyncQueue::AsyncQueue(size_t capacity)
    : buffer_(nullptr), capacity_(capacity), tail_(0), head_(0), size_(0) {
    assert(capacity_ > 0);
    buffer_ = std::make_unique<Cell[]>(capacity_);

    // Initialize per-slot sequence numbers to their index
    for (size_t i = 0; i < capacity_; ++i) {
        buffer_[i].seq.store(i, std::memory_order_relaxed);
    }
}

AsyncQueue::~AsyncQueue() {
    // Defensive cleanup: destroy any remaining storage
    size_t head = head_.load(std::memory_order_relaxed);
    size_t tail = tail_.load(std::memory_order_relaxed);
    size_t available = (tail >= head) ? (tail - head) : 0;

    for (size_t i = 0; i < available; ++i) {
        size_t pos = (head + i) % capacity_;
        if (buffer_[pos].storage.has_value()) {
            buffer_[pos].storage.reset();
        }
    }
}

bool AsyncQueue::tryPush(LogEntry& entry) {
    const size_t cap = capacity_;

    while (true) {
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t current_head = head_.load(std::memory_order_acquire);

        // Detect full: reserved but not yet consumed count >= cap
        if (current_tail - current_head >= cap) {
            return false; // queue full
        }

        // Try to reserve a ticket (slot index = current_tail)
        if (tail_.compare_exchange_weak(current_tail, current_tail + 1,
            std::memory_order_acq_rel,
            std::memory_order_relaxed)) {
            size_t pos = current_tail % cap;
            Cell& cell = buffer_[pos];

            // Wait until cell.seq == current_tail (slot free for this ticket)
            while (cell.seq.load(std::memory_order_acquire) != current_tail) {
                // polite spin
                std::this_thread::yield();
            }

            // Construct entry in-place by moving from caller's entry
            cell.storage.emplace(std::move(entry));

            // Publish slot as ready: seq = current_tail + 1
            cell.seq.store(current_tail + 1, std::memory_order_release);

            // Update size counter
            size_.fetch_add(1, std::memory_order_acq_rel);

            return true;
        }
        // CAS failed; retry (another producer won the ticket)
    }
}

bool AsyncQueue::tryPush(LogEntry&& entry) {
    // Forward to lvalue overload (treat entry as lvalue here)
    return tryPush(static_cast<LogEntry&>(entry));
}

std::vector<LogEntry> AsyncQueue::popAll() {
    std::vector<LogEntry> result;
    const size_t cap = capacity_;

    size_t head = head_.load(std::memory_order_relaxed);
    size_t tail = tail_.load(std::memory_order_acquire);

    size_t available = (tail >= head) ? (tail - head) : 0;
    if (available == 0) {
        return result;
    }

    result.reserve(available);

    for (size_t i = 0; i < available; ++i) {
        size_t idx = (head + i) % cap;
        Cell& cell = buffer_[idx];

        // Wait until producer published slot: seq == head + i + 1
        size_t expected = head + i + 1;
        while (cell.seq.load(std::memory_order_acquire) != expected) {
            std::this_thread::yield();
        }

        // Move entry out if present
        if (cell.storage.has_value()) {
            result.push_back(std::move(*cell.storage));
            cell.storage.reset();
        }

        // Mark slot free for future producers: set seq = head + i + cap
        cell.seq.store(head + i + cap, std::memory_order_release);
    }

    // Advance head and update size
    head_.store(head + available, std::memory_order_release);
    size_.fetch_sub(available, std::memory_order_acq_rel);

    return result;
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