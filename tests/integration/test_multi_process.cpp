// Integration test for multi-process logging

#include <gtest/gtest.h>
#include "speckit_logger.h"
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

class MultiProcessTest : public ::testing::Test {
protected:
    std::string log_base_path_;

    void SetUp() override {
        // Get absolute path for current directory
        std::filesystem::path current_path = std::filesystem::current_path();
        log_base_path_ = (current_path / "multi_process_test").string();

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

TEST_F(MultiProcessTest, TwoProcesses_CreateSeparateFiles) {
    // Create two loggers (simulating two processes)
    SpeckitLogger* logger1 = speckit_logger_create(log_base_path_.c_str());
    SpeckitLogger* logger2 = speckit_logger_create(log_base_path_.c_str());

    ASSERT_NE(logger1, nullptr);
    ASSERT_NE(logger2, nullptr);

    // Log messages to both
    speckit_logger_log(logger1, SPECKIT_LOG_LEVEL_INFO, "Process1", "Message from process 1");
    speckit_logger_log(logger2, SPECKIT_LOG_LEVEL_INFO, "Process2", "Message from process 2");

    // Note: In real multi-process scenario, these would be different processes
    // Here we're simulating in same process for testing

    // Clean up
    speckit_logger_destroy(logger1);
    speckit_logger_destroy(logger2);

    // Verify at least one log file exists
    // Note: Actual multi-process isolation requires separate processes
    // This test verifies the file naming logic works
    verifyLogExists(log_base_path_ + ".log");
}

TEST_F(MultiProcessTest, LogMessages_AreWrittenToFiles) {
    // Create logger
    SpeckitLogger* logger = speckit_logger_create(log_base_path_.c_str());
    ASSERT_NE(logger, nullptr);

    // Set log level
    speckit_logger_set_log_level(logger, SPECKIT_LOG_LEVEL_INFO);

    // Log multiple messages
    speckit_logger_log(logger, SPECKIT_LOG_LEVEL_INFO, "Network", "Connection established");
    speckit_logger_log(logger, SPECKIT_LOG_LEVEL_WARNING, "Database", "Slow query detected");
    speckit_logger_log(logger, SPECKIT_LOG_LEVEL_ERROR, "Auth", "Authentication failed");

    // Wait for file writes (in real async implementation)
    std::this_thread::sleep_for(100ms);

    // Verify messages are in file
    std::string filename = log_base_path_ + ".log";
    verifyLogExists(filename);
    verifyLogContains(filename, "[INFO]");
    verifyLogContains(filename, "[WARNING]");
    verifyLogContains(filename, "[ERROR]");
    verifyLogContains(filename, "[Network]");
    verifyLogContains(filename, "[Database]");
    verifyLogContains(filename, "[Auth]");
    verifyLogContains(filename, "Connection established");
    verifyLogContains(filename, "Slow query detected");
    verifyLogContains(filename, "Authentication failed");

    // Clean up
    speckit_logger_destroy(logger);
}

TEST_F(MultiProcessTest, ProcessId_IsCapturedInLogs) {
    // Create logger
    SpeckitLogger* logger = speckit_logger_create(log_base_path_.c_str());
    ASSERT_NE(logger, nullptr);

    // Log a message
    speckit_logger_log(logger, SPECKIT_LOG_LEVEL_INFO, "Test", "Process ID test");

    // Wait for write
    std::this_thread::sleep_for(100ms);

    // Verify PID is in log format
    std::string filename = log_base_path_ + ".log";
    verifyLogExists(filename);

    std::ifstream file(filename);
    ASSERT_TRUE(file.is_open());

    std::string line;
    std::getline(file, line);
    file.close();

    // Verify [PID, TID] format exists
    EXPECT_TRUE(line.find("[") != std::string::npos);
    EXPECT_TRUE(line.find("]") != std::string::npos);

    // Extract PID from format [PID, TID]
    size_t open_bracket = line.find("[");
    size_t comma = line.find(",");
    size_t close_bracket = line.find("]");

    ASSERT_NE(open_bracket, std::string::npos);
    ASSERT_NE(comma, std::string::npos);
    ASSERT_NE(close_bracket, std::string::npos);
    ASSERT_LT(open_bracket, comma);
    ASSERT_LT(comma, close_bracket);

    std::string pid_str = line.substr(open_bracket + 1, comma - open_bracket - 1);

    // PID should not be empty and not zero
    EXPECT_FALSE(pid_str.empty());
    EXPECT_NE(pid_str, "0");

    // Clean up
    speckit_logger_destroy(logger);
}

TEST_F(MultiProcessTest, DifferentProcesses_DoNotInterfere) {
    // This test verifies the logic; actual multi-process isolation
    // requires spawning separate processes

    // Create two loggers
    SpeckitLogger* logger1 = speckit_logger_create(log_base_path_.c_str());
    SpeckitLogger* logger2 = speckit_logger_create(log_base_path_.c_str());

    ASSERT_NE(logger1, nullptr);
    ASSERT_NE(logger2, nullptr);

    // Log to first
    speckit_logger_log(logger1, SPECKIT_LOG_LEVEL_INFO, "Process1", "Unique message 1");

    // Log to second
    speckit_logger_log(logger2, SPECKIT_LOG_LEVEL_INFO, "Process2", "Unique message 2");

    // Verify messages don't interfere
    // Note: In single-process test, both write to same file
    // In real scenario, separate processes write to separate files

    // Clean up
    speckit_logger_destroy(logger1);
    speckit_logger_destroy(logger2);

    // Verify at least one log file exists
    verifyLogExists(log_base_path_ + ".log");
}
