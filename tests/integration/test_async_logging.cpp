// Integration test for async logging

#include <gtest/gtest.h>
#include "speckit_logger.h"
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

class AsyncLoggingIntegrationTest : public ::testing::Test {
protected:
    std::string log_base_path_;

    void SetUp() override {
        // Get absolute path for current directory
        std::filesystem::path current_path = std::filesystem::current_path();
        log_base_path_ = (current_path / "async_integration_test").string();

        // Clean up any existing test files
        std::filesystem::remove_all(log_base_path_ + "*.log");
    }

    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all(log_base_path_ + "*.log");
    }

    void verifyLogExists(const std::string& filename) {
        EXPECT_TRUE(std::filesystem::exists(filename))
            << "Log file " << filename << " does not exist";
    }

    void verifyLogContains(const std::string& filename, const std::string& content) {
        std::ifstream file(filename);
        ASSERT_TRUE(file.is_open()) << "Cannot open " << filename;

        std::string file_content((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
        file.close();

        EXPECT_TRUE(file_content.find(content) != std::string::npos)
            << "File " << filename << " does not contain: " << content;
    }
};

TEST_F(AsyncLoggingIntegrationTest, AsyncLog_NonBlocking) {
    // Create logger
    SpeckitLogger* logger = speckit_logger_create(log_base_path_.c_str());
    ASSERT_NE(logger, nullptr);

    // Set log level to INFO
    speckit_logger_set_log_level(logger, SPECKIT_LOG_LEVEL_INFO);

    // Log multiple messages rapidly
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; ++i) {
        std::string msg = "Message " + std::to_string(i);
        speckit_logger_log(logger, SPECKIT_LOG_LEVEL_INFO, "Test", msg.c_str());
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // All 100 calls should complete quickly (non-blocking)
    EXPECT_LT(duration.count(), 100) << "100 async log calls took " << duration.count() << " ms (too slow)";

    // Wait for background processing
    std::this_thread::sleep_for(500ms);

    // Verify logs are in file
    std::string filename = log_base_path_ + ".log";
    verifyLogExists(filename);

    // Clean up
    speckit_logger_destroy(logger);
}

TEST_F(AsyncLoggingIntegrationTest, AsyncLog_WritesCorrectly) {
    // Create logger
    SpeckitLogger* logger = speckit_logger_create(log_base_path_.c_str());
    ASSERT_NE(logger, nullptr);

    speckit_logger_set_log_level(logger, SPECKIT_LOG_LEVEL_INFO);

    // Log messages with different levels and tags
    speckit_logger_log(logger, SPECKIT_LOG_LEVEL_DEBUG, "Test", "Debug message");
    speckit_logger_log(logger, SPECKIT_LOG_LEVEL_INFO, "Network", "Info message");
    speckit_logger_log(logger, SPECKIT_LOG_LEVEL_WARNING, "Database", "Warning message");
    speckit_logger_log(logger, SPECKIT_LOG_LEVEL_ERROR, "Auth", "Error message");

    // Wait for processing
    std::this_thread::sleep_for(500ms);

    // Verify log file
    std::string filename = log_base_path_ + ".log";
    verifyLogExists(filename);
    verifyLogContains(filename, "[INFO]");
    verifyLogContains(filename, "[WARNING]");
    verifyLogContains(filename, "[ERROR]");
    verifyLogContains(filename, "[Network]");
    verifyLogContains(filename, "[Database]");
    verifyLogContains(filename, "[Auth]");

    // Clean up
    speckit_logger_destroy(logger);
}

TEST_F(AsyncLoggingIntegrationTest, AsyncLog_LevelFilteringWorks) {
    // Create logger
    SpeckitLogger* logger = speckit_logger_create(log_base_path_.c_str());
    ASSERT_NE(logger, nullptr);

    // Set log level to WARNING
    speckit_logger_set_log_level(logger, SPECKIT_LOG_LEVEL_WARNING);

    // Log different levels
    speckit_logger_log(logger, SPECKIT_LOG_LEVEL_DEBUG, "Test", "Debug");
    speckit_logger_log(logger, SPECKIT_LOG_LEVEL_INFO, "Test", "Info");
    speckit_logger_log(logger, SPECKIT_LOG_LEVEL_WARNING, "Test", "Warning");
    speckit_logger_log(logger, SPECKIT_LOG_LEVEL_ERROR, "Test", "Error");

    // Wait for processing
    std::this_thread::sleep_for(500ms);

    // Verify only WARNING and ERROR are in file
    std::string filename = log_base_path_ + ".log";
    verifyLogExists(filename);

    std::ifstream file(filename);
    ASSERT_TRUE(file.is_open());

    std::string content((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    file.close();

    // DEBUG and INFO should not be present
    EXPECT_FALSE(content.find("Debug") != std::string::npos);
    EXPECT_FALSE(content.find("Info") != std::string::npos);

    // WARNING and ERROR should be present
    EXPECT_TRUE(content.find("Warning") != std::string::npos);
    EXPECT_TRUE(content.find("Error") != std::string::npos);

    // Clean up
    speckit_logger_destroy(logger);
}

TEST_F(AsyncLoggingIntegrationTest, AsyncLog_HighFrequency_DoesNotBlock) {
    // Create logger
    SpeckitLogger* logger = speckit_logger_create(log_base_path_.c_str());
    ASSERT_NE(logger, nullptr);

    speckit_logger_set_log_level(logger, SPECKIT_LOG_LEVEL_INFO);

    // Log 1000 messages rapidly
    const int NUM_LOGS = 1000;
    for (int i = 0; i < NUM_LOGS; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        std::string msg = "Message " + std::to_string(i);
        speckit_logger_log(logger, SPECKIT_LOG_LEVEL_INFO, "Test", msg.c_str());
        auto end = std::chrono::high_resolution_clock::now();

        // Each call should be fast (<1ms)
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        EXPECT_LT(duration.count(), 1000) << "Log call " << i << " took " << duration.count() << " µs";
    }

    // Wait for background processing
    std::this_thread::sleep_for(1s);

    // Verify messages are in file
    std::string filename = log_base_path_ + ".log";
    verifyLogExists(filename);

    // Clean up
    speckit_logger_destroy(logger);
}

TEST_F(AsyncLoggingIntegrationTest, AsyncLog_FlushOnDestroy) {
    // Create logger
    SpeckitLogger* logger = speckit_logger_create(log_base_path_.c_str());
    ASSERT_NE(logger, nullptr);

    speckit_logger_set_log_level(logger, SPECKIT_LOG_LEVEL_INFO);

    // Log 100 messages
    for (int i = 0; i < 100; ++i) {
        std::string msg = "Message " + std::to_string(i);
        speckit_logger_log(logger, SPECKIT_LOG_LEVEL_INFO, "Test", msg.c_str());
    }

    // Destroy logger (should flush all pending logs)
    speckit_logger_destroy(logger);

    // Wait a bit for file operations
    std::this_thread::sleep_for(500ms);

    // Verify all 100 messages are in file
    std::string filename = log_base_path_ + ".log";
    verifyLogExists(filename);

    // Count log entries
    std::ifstream file(filename);
    ASSERT_TRUE(file.is_open());

    int count = 0;
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("[INFO]") != std::string::npos) {
            ++count;
        }
    }
    file.close();

    // All 100 messages should be present
    EXPECT_EQ(count, 100) << "Expected 100 log entries, found " << count;
}
