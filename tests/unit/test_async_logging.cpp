// Unit tests for async logging

#include <gtest/gtest.h>
#include "speckit/log/async_logger.h"
#include "speckit/log/log_level.h"
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

class AsyncLoggingTest : public ::testing::Test {
protected:
    std::string log_base_path_;
    
    void SetUp() override {
        // Get absolute path for current directory
        std::filesystem::path current_path = std::filesystem::current_path();
        log_base_path_ = (current_path / "async_test").string();

        // Clean up any existing test files
        std::filesystem::remove_all(log_base_path_ + "*.log");
    }
    
    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all(log_base_path_ + "*.log");
    }
};

TEST_F(AsyncLoggingTest, Create_InitializesAsyncLogger) {
    // Create async logger
    auto logger = AsyncLogger::create(log_base_path_);
    ASSERT_NE(logger, nullptr);

    // Verify log level
    EXPECT_EQ(logger->getLogLevel(), LogLevel::kLogLevelDebug);
}

TEST_F(AsyncLoggingTest, SetAndGetLogLevel) {
    // Create async logger
    auto logger = AsyncLogger::create(log_base_path_);
    ASSERT_NE(logger, nullptr);

    // Set log level to INFO
    logger->setLogLevel(LogLevel::kLogLevelInfo);

    // Verify log level was set
    EXPECT_EQ(logger->getLogLevel(), LogLevel::kLogLevelInfo);
}

TEST_F(AsyncLoggingTest, Log_WritesToBackgroundThread) {
    // Create async logger
    auto logger = AsyncLogger::create(log_base_path_);
    ASSERT_NE(logger, nullptr);

    // Set log level to INFO
    logger->setLogLevel(LogLevel::kLogLevelInfo);

    // Log a message (should be non-blocking)
    auto start = std::chrono::high_resolution_clock::now();
    logger->log(LogLevel::kLogLevelInfo, "Test", "Async log message");
    auto end = std::chrono::high_resolution_clock::now();
    
    // Verify log call is fast (<1ms requirement)
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    EXPECT_LT(duration.count(), 1000);  // <1ms = 1000 microseconds
    
    // Wait for background thread to write
    std::this_thread::sleep_for(100ms);
    
    // Verify message is in file
    std::string log_file = log_base_path_ + ".log";
    std::ifstream file(log_file);
    ASSERT_TRUE(file.is_open());
    
    std::string content((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    file.close();
    
    EXPECT_TRUE(content.find("Async log message") != std::string::npos);
}

TEST_F(AsyncLoggingTest, MultipleLogs_AreProcessed) {
    // Create async logger
    auto logger = AsyncLogger::create(log_base_path_);
    ASSERT_NE(logger, nullptr);

    // Set log level to INFO
    logger->setLogLevel(LogLevel::kLogLevelInfo);

    // Log multiple messages rapidly
    for (int i = 0; i < 100; ++i) {
        logger->log(LogLevel::kLogLevelInfo, "Test", "Message " + std::to_string(i));
    }
    
    // Wait for background thread to process
    std::this_thread::sleep_for(500ms);
    
    // Verify messages are in file
    std::string log_file = log_base_path_ + ".log";
    std::ifstream file(log_file);
    ASSERT_TRUE(file.is_open());
    
    std::string content((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    file.close();
    
    // Count occurrences of "Message" (should be close to 100)
    size_t count = 0;
    size_t pos = 0;
    while ((pos = content.find("Message", pos)) != std::string::npos) {
        ++count;
        pos += 7;  // Length of "Message"
    }
    
    EXPECT_GE(count, 90);  // Allow some margin for processing time
}

TEST_F(AsyncLoggingTest, ShouldLog_RespectsFiltering) {
    // Create async logger
    auto logger = AsyncLogger::create(log_base_path_);
    ASSERT_NE(logger, nullptr);

    // Set minimum level to WARNING
    logger->setLogLevel(LogLevel::kLogLevelWarning);

    // Log DEBUG message (should be filtered)
    logger->log(LogLevel::kLogLevelDebug, "Test", "Debug message");

    // Log INFO message (should be filtered)
    logger->log(LogLevel::kLogLevelInfo, "Test", "Info message");

    // Log WARNING message (should be written)
    logger->log(LogLevel::kLogLevelWarning, "Test", "Warning message");
    
    // Wait for processing
    std::this_thread::sleep_for(100ms);
    
    // Verify only WARNING message is in file
    std::string log_file = log_base_path_ + ".log";
    std::ifstream file(log_file);
    ASSERT_TRUE(file.is_open());
    
    std::string content((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    file.close();
    
    EXPECT_FALSE(content.find("Debug message") != std::string::npos);
    EXPECT_FALSE(content.find("Info message") != std::string::npos);
    EXPECT_TRUE(content.find("Warning message") != std::string::npos);
}

TEST_F(AsyncLoggingTest, Flush_WritesAllPending) {
    // Create async logger
    auto logger = AsyncLogger::create(log_base_path_);
    ASSERT_NE(logger, nullptr);

    // Set log level to INFO
    logger->setLogLevel(LogLevel::kLogLevelInfo);

    // Log multiple messages
    for (int i = 0; i < 50; ++i) {
        logger->log(LogLevel::kLogLevelInfo, "Test", "Message " + std::to_string(i));
    }
    
    // Call flush
    logger->flush();
    
    // Wait a bit for file operations
    std::this_thread::sleep_for(100ms);
    
    // Verify messages are in file
    std::string log_file = log_base_path_ + ".log";
    std::ifstream file(log_file);
    ASSERT_TRUE(file.is_open());
    
    std::string content((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    file.close();
    
    // Count messages
    size_t count = 0;
    size_t pos = 0;
    while ((pos = content.find("Message", pos)) != std::string::npos) {
        ++count;
        pos += 7;
    }
    
    EXPECT_EQ(count, 50);
}
