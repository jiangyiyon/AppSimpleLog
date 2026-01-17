// Unit tests for process ID detection

#include <gtest/gtest.h>
#include "speckit/log/async_logger.h"
#include "speckit/log/platform.h"
#include <fstream>
#include <filesystem>



class ProcessIsolationTest : public ::testing::Test {
protected:
    std::string log_base_path_;

    void SetUp() override {
        // Get absolute path for current directory
        std::filesystem::path current_path = std::filesystem::current_path();
        log_base_path_ = (current_path / "process_test").string();

        // Clean up any existing test files
        std::filesystem::remove_all(log_base_path_ + "*.log");
    }

    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all(log_base_path_ + "*.log");
    }
};

TEST_F(ProcessIsolationTest, ProcessId_IsNotZero) {
    // Create async logger
    auto logger = AsyncLogger::create(log_base_path_);
    ASSERT_NE(logger, nullptr);

    // Log a message to trigger process ID detection
    logger->log(LogLevel::kLogLevelInfo, "Test", "Process ID test");
    logger->flush();

    // Read log file
    std::string log_file = log_base_path_ + ".log";
    std::ifstream file(log_file);
    ASSERT_TRUE(file.is_open());

    std::string line;
    std::getline(file, line);
    file.close();

    // Verify PID is not zero
    EXPECT_TRUE(line.find(",") != std::string::npos);
    EXPECT_TRUE(line.find("]") != std::string::npos);

    // PID should be between brackets [PID, TID]
    size_t open_bracket = line.find("[");
    size_t comma = line.find(",");
    ASSERT_NE(open_bracket, std::string::npos);
    ASSERT_NE(comma, std::string::npos);
    ASSERT_LT(open_bracket, comma);

    std::string pid_str = line.substr(open_bracket + 1, comma - open_bracket - 1);
    EXPECT_FALSE(pid_str.empty());

    // PID should not be "0"
    EXPECT_NE(pid_str, "0");
}

TEST_F(ProcessIsolationTest, ThreadId_IsCaptured) {
    // Create async logger
    auto logger = AsyncLogger::create(log_base_path_);
    ASSERT_NE(logger, nullptr);

    // Log a message
    logger->log(LogLevel::kLogLevelInfo, "Test", "Thread ID test");
    logger->flush();

    // Read log file
    std::string log_file = log_base_path_ + ".log";
    std::ifstream file(log_file);
    ASSERT_TRUE(file.is_open());

    std::string line;
    std::getline(file, line);
    file.close();

    // Verify TID is captured
    size_t comma = line.find(",");
    size_t close_bracket = line.find("]");
    ASSERT_NE(comma, std::string::npos);
    ASSERT_NE(close_bracket, std::string::npos);
    ASSERT_LT(comma, close_bracket);

    std::string tid_str = line.substr(comma + 1, close_bracket - comma - 1);
    EXPECT_FALSE(tid_str.empty());
}

TEST_F(ProcessIsolationTest, ProcessId_RemainsConstant) {
    // Create async logger
    auto logger = AsyncLogger::create(log_base_path_);
    ASSERT_NE(logger, nullptr);

    // Log multiple messages
    logger->log(LogLevel::kLogLevelInfo, "Test", "Message 1");
    logger->log(LogLevel::kLogLevelInfo, "Test", "Message 2");
    logger->log(LogLevel::kLogLevelInfo, "Test", "Message 3");
    logger->flush();

    // Read log file
    std::string log_file = log_base_path_ + ".log";
    std::ifstream file(log_file);
    ASSERT_TRUE(file.is_open());

    std::string line1, line2, line3;
    std::getline(file, line1);
    std::getline(file, line2);
    std::getline(file, line3);
    file.close();

    // Extract PIDs from all lines
    auto extractPid = [](const std::string& line) -> std::string {
        size_t open_bracket = line.find("[");
        size_t comma = line.find(",");
        if (open_bracket != std::string::npos && comma != std::string::npos) {
            return line.substr(open_bracket + 1, comma - open_bracket - 1);
        }
        return "";
    };

    std::string pid1 = extractPid(line1);
    std::string pid2 = extractPid(line2);
    std::string pid3 = extractPid(line3);

    // All PIDs should be identical
    EXPECT_EQ(pid1, pid2);
    EXPECT_EQ(pid2, pid3);
    EXPECT_FALSE(pid1.empty());
}

TEST_F(ProcessIsolationTest, FirstProcess_UsesBaseFileName) {
    // Create async logger (assumes first process)
    auto logger = AsyncLogger::create(log_base_path_);
    ASSERT_NE(logger, nullptr);

    // Log a message
    logger->log(LogLevel::kLogLevelInfo, "Test", "First process test");
    logger->flush();

    // Verify base filename is used
    std::string base_log_file = log_base_path_ + ".log";
    EXPECT_TRUE(std::filesystem::exists(base_log_file));
    EXPECT_FALSE(std::filesystem::exists(log_base_path_ + "_1234.log"));  // Should not exist
}

TEST_F(ProcessIsolationTest, SubsequentProcess_UsPidFileName) {
    // Simulate subsequent process by creating a file with base name
    {
        std::string base_log_file = log_base_path_ + ".log";
        std::ofstream dummy_file(base_log_file);
        dummy_file << "Dummy content";
    }

    // Create async logger (should detect existing file and use PID)
    auto logger = AsyncLogger::create(log_base_path_);
    ASSERT_NE(logger, nullptr);

    // Log a message
    logger->log(LogLevel::kLogLevelInfo, "Test", "Subsequent process test");
    logger->flush();

    // Verify PID filename is used
    // Note: This test assumes the first file exists
    std::vector<std::string> log_files;
    std::filesystem::path current_path = std::filesystem::current_path();
    for (const auto& entry : std::filesystem::directory_iterator(current_path)) {
        std::string filename = entry.path().filename().string();
        std::string base_name = std::filesystem::path(log_base_path_).filename().string();
        if (filename.find(base_name) == 0 && filename.find(".log") == filename.length() - 4) {
            log_files.push_back(entry.path().string());
        }
    }

    // Should have base file and PID file
    EXPECT_GE(log_files.size(), 1);

    // At least one file should have _PID format
    bool has_pid_file = false;
    std::string base_log_file = log_base_path_ + ".log";
    for (const auto& file : log_files) {
        if (file.find("_") != std::string::npos && file != base_log_file) {
            has_pid_file = true;
            break;
        }
    }
    EXPECT_TRUE(has_pid_file);
}
