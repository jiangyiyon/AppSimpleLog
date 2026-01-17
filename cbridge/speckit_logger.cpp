/*
Implementation notes (inline):
- tryPush(LogEntry&):
    1) Check for full: if tail_ - head_ >= capacity_ => full.
    2) Reserve ticket by CAS on tail_ (compare_exchange_weak).
    3) Compute slot index = ticket % capacity_.
    4) Spin until cell.seq == ticket (slot free for this ticket).
    5) emplace/move entry into storage.
    6) Publish by setting cell.seq = ticket + 1 (release store).
    7) Update size_ and return true.
- tryPush(LogEntry&&) forwards to the lvalue overload so callers can pass temporaries.
- popAll:
    - Computes available = tail_ - head_, loops through each slot,
      waits for seq == head + i + 1 (acquire), moves out storage, resets optional,
      then sets seq = head + i + capacity_ (release) to mark slot reusable.
- Memory ordering:
    - Producer publishes with release, consumer observes with acquire to form happens-before.
*/

#include "speckit/log/async_queue.h"
#include <cassert>
#include <thread>

AsyncQueue::AsyncQueue(size_t capacity)
    : buffer_(nullptr), capacity_(capacity), tail_(0), head_(0), size_(0) {
    assert(capacity_ > 0);
    buffer_ = std::make_unique<Cell[]>(capacity_);

    // Initialize per-slot sequence numbers to their index.
    // seq == index means slot is free for ticket == index.
    for (size_t i = 0; i < capacity_; ++i) {
        buffer_[i].seq.store(i, std::memory_order_relaxed);
    }
}

AsyncQueue::~AsyncQueue() {
    // Defensive cleanup: destroy any remaining storage (consumer should normally drain).
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
        // Fast path check (optimistic)
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t current_head = head_.load(std::memory_order_acquire);

        // If the number of reserved tickets (tail - head) is >= capacity, queue is full.
        if (current_tail - current_head >= cap) {
            return false; // full
        }

        // Reserve a ticket using CAS. Using compare_exchange_weak in a loop is typical.
        if (tail_.compare_exchange_weak(current_tail, current_tail + 1,
            std::memory_order_acq_rel,
            std::memory_order_relaxed)) {
            // We own ticket == current_tail
            size_t pos = current_tail % cap;
            Cell& cell = buffer_[pos];

            // Wait until the slot's sequence equals our ticket, indicating it is free.
            // This avoids overwriting data that hasn't been consumed yet.
            while (cell.seq.load(std::memory_order_acquire) != current_tail) {
                // polite spin; tune for your workload (busy-spin/pause/backoff)
                std::this_thread::yield();
            }

            // Construct/move the log entry into the cell's storage.
            cell.storage.emplace(std::move(entry));

            // Publish the slot as ready to read. Release ensures prior writes (storage)
            // are visible to a consumer that performs an acquire load on seq.
            cell.seq.store(current_tail + 1, std::memory_order_release);

            // Update size for external queries (optional optimization: derive from tail-head).
            size_.fetch_add(1, std::memory_order_acq_rel);

            return true;
        }

        // CAS failed: another producer reserved that ticket. Retry.
    }
}

bool AsyncQueue::tryPush(LogEntry&& entry) {
    // Forward rvalue to lvalue overload ¡ª treat local parameter as an lvalue to move from.
    return tryPush(static_cast<LogEntry&>(entry));
}

std::vector<LogEntry> AsyncQueue::popAll() {
    std::vector<LogEntry> result;
    const size_t cap = capacity_;

    // Sample head and tail to determine how many entries are available.
    size_t head = head_.load(std::memory_order_relaxed);
    size_t tail = tail_.load(std::memory_order_acquire);

    size_t available = (tail >= head) ? (tail - head) : 0;
    if (available == 0) {
        return result;
    }

    result.reserve(available);

    // For each available ticket, wait until the slot is published (seq == ticket+1),
    // then move the entry out and mark the slot reusable.
    for (size_t i = 0; i < available; ++i) {
        size_t idx = (head + i) % cap;
        Cell& cell = buffer_[idx];

        // Wait until producer has published this ticket. The acquire load synchronizes
        // with the producer's release store on seq, ensuring the storage contents are visible.
        size_t expected = head + i + 1;
        while (cell.seq.load(std::memory_order_acquire) != expected) {
            std::this_thread::yield();
        }

        // Move the entry out if present. Optional guards against unexpected states.
        if (cell.storage.has_value()) {
            result.push_back(std::move(*cell.storage));
            cell.storage.reset();
        }

        // Mark the slot free for future producers by advancing seq to a value that
        // represents the next cycle where this slot becomes available.
        cell.seq.store(head + i + cap, std::memory_order_release);
    }

    // Advance head and adjust size atomically.
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