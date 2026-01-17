/*
AsyncQueue ¡ª documentation & usage notes (one-page)

Overview
- This file implements a bounded lock-free MPSC (Multiple-Producers, Single-Consumer)
  queue based on a per-slot sequence (ticket) algorithm.
- Intended use: asynchronous logging where many threads produce log entries and a single
  background writer thread consumes and flushes them to disk.

Design summary (high-level)
- Two monotonic counters:
    - tail_ (producer ticket allocator) ¡ª producers atomically increment to reserve a ticket.
    - head_ (consumer ticket pointer) ¡ª consumer advances as it consumes tickets.
- Fixed-size ring of slots (cells). Each Cell contains:
    - seq (atomic<size_t>): a sequence number encoding the slot state relative to tickets.
    - storage (optional<LogEntry>): in-place storage for the log entry object.
- Producers:
    1. Atomically reserve a ticket = tail_++ (CAS loop).
    2. Compute slot index = ticket % capacity.
    3. Spin until cell.seq == ticket (slot is free for this ticket).
    4. Emplace/move LogEntry into cell.storage.
    5. Publish by setting cell.seq = ticket + 1 (release store).
- Consumer:
    1. Read head and tail, determine available = tail - head.
    2. For each ticket in range [head, tail):
         - idx = ticket % capacity
         - Spin until cell.seq == ticket + 1 (acquire load).
         - Move out storage and reset optional.
         - Mark slot free by setting cell.seq = ticket + capacity (release store).
    3. Advance head_ and update size_.

Why it is safe (no read-of-partial-write)
- Producers do data writes (emplace) before publishing seq (release store).
- Consumer uses acquire loads on seq; seeing ticket+1 implies it also happens-after producer's writes.
- Thus consumer never observes a seq indicating readiness without the associated data being fully written.

Memory ordering rules used
- tail_ CAS uses memory_order_acq_rel on success to pair with consumer store of head_ (release).
- Producers use cell.seq.load(memory_order_acquire) to observe prior state and
  cell.seq.store(..., memory_order_release) to publish.
- Consumers use cell.seq.load(memory_order_acquire) to observe published entries and
  cell.seq.store(..., memory_order_release) to mark slot reusable.
- size_ updates use fetch_add / fetch_sub with acq_rel to make size() queries consistent.

Interface rules & choices
- tryPush(LogEntry& entry):
    - Accepts an lvalue reference; only moves from entry if push succeeds.
    - Advantage: caller keeps ownership of entry when push fails (fallback to ring buffer).
- tryPush(LogEntry&& entry):
    - Convenience overload that forwards to the lvalue overload (moves on success).
- popAll():
    - Single-consumer method which drains all available entries and returns them in a vector.
- capacity():
    - Fixed capacity set in constructor; queue is non-resizing.

Important implementation notes / pitfalls
- Cell must not be copyable/movable because it contains std::atomic members.
  The implementation stores Cell objects in a contiguous heap allocation (unique_ptr<Cell[]>)
  rather than std::vector<Cell> to avoid vector resizing/moving issues.
- Ticket counters (tail_, head_) are monotonic (size_t). On long-running systems with
  extreme throughput, consider 64-bit counters to avoid wrap-around. In practical apps
  64-bit size_t is sufficient.
- The spin-wait uses std::this_thread::yield() for politeness. For best performance/tuning:
    - Use short busy-spin loops + platform-specific pause (e.g. _mm_pause) before yielding.
    - Consider exponential backoff under heavy contention to reduce CPU usage.
- LogEntry move-constructor should ideally be noexcept. If emplace throws, tail_ has already
  been incremented (ticket was reserved). This implementation does not roll back tail_; if this
  is a concern, special handling is required (rare in practice for simple log entry moves).
- size_ is kept for convenience. Alternatively, callers can compute size via tail_ - head_.

Testing & validation recommendations
- Unit tests:
    - Concurrent producers pushing while single consumer popping (ConcurrentPush test).
    - Fill-to-capacity and wrap-around behavior (TryPush_UntilFull, WrapAround).
    - popAll correctness (PopAll_ReturnsAllEntries).
- Tools:
    - Run ThreadSanitizer (where available) or MSVC concurrency/analysis tools.
    - Add stress tests under high thread counts and measure CPU/latency to tune backoff.
- Integration:
    - Verify AsyncLogger fallback ring buffer path still works when tryPush returns false.

Credit & references
- This implementation follows the Vyukov-style bounded MPSC/MPMC ring queue patterns
  (per-slot sequence numbers and ticket allocation).
*/
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