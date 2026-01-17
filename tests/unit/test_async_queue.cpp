// Unit tests for AsyncQueue

#include <gtest/gtest.h>
#include "speckit/log/async_queue.h"
#include "speckit/log/log_level.h"



class AsyncQueueTest : public ::testing::Test {
protected:
    static constexpr size_t TEST_CAPACITY = 1000;
    
    LogEntry createTestEntry(LogLevel level, const std::string& tag, 
                         const std::string& message) {
        return LogEntry(level, 12345, 1234, 5678, tag, message);
    }
};

TEST_F(AsyncQueueTest, Constructor_InitializesEmpty) {
    AsyncQueue queue(TEST_CAPACITY);
    
    EXPECT_TRUE(queue.isEmpty());
    EXPECT_EQ(queue.size(), 0);
    EXPECT_EQ(queue.capacity(), TEST_CAPACITY);
}

TEST_F(AsyncQueueTest, TryPush_IncreasesSize) {
    AsyncQueue queue(TEST_CAPACITY);
    auto entry = createTestEntry(LogLevel::kLogLevelInfo, "Test", "Message");

    EXPECT_TRUE(queue.tryPush(entry));
    EXPECT_FALSE(queue.isEmpty());
    EXPECT_EQ(queue.size(), 1);
}

TEST_F(AsyncQueueTest, TryPush_UntilFull) {
    AsyncQueue queue(TEST_CAPACITY);

    // Fill queue to capacity
    for (size_t i = 0; i < TEST_CAPACITY; ++i) {
        auto entry = createTestEntry(LogLevel::kLogLevelInfo, "Test", "Message " + std::to_string(i));
        EXPECT_TRUE(queue.tryPush(entry)) << "Push failed at index " << i;
    }

    EXPECT_EQ(queue.size(), TEST_CAPACITY);

    // Next push should fail
    auto extra_entry = createTestEntry(LogLevel::kLogLevelInfo, "Test", "Extra");
    EXPECT_FALSE(queue.tryPush(extra_entry));
    EXPECT_EQ(queue.size(), TEST_CAPACITY);
}

TEST_F(AsyncQueueTest, PopAll_ReturnsAllEntries) {
    AsyncQueue queue(TEST_CAPACITY);

    // Add some entries
    auto entry1 = createTestEntry(LogLevel::kLogLevelInfo, "Test1", "Message 1");
    auto entry2 = createTestEntry(LogLevel::kLogLevelWarning, "Test2", "Message 2");
    auto entry3 = createTestEntry(LogLevel::kLogLevelError, "Test3", "Message 3");

    queue.tryPush(entry1);
    queue.tryPush(entry2);
    queue.tryPush(entry3);
    
    // Pop all entries
    auto entries = queue.popAll();
    
    EXPECT_EQ(entries.size(), 3);
    EXPECT_TRUE(queue.isEmpty());
    EXPECT_EQ(queue.size(), 0);
    
    // Verify entries
    EXPECT_EQ(entries[0].tag, "Test1");
    EXPECT_EQ(entries[1].tag, "Test2");
    EXPECT_EQ(entries[2].tag, "Test3");
}

TEST_F(AsyncQueueTest, PopAll_ClearsQueue) {
    AsyncQueue queue(TEST_CAPACITY);

    // Add entries
    for (size_t i = 0; i < 10; ++i) {
        auto entry = createTestEntry(LogLevel::kLogLevelInfo, "Test", "Message " + std::to_string(i));
        queue.tryPush(entry);
    }
    
    EXPECT_EQ(queue.size(), 10);
    
    // Pop all
    auto entries = queue.popAll();
    EXPECT_EQ(entries.size(), 10);
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(AsyncQueueTest, WrapAround_WorksCorrectly) {
    AsyncQueue queue(10);  // Small capacity for wrap-around testing

    // Fill queue to capacity
    for (size_t i = 0; i < 10; ++i) {
        auto entry = createTestEntry(LogLevel::kLogLevelInfo, "Test", "Message " + std::to_string(i));
        queue.tryPush(entry);
    }

    EXPECT_EQ(queue.size(), 10);

    // Pop half the entries
    auto entries1 = queue.popAll();
    EXPECT_EQ(entries1.size(), 10);  // All entries

    EXPECT_TRUE(queue.isEmpty());

    // Push new entries (should wrap around)
    for (size_t i = 0; i < 5; ++i) {
        auto entry = createTestEntry(LogLevel::kLogLevelInfo, "Test", "New " + std::to_string(i));
        queue.tryPush(entry);
    }
    
    EXPECT_EQ(queue.size(), 5);
    
    // Pop and verify
    auto entries2 = queue.popAll();
    EXPECT_EQ(entries2.size(), 5);
    EXPECT_TRUE(queue.isEmpty());
}

TEST_F(AsyncQueueTest, ConcurrentPush_ThreadSafe) {
    AsyncQueue queue(TEST_CAPACITY);

    const int NUM_THREADS = 4;
    const int ENTRIES_PER_THREAD = 100;
    std::vector<std::thread> threads;

    // Launch multiple threads pushing to queue
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([this, t, &queue]() {
            for (int i = 0; i < ENTRIES_PER_THREAD; ++i) {
                auto entry = createTestEntry(LogLevel::kLogLevelInfo, "Thread" + std::to_string(t),
                                       "Message " + std::to_string(i));
                queue.tryPush(entry);
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify queue has all entries (or full)
    size_t expected_size = std::min<size_t>(NUM_THREADS * ENTRIES_PER_THREAD, TEST_CAPACITY);
    EXPECT_EQ(queue.size(), expected_size);
}

TEST_F(AsyncQueueTest, EmptyQueue_PopReturnsEmptyVector) {
    AsyncQueue queue(TEST_CAPACITY);
    
    auto entries = queue.popAll();
    
    EXPECT_TRUE(entries.empty());
    EXPECT_TRUE(queue.isEmpty());
}
