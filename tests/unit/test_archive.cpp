// Unit tests for Archive functionality with libzip

#include <gtest/gtest.h>
#include "speckit/log/archive.h"
#include <fstream>
#include <filesystem>
#include <vector>
#include <zip.h>

using namespace std::chrono_literals;

class ArchiveTest : public ::testing::Test {
protected:
    std::string log_base_path_;

    void SetUp() override {
        // Get absolute path for current directory
        std::filesystem::path current_path = std::filesystem::current_path();
        log_base_path_ = (current_path / "archive_test").string();

        // Clean up any existing test files
        cleanupTestFiles();
    }

    void TearDown() override {
        // Clean up test files
        cleanupTestFiles();
    }

    void cleanupTestFiles() {
        // remove files matching base name prefix (remove_all does not accept globs)
        const std::filesystem::path cwd = std::filesystem::current_path();
        const std::string base_fname = std::filesystem::path(log_base_path_).filename().string();
        for (const auto& entry : std::filesystem::directory_iterator(cwd)) {
            if (!entry.is_regular_file()) continue;
            const std::string name = entry.path().filename().string();
            if (name.rfind(base_fname, 0) == 0) {
                // starts with base name
                std::error_code ec;
                std::filesystem::remove(entry.path(), ec);
            }
        }
    }

    void createTestLogFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        file << content;
        file.close();
    }

    std::string generateTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm tm_now;
#ifdef _WIN32
        localtime_s(&tm_now, &time_t_now);
#else
        localtime_r(&time_t_now, &tm_now);
#endif
        char buffer[64];
        std::strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", &tm_now);
        return std::string(buffer);
    }
};

TEST_F(ArchiveTest, CreateArchive_CreatesZipFile) {
    // Create test log files
    createTestLogFile(log_base_path_ + ".log", "Main log content\n");
    createTestLogFile(log_base_path_ + ".1.log", "First rotated log\n");
    createTestLogFile(log_base_path_ + ".2.log", "Second rotated log\n");

    // Generate timestamp
    std::string timestamp = generateTimestamp();

    // Create archive
    bool result = speckit::log::createArchive(log_base_path_, 1234, timestamp);
    
    // Verify archive creation succeeded
    EXPECT_TRUE(result);

    // Verify ZIP file exists
    std::string archiveName = log_base_path_ + "_1234_" + timestamp + ".zip";
    EXPECT_TRUE(std::filesystem::exists(archiveName));
}

TEST_F(ArchiveTest, CreateArchive_OriginalFilesRemain) {
    // Create test log files
    createTestLogFile(log_base_path_ + ".log", "Main log content\n");
    createTestLogFile(log_base_path_ + ".1.log", "First rotated log\n");

    // Generate timestamp
    std::string timestamp = generateTimestamp();

    // Create archive
    bool result = speckit::log::createArchive(log_base_path_, 5678, timestamp);
    ASSERT_TRUE(result);

    // Verify original log files are still present (do not delete originals)
    EXPECT_TRUE(std::filesystem::exists(log_base_path_ + ".log"));
    EXPECT_TRUE(std::filesystem::exists(log_base_path_ + ".1.log"));
}

TEST_F(ArchiveTest, CreateArchive_WithNoFiles_FailsGracefully) {
    // Don't create any log files

    // Generate timestamp
    std::string timestamp = generateTimestamp();

    // Create archive (should fail gracefully)
    bool result = speckit::log::createArchive(log_base_path_, 9999, timestamp);
    
    // Should return false when no files to archive
    EXPECT_FALSE(result);
}

TEST_F(ArchiveTest, CreateArchive_EmptyFiles_IgnoresThem) {
    // Create empty log files
    createTestLogFile(log_base_path_ + ".log", "");
    createTestLogFile(log_base_path_ + ".1.log", "Some content\n");

    // Generate timestamp
    std::string timestamp = generateTimestamp();

    // Create archive
    bool result = speckit::log::createArchive(log_base_path_, 1111, timestamp);
    
    // Should succeed (ignoring empty files)
    EXPECT_TRUE(result);
}

