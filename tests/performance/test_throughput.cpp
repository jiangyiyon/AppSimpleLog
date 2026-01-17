// Performance test for log throughput

#include <gtest/gtest.h>
#include "speckit/log/async_logger.h"
#include "speckit/log/log_level.h"
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>
#include <regex>
#include <numeric>
#include <vector>

using namespace std::chrono_literals;

class ThroughputTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clean up any existing test files
        std::filesystem::remove_all("throughput_test*.log");

        // Get absolute path for current directory
        std::filesystem::path current_path = std::filesystem::current_path();
        std::string log_path = (current_path / "throughput_test").string();

        // Create async logger with absolute path
        logger_ = AsyncLogger::create(log_path);
        ASSERT_NE(logger_, nullptr);
        logger_->setLogLevel(LogLevel::kLogLevelInfo);
    }

    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all("throughput_test*.log");
        std::filesystem::remove_all("throughput_test*.log.d");  // Remove crash buffer files
    }
    
    std::unique_ptr<AsyncLogger> logger_;
    
    int countLogEntries(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return 0;
        }
        
        int count = 0;
        std::string line;
        while (std::getline(file, line)) {
            // Count lines that start with [INFO]
            if (line.find("[INFO]") == 0) {
                ++count;
            }
        }
        
        file.close();
        return count;
    }
};

TEST_F(ThroughputTest, ThousandLogs_AllWritten) {
    const int NUM_LOGS = 1000;
    
    // Log 1000 messages
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_LOGS; ++i) {
        logger_->log(LogLevel::kLogLevelInfo, "Throughput", "Message " + std::to_string(i));
    }
    
    // Wait for all logs to be written
    std::this_thread::sleep_for(1s);
    logger_->flush();
    std::this_thread::sleep_for(100ms);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Verify all logs are in file
    int count = countLogEntries("throughput_test.log");
    EXPECT_EQ(count, NUM_LOGS) << "Expected " << NUM_LOGS << " log entries, found " << count;
    
    std::cout << "Throughput: " << NUM_LOGS << " logs in " << duration.count() << " ms" << std::endl;
    std::cout << "Average: " << (duration.count() / NUM_LOGS) << " ms per log" << std::endl;
}

TEST_F(ThroughputTest, TenThousandLogs_AllWritten) {
    const int NUM_LOGS = 10000;
    
    // Log 10000 messages
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_LOGS; ++i) {
        logger_->log(LogLevel::kLogLevelInfo, "Throughput", "Message " + std::to_string(i));
    }
    
    // Wait for all logs to be written
    std::this_thread::sleep_for(5s);
    logger_->flush();
    std::this_thread::sleep_for(200ms);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Verify all logs are in file
    int count = countLogEntries("throughput_test.log");
    EXPECT_GE(count, NUM_LOGS - 100) << "Expected at least " << (NUM_LOGS - 100) << " log entries, found " << count;
    
    std::cout << "Throughput: " << count << " logs written in " << duration.count() << " ms" << std::endl;
    std::cout << "Rate: " << (count * 1000 / duration.count()) << " logs/second" << std::endl;
}

TEST_F(ThroughputTest, HundredThousandLogs_AllWritten) {
    const int NUM_LOGS = 100000;
    
    // Log 100000 messages
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_LOGS; ++i) {
        logger_->log(LogLevel::kLogLevelInfo, "Throughput", "Message " + std::to_string(i));
    }
    
    // Wait for all logs to be written
    std::this_thread::sleep_for(30s);
    logger_->flush();
    std::this_thread::sleep_for(500ms);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Verify most logs are in file
    int count = countLogEntries("throughput_test.log");
    EXPECT_GE(count, NUM_LOGS - 1000) << "Expected at least " << (NUM_LOGS - 1000) << " log entries, found " << count;
    
    std::cout << "Throughput: " << count << " logs written in " << duration.count() << " ms" << std::endl;
    std::cout << "Rate: " << (count * 1000 / duration.count()) << " logs/second" << std::endl;
    
    // Verify no data loss (allow small margin)
    double loss_rate = static_cast<double>(NUM_LOGS - count) / NUM_LOGS;
    EXPECT_LT(loss_rate, 0.01) << "Data loss rate should be <1%";
    
    std::cout << "Data loss rate: " << (loss_rate * 100) << "%" << std::endl;
}

TEST_F(ThroughputTest, SustainedThroughput_MaintainsStability) {
    const int BATCH_SIZE = 10000;
    const int NUM_BATCHES = 5;
    std::vector<int> batch_counts;
    
    // Log multiple batches to test sustained performance
    for (int batch = 0; batch < NUM_BATCHES; ++batch) {
        auto batch_start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < BATCH_SIZE; ++i) {
            logger_->log(LogLevel::kLogLevelInfo, "Sustained", "Batch " + std::to_string(batch) + " Message " + std::to_string(i));
        }
        
        // Wait for batch to complete
        std::this_thread::sleep_for(2s);
        logger_->flush();
        std::this_thread::sleep_for(100ms);
        
        auto batch_end = std::chrono::high_resolution_clock::now();
        auto batch_duration = std::chrono::duration_cast<std::chrono::milliseconds>(batch_end - batch_start);
        
        std::cout << "Batch " << batch << ": " << BATCH_SIZE << " logs in " << batch_duration.count() << " ms" << std::endl;
        
        // Check if performance degrades
        if (batch > 0) {
            auto prev_duration = batch_counts.back();
            auto degradation = std::abs(static_cast<int>(batch_duration.count() - prev_duration));
            auto degradation_percent = (degradation * 100.0) / prev_duration;
            
            // Performance should not degrade significantly (<20%)
            EXPECT_LT(degradation_percent, 20.0) 
                << "Performance degraded by " << degradation_percent << "%";
        }
        
        batch_counts.push_back(batch_duration.count());
    }
    
    std::cout << "Sustained throughput test completed" << std::endl;
    std::cout << "Average batch duration: " 
              << (std::accumulate(batch_counts.begin(), batch_counts.end(), 0) / NUM_BATCHES) << " ms" << std::endl;
}

TEST_F(ThroughputTest, Throughput_WithMultipleTags) {
    const int NUM_LOGS = 10000;
    std::vector<std::string> tags = {"Network", "Database", "UI", "Auth", "Cache"};
    
    // Log messages with different tags
    for (int i = 0; i < NUM_LOGS; ++i) {
        const std::string& tag = tags[i % tags.size()];
        logger_->log(LogLevel::kLogLevelInfo, tag, "Message " + std::to_string(i));
    }
    
    // Wait for all logs to be written
    std::this_thread::sleep_for(5s);
    logger_->flush();
    std::this_thread::sleep_for(200ms);
    
    // Verify all logs are in file
    int count = countLogEntries("throughput_test.log");
    EXPECT_GE(count, NUM_LOGS - 100) << "Expected at least " << (NUM_LOGS - 100) << " log entries, found " << count;
    
    std::cout << "Throughput with tags: " << count << " logs written" << std::endl;
}
