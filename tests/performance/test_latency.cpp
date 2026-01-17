// Performance test for log call latency

#include <gtest/gtest.h>
#include "speckit/log/async_logger.h"
#include "speckit/log/log_level.h"
#include <chrono>
#include <vector>
#include <numeric>
#include <algorithm>
#include <filesystem>

using namespace std::chrono_literals;

class LatencyTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clean up any existing test files
        std::filesystem::remove_all("latency_test*.log");

        // Get absolute path for current directory
        std::filesystem::path current_path = std::filesystem::current_path();
        std::string log_path = (current_path / "latency_test").string();

        // Create async logger with absolute path
        logger_ = AsyncLogger::create(log_path);
        ASSERT_NE(logger_, nullptr);
        logger_->setLogLevel(LogLevel::kLogLevelInfo);
    }
    
    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all("latency_test*.log");
        std::filesystem::remove_all("latency_test*.log.d");  // Remove crash buffer files
    }
    
    std::unique_ptr<AsyncLogger> logger_;
};

TEST_F(LatencyTest, SingleLogCall_LessThan1ms) {
    // Measure single log call latency
    auto start = std::chrono::high_resolution_clock::now();
    logger_->log(LogLevel::kLogLevelInfo, "Test", "Latency test message");
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    // Verify <1ms requirement (1,000,000 nanoseconds)
    EXPECT_LT(duration.count(), 1000000);
    
    std::cout << "Single log call latency: " << duration.count() << " ns" << std::endl;
}

TEST_F(LatencyTest, MultipleLogCalls_AllLessThan1ms) {
    const int NUM_CALLS = 1000;
    std::vector<int64_t> latencies;
    latencies.reserve(NUM_CALLS);
    
    // Measure latency for multiple calls
    for (int i = 0; i < NUM_CALLS; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        logger_->log(LogLevel::kLogLevelInfo, "Test", "Message " + std::to_string(i));
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        latencies.push_back(duration.count());
    }
    
    // Calculate statistics
    int64_t avg = std::accumulate(latencies.begin(), latencies.end(), 0LL) / NUM_CALLS;
    int64_t max = *std::max_element(latencies.begin(), latencies.end());
    int64_t min = *std::min_element(latencies.begin(), latencies.end());
    
    // Verify all calls are <1ms
    bool all_under_1ms = std::all_of(latencies.begin(), latencies.end(),
        [](int64_t latency) { return latency < 1000000; });
    
    EXPECT_TRUE(all_under_1ms) << "Not all log calls were <1ms";
    EXPECT_LT(avg, 100000) << "Average latency should be <100µs";
    
    std::cout << "Log call latency statistics:" << std::endl;
    std::cout << "  Average: " << avg << " ns" << std::endl;
    std::cout << "  Min: " << min << " ns" << std::endl;
    std::cout << "  Max: " << max << " ns" << std::endl;
    std::cout << "  All <1ms: " << (all_under_1ms ? "Yes" : "No") << std::endl;
}

TEST_F(LatencyTest, HighFrequency_Calls_MaintainLowLatency) {
    const int NUM_CALLS = 10000;
    std::vector<int64_t> latencies;
    latencies.reserve(NUM_CALLS);
    
    // Measure latency for high-frequency calls
    for (int i = 0; i < NUM_CALLS; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        logger_->log(LogLevel::kLogLevelInfo, "Test", "Message " + std::to_string(i));
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        latencies.push_back(duration.count());
    }
    
    // Calculate 95th percentile
    std::sort(latencies.begin(), latencies.end());
    size_t p95_index = NUM_CALLS * 95 / 100;
    int64_t p95 = latencies[p95_index];
    
    // Verify 95th percentile is <1ms
    EXPECT_LT(p95, 1000000) << "95th percentile should be <1ms";
    
    std::cout << "High-frequency log call statistics:" << std::endl;
    std::cout << "  Total calls: " << NUM_CALLS << std::endl;
    std::cout << "  95th percentile: " << p95 << " ns" << std::endl;
    std::cout << "  95th percentile <1ms: " << (p95 < 1000000 ? "Yes" : "No") << std::endl;
}

TEST_F(LatencyTest, LogCall_WithDifferentLevels_AllFast) {
    std::vector<int64_t> latencies;
    std::vector<LogLevel> levels = {LogLevel::kLogLevelDebug, LogLevel::kLogLevelInfo, LogLevel::kLogLevelWarning, LogLevel::kLogLevelError};
    
    // Measure latency for different log levels
    for (int i = 0; i < 1000; ++i) {
        LogLevel level = levels[i % levels.size()];
        
        auto start = std::chrono::high_resolution_clock::now();
        logger_->log(level, "Test", "Message " + std::to_string(i));
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        latencies.push_back(duration.count());
    }
    
    // Calculate average
    int64_t avg = std::accumulate(latencies.begin(), latencies.end(), 0LL) / latencies.size();
    
    // Verify all levels are fast
    EXPECT_LT(avg, 100000) << "Average latency should be <100µs";
    
    std::cout << "Log call latency by level:" << std::endl;
    std::cout << "  Average: " << avg << " ns" << std::endl;
}