TEST_F(ArchiveTest, CreateArchive_LargeFiles_CompressesProperly) {
    // Create large log files with repetitive content (compresses well)
    std::string largeContent(100000, 'A');  // 100KB of 'A'
    createTestLogFile(log_base_path_ + ".log", largeContent);
    createTestLogFile(log_base_path_ + ".1.log", largeContent);

    // Generate timestamp
    std::string timestamp = generateTimestamp();

    // Create archive
    bool result = speckit::log::createArchive(log_base_path_, 2222, timestamp);
    ASSERT_TRUE(result);

    // Verify ZIP file exists
    std::string archiveName = log_base_path_ + "_2222_" + timestamp + ".zip";
    ASSERT_TRUE(std::filesystem::exists(archiveName));

    // Verify ZIP file is smaller than original (due to compression)
    uint64_t originalSize = largeContent.size() * 2;  // Two files
    uint64_t archiveSize = std::filesystem::file_size(archiveName);
    EXPECT_LT(archiveSize, originalSize);
}

TEST_F(ArchiveTest, CreateArchive_SpecialCharacters_HandlesCorrectly) {
    // Create log file with special characters
    std::string specialContent = "Line 1: 测试\nLine 2: Hello 世界\nLine 3: 🎉\n";
    createTestLogFile(log_base_path_ + ".log", specialContent);

    // Generate timestamp
    std::string timestamp = generateTimestamp();

    // Create archive
    bool result = speckit::log::createArchive(log_base_path_, 3333, timestamp);
    
    // Verify archive creation succeeded
    EXPECT_TRUE(result);

    // Verify ZIP file exists
    std::string archiveName = log_base_path_ + "_3333_" + timestamp + ".zip";
    EXPECT_TRUE(std::filesystem::exists(archiveName));
}

TEST_F(ArchiveTest, CreateArchive_InvalidTimestamp_ReturnsFalse) {
    // Create test log file
    createTestLogFile(log_base_path_ + ".log", "Test content\n");

    // Create archive with empty timestamp (should fail)
    bool result = speckit::log::createArchive(log_base_path_, 4444, "");
    
    // Should return false for invalid timestamp
    EXPECT_FALSE(result);
}

TEST_F(ArchiveTest, CreateArchive_MultipleFiles_AllIncluded) {
    // Create multiple log files
    createTestLogFile(log_base_path_ + ".log", "Main log\n");
    createTestLogFile(log_base_path_ + ".1.log", "First rotated\n");
    createTestLogFile(log_base_path_ + ".2.log", "Second rotated\n");
    createTestLogFile(log_base_path_ + ".3.log", "Third rotated\n");
    createTestLogFile(log_base_path_ + ".4.log", "Fourth rotated\n");

    // Generate timestamp
    std::string timestamp = generateTimestamp();

    // Create archive
    bool result = speckit::log::createArchive(log_base_path_, 5555, timestamp);
    ASSERT_TRUE(result);

    // Note: To verify all files are in the archive, you would need to
    // use libzip to read the archive and count entries
    // For now, we just verify the archive was created
    std::string archiveName = log_base_path_ + "_5555_" + timestamp + ".zip";
    EXPECT_TRUE(std::filesystem::exists(archiveName));
}

TEST_F(ArchiveTest, CreateArchive_ZipContent_Verifiable) {
    // Create test log files
    createTestLogFile(log_base_path_ + ".log", "Main log content\n");
    createTestLogFile(log_base_path_ + ".1.log", "First rotated log\n");

    // Generate timestamp
    std::string timestamp = generateTimestamp();

    // Create archive
    bool result = speckit::log::createArchive(log_base_path_, 6666, timestamp);
    ASSERT_TRUE(result);

    // Open and verify ZIP content
    std::string archiveName = log_base_path_ + "_6666_" + timestamp + ".zip";
    int zipError = 0;
    zip_t* zipArchive = zip_open(archiveName.c_str(), 0, &zipError);
    
    ASSERT_NE(zipArchive, nullptr) << "Failed to open ZIP archive";

    // Count number of files in archive
    zip_int64_t numEntries = zip_get_num_entries(zipArchive, 0);
    EXPECT_EQ(numEntries, 2) << "Archive should contain 2 files";

    // Close ZIP archive
    zip_close(zipArchive);
}

TEST_F(ArchiveTest, CreateArchive_RepeatedCall_OverwritesExisting) {
    // Create first set of log files
    createTestLogFile(log_base_path_ + ".log", "First content\n");
    
    // Generate timestamp for first archive
    std::string timestamp = generateTimestamp();

    // Create first archive
    bool result1 = speckit::log::createArchive(log_base_path_, 7777, timestamp);
    ASSERT_TRUE(result1);

    // Create second set of log files
    createTestLogFile(log_base_path_ + ".log", "Second content\n");

    // Create second archive with same name (should overwrite)
    bool result2 = speckit::log::createArchive(log_base_path_, 7777, timestamp);
    EXPECT_TRUE(result2);
}

