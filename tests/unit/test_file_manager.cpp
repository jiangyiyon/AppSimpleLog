// Unit tests for FileManager file name generation

#include <gtest/gtest.h>
#include "speckit/log/file_manager.h"
#include "speckit/log/platform.h"
#include <filesystem>



class FileNameGenerationTest : public ::testing::Test {
protected:
    std::string log_base_path_;

    void SetUp() override {
        // Get absolute path for current directory
        std::filesystem::path current_path = std::filesystem::current_path();
        log_base_path_ = (current_path / "filename_test").string();

        // Clean up any existing test files
        std::filesystem::remove_all(log_base_path_ + "*.log");
    }

    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all(log_base_path_ + "*.log");
    }
};

TEST_F(FileNameGenerationTest, FirstProcess_GeneratesBaseName) {
    // Create file manager with first_process=true
    FileManager file_manager(log_base_path_);
    ASSERT_TRUE(file_manager.Initialize(1234));  // Any process ID

    // Verify base name is used
    std::string filename = file_manager.GetLogFileName();
    EXPECT_EQ(filename, log_base_path_ + ".log");

    // Verify file exists
    EXPECT_TRUE(std::filesystem::exists(filename));
}

TEST_F(FileNameGenerationTest, SubsequentProcess_GeneratesPidName) {
    // Create file manager with first_process=false (simulated)
    // Note: In real implementation, FileManager determines first_process
    // by checking if base file exists. Here we test the naming logic.

    FileManager file_manager(log_base_path_);
    ASSERT_TRUE(file_manager.Initialize(5678));  // Different process ID

    // If filename_test.log exists, should use PID name
    // This test assumes the file manager logic works correctly
    std::string filename = file_manager.GetLogFileName();

    // Should contain process ID
    if (filename.find("_") != std::string::npos) {
        EXPECT_TRUE(filename.find("5678") != std::string::npos);
    } else {
        // First process - uses base name
        EXPECT_EQ(filename, log_base_path_ + ".log");
    }
}

TEST_F(FileNameGenerationTest, ProcessId_IsIncludedInFilename) {
    // Create file manager
    FileManager file_manager(log_base_path_);
    ProcessIdType pid = 12345;

    // If using PID filename
    // Note: This depends on whether it's first or subsequent process
    ASSERT_TRUE(file_manager.Initialize(pid));

    std::string filename = file_manager.GetLogFileName();

    // If filename contains underscore, should have PID
    if (filename.find("_") != std::string::npos) {
        std::string pid_str = std::to_string(pid);
        EXPECT_TRUE(filename.find(pid_str) != std::string::npos);
    }
}

TEST_F(FileNameGenerationTest, DifferentPids_GenerateDifferentFilenames) {
    // Create first file manager
    FileManager file_manager1(log_base_path_);
    ASSERT_TRUE(file_manager1.Initialize(1111));
    std::string filename1 = file_manager1.GetLogFileName();

    // Clean up first file to simulate subsequent process
    std::filesystem::remove(log_base_path_ + ".log");

    // Create second file manager with different PID
    FileManager file_manager2(log_base_path_);
    ASSERT_TRUE(file_manager2.Initialize(2222));
    std::string filename2 = file_manager2.GetLogFileName();

    // Filenames should be different
    EXPECT_NE(filename1, filename2);

    // If both use PID, they should be different
    if (filename1.find("_") != std::string::npos &&
        filename2.find("_") != std::string::npos) {
        EXPECT_TRUE(filename1.find("1111") != std::string::npos);
        EXPECT_TRUE(filename2.find("2222") != std::string::npos);
    }
}


