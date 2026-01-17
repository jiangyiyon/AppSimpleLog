// Unit tests for LogEntry formatting

#include <gtest/gtest.h>
#include "speckit/log/log_entry.h"
#include "speckit/log/log_level.h"
#include <string>



class LogEntryTest : public ::testing::Test {
protected:
    LogEntry createTestEntry(LogLevel level, const std::string& tag, 
                         const std::string& message) {
        return LogEntry(
            level,
            12345,  // Timestamp in ms
            1234,    // Process ID
            5678,    // Thread ID
            tag,
            message
        );
    }
};

TEST_F(LogEntryTest, Constructor_CopiesAllFields) {
    auto entry = createTestEntry(LogLevel::kLogLevelInfo, "Network", "Connected");

    EXPECT_EQ(entry.level, LogLevel::kLogLevelInfo);
    EXPECT_EQ(entry.timestamp_ms, 12345);
    EXPECT_EQ(entry.process_id, 1234);
    EXPECT_EQ(entry.thread_id, 5678);
    EXPECT_EQ(entry.tag, "Network");
    EXPECT_EQ(entry.message, "Connected");
}

TEST_F(LogEntryTest, MoveConstructor_TransfersOwnership) {
    auto entry1 = createTestEntry(LogLevel::kLogLevelError, "Database", "Failed");

    // Move construct
    LogEntry entry2 = std::move(entry1);

    EXPECT_EQ(entry2.level, LogLevel::kLogLevelError);
    EXPECT_EQ(entry2.timestamp_ms, 12345);
    EXPECT_EQ(entry2.process_id, 1234);
    EXPECT_EQ(entry2.thread_id, 5678);
    EXPECT_EQ(entry2.tag, "Database");
    EXPECT_EQ(entry2.message, "Failed");
}

TEST_F(LogEntryTest, MoveAssignment_TransfersOwnership) {
    auto entry1 = createTestEntry(LogLevel::kLogLevelWarning, "UI", "Button clicked");
    auto entry2 = createTestEntry(LogLevel::kLogLevelInfo, "Network", "Connected");

    // Move assign
    entry1 = std::move(entry2);

    EXPECT_EQ(entry1.level, LogLevel::kLogLevelInfo);
    EXPECT_EQ(entry1.tag, "Network");
    EXPECT_EQ(entry1.message, "Connected");
}

TEST_F(LogEntryTest, StringView_ReferencesOriginalString) {
    std::string tag = "TestTag";
    std::string message = "Test message";

    auto entry = createTestEntry(LogLevel::kLogLevelInfo, tag, message);
    
    // String view should reference original strings
    EXPECT_EQ(entry.tag.data(), tag.data());
    EXPECT_EQ(entry.message.data(), message.data());
}

TEST_F(LogEntryTest, ShouldLog_DebugWithInfoLevel) {
    EXPECT_TRUE(shouldLog(LogLevel::kLogLevelInfo, LogLevel::kLogLevelInfo));
}

TEST_F(LogEntryTest, ShouldLog_WarningWithInfoLevel) {
    EXPECT_TRUE(shouldLog(LogLevel::kLogLevelWarning, LogLevel::kLogLevelInfo));
}

TEST_F(LogEntryTest, ShouldLog_ErrorWithInfoLevel) {
    EXPECT_TRUE(shouldLog(LogLevel::kLogLevelError, LogLevel::kLogLevelInfo));
}

TEST_F(LogEntryTest, ShouldNotLog_DebugWithWarningLevel) {
    EXPECT_FALSE(shouldLog(LogLevel::kLogLevelDebug, LogLevel::kLogLevelWarning));
}

TEST_F(LogEntryTest, ShouldNotLog_InfoWithErrorLevel) {
    EXPECT_FALSE(shouldLog(LogLevel::kLogLevelInfo, LogLevel::kLogLevelError));
}

TEST_F(LogEntryTest, LevelToString_ReturnsCorrectString) {
    EXPECT_STREQ(levelToString(LogLevel::kLogLevelDebug), "DEBUG");
    EXPECT_STREQ(levelToString(LogLevel::kLogLevelInfo), "INFO");
    EXPECT_STREQ(levelToString(LogLevel::kLogLevelWarning), "WARNING");
    EXPECT_STREQ(levelToString(LogLevel::kLogLevelError), "ERROR");
}

TEST_F(LogEntryTest, LevelToString_ReturnsUnknownForInvalid) {
    // Note: This tests behavior with an out-of-range enum value
    // In practice, this should not happen with valid enum values
    EXPECT_STREQ(levelToString(static_cast<LogLevel>(99)), "UNKNOWN");
}