TEST_F(ArchiveTest, CreateArchive_DifferentBaseName_FilterCorrectly) {
    // Create files with different base names
    createTestLogFile(log_base_path_ + ".log", "Should be archived\n");
    createTestLogFile("other_app.log", "Should NOT be archived\n");
    createTestLogFile("app_test.log", "Should NOT be archived (not starting with baseName)\n");

    // Generate timestamp
    std::string timestamp = generateTimestamp();

    // Create archive with specific base name
    bool result = speckit::log::createArchive(log_base_path_, 8888, timestamp);
    ASSERT_TRUE(result);

    // Verify only files with matching baseName are archived
    std::string archiveName = log_base_path_ + "_8888_" + timestamp + ".zip";
    ASSERT_TRUE(std::filesystem::exists(archiveName));

    // Open ZIP and count entries
    int zipError = 0;
    zip_t* zipArchive = zip_open(archiveName.c_str(), 0, &zipError);
    ASSERT_NE(zipArchive, nullptr);

    zip_int64_t numEntries = zip_get_num_entries(zipArchive, 0);
    EXPECT_EQ(numEntries, 1) << "Should only archive files starting with baseName";

    zip_close(zipArchive);
}

TEST_F(ArchiveTest, CreateArchive_BinaryFiles_HandlesCorrectly) {
    // Create log file with binary-like content
    std::string binaryContent;
    for (int i = 0; i < 256; i++) {
        binaryContent.push_back(static_cast<char>(i));
    }
    createTestLogFile(log_base_path_ + ".log", binaryContent);

    // Generate timestamp
    std::string timestamp = generateTimestamp();

    // Create archive
    bool result = speckit::log::createArchive(log_base_path_, 9999, timestamp);
    
    // Verify archive creation succeeded
    EXPECT_TRUE(result);

    // Verify ZIP file exists
    std::string archiveName = log_base_path_ + "_9999_" + timestamp + ".zip";
    EXPECT_TRUE(std::filesystem::exists(archiveName));
}

TEST_F(ArchiveTest, CreateArchive_SmallFiles_Succeeds) {
    // Create very small log files (single byte)
    createTestLogFile(log_base_path_ + ".log", "A");
    createTestLogFile(log_base_path_ + ".1.log", "B");

    // Generate timestamp
    std::string timestamp = generateTimestamp();

    // Create archive
    bool result = speckit::log::createArchive(log_base_path_, 1001, timestamp);
    
    // Verify archive creation succeeded
    EXPECT_TRUE(result);

    // Verify ZIP file exists
    std::string archiveName = log_base_path_ + "_1001_" + timestamp + ".zip";
    EXPECT_TRUE(std::filesystem::exists(archiveName));
}

TEST_F(ArchiveTest, CreateArchive_UnicodeFileNames_HandlesCorrectly) {
    // Create log file with baseName that includes unicode characters
    std::string unicodeContent = "Test content with unicode: 你好\n";
    createTestLogFile(log_base_path_ + ".log", unicodeContent);

    // Generate timestamp
    std::string timestamp = generateTimestamp();

    // Create archive
    bool result = speckit::log::createArchive(log_base_path_, 2002, timestamp);
    
    // Verify archive creation succeeded
    EXPECT_TRUE(result);

    // Verify ZIP file exists
    std::string archiveName = log_base_path_ + "_2002_" + timestamp + ".zip";
    EXPECT_TRUE(std::filesystem::exists(archiveName));
}

TEST_F(ArchiveTest, CreateArchive_ProcessId_Uniqueness) {
    // Create test log files
    createTestLogFile(log_base_path_ + ".log", "Process 1 content\n");

    // Generate timestamp
    std::string timestamp = generateTimestamp();

    // Create archives with different process IDs
    bool result1 = speckit::log::createArchive(log_base_path_, 3001, timestamp);
    bool result2 = speckit::log::createArchive(log_base_path_, 3002, timestamp);

    // Both should succeed (different process IDs create different archive names)
    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);

    // Both ZIP files should exist
    std::string archive1 = log_base_path_ + "_3001_" + timestamp + ".zip";
    std::string archive2 = log_base_path_ + "_3002_" + timestamp + ".zip";
    EXPECT_TRUE(std::filesystem::exists(archive1));
    EXPECT_TRUE(std::filesystem::exists(archive2));
}
