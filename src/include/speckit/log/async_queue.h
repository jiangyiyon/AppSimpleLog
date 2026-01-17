#pragma once

#include <atomic>
#include <memory>
#include <vector>
#include <optional>
#include <cstddef>
#include "log_entry.h"

/// Lock-free MPSC bounded queue for async logging
/// Multiple producers, single consumer. High-performance, per-slot sequence algorithm.
class AsyncQueue {
public:
    explicit AsyncQueue(size_t capacity);
    ~AsyncQueue();

    // Accept an lvalue reference; the queue will move from the entry only on success.
    // This allows callers to keep the original entry when push fails (fallback path).
    bool tryPush(LogEntry& entry);

    // Also accept rvalue (temporary or moved) for convenience / performance
    bool tryPush(LogEntry&& entry);

    // Disable accidental passing of const lvalues
    bool tryPush(const LogEntry&) = delete;

    // Single-consumer: pop all available entries
    std::vector<LogEntry> popAll();

    bool isEmpty() const;
    size_t size() const;
    size_t capacity() const;

private:
    struct Cell {
        std::atomic<size_t> seq;
        std::optional<LogEntry> storage; // construct in-place when producer writes

        Cell() : seq(0), storage(std::nullopt) {}
    };

    // Use contiguous heap array because Cell is non-copyable/non-movable (contains atomics).
    std::unique_ptr<Cell[]> buffer_;
    const size_t capacity_;

    // Monotonic counters
    std::atomic<size_t> tail_; // producers increment (reservation)
    std::atomic<size_t> head_; // consumer increments (consumption)

    std::atomic<size_t> size_;
};