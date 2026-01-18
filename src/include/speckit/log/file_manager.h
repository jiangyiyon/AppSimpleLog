// File manager for log file operations
// Handles file creation, writing, rotation, and retention

#pragma once

#include <string>
#include <vector>
#include <memory>
#include "platform.h"

#ifdef SPECKIT_PLATFORM_WINDOWS
#include <windows.h>
#elif defined(SPECKIT_PLATFORM_MACOS)
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#else
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#endif

#include <filesystem>

/// File manager for log file operations
/// Thread-safe file I/O with rotation support
class FileManager {
public:
    /// Constructor
    /// @param base_name Base name for log files (e.g., "MyApp")
    explicit FileManager(const std::string& base_name);

    /// Destructor - ensures file is closed
    ~FileManager();

    /// Initialize the file manager
    /// @param process_id Process ID for unique file naming
    /// @return true if successful
    bool Initialize(ProcessIdType process_id);

    /// Write a string to the log file
    /// @param data String to write
    /// @return true if successful
    bool Write(const std::string& data);

    /// Check if current file exceeds maximum size
    /// @return true if file size >= max_size_
    bool NeedsRotation() const;

    /// Rotate log files (close current, rename, create new)
    /// @return true if successful
    bool Rotate();

    /// Set maximum file size before rotation
    /// @param max_size Maximum size in bytes
    void SetMaxFileSize(size_t max_size);

    /// Set maximum number of historical files to retain
    /// @param count Maximum number of historical files
    void SetRetentionCount(size_t count);

    /// Get current log file name
    /// @return Full path to current log file
    std::string GetLogFileName() const;

    /// Force flush pending writes to disk
    /// @return true if successful
    bool Flush();

private:
    /// Generate log file name based on process
    /// @param first_process Is this the first process instance?
    /// @return Log file name
    std::string GenerateFileName(bool first_process) const;

    /// Check if another process is already using this log file using mutex
    /// @param base_name Base name for log files
    /// @return true if another process is already logging
    bool IsAnotherProcessLogging(const std::string& base_name);

    bool IsAnotherProcessLoggingPOSIX(const std::string& filename);

    /// Rename current file with sequence number
    /// @param sequence Sequence number for rotated file
    /// @return true if successful
    bool RenameWithSequence(int sequence);

    /// Delete oldest historical files beyond retention limit
    void DeleteOldestFiles();

    /// Get list of historical log files
    /// @return Vector of historical file names sorted by sequence
    std::vector<std::string> GetHistoricalFiles() const;

    /// Close the current log file if open
    void CloseCurrentFile();

    /// Clean up mutex resources
    void CleanupMutex();

    /// Ensure parent directory exists for the log file
    /// @param base_path Path to the log file
    /// @return true if directory exists or was created successfully
    bool EnsureParentDirectoryExists(const std::filesystem::path& base_path);

    /// Open the log file for writing/appending
    /// @return true if file opened successfully
    bool OpenLogFile();

    /// Update the current file size member variable
    void UpdateCurrentFileSize();

    /// Determine if file should be flushed to disk based on size
    /// @return true if flush is needed
    bool ShouldFlushToFile() const;

    /// Actually flush the file to disk
    /// @return true if flush was successful
    bool FlushToFile();

    /// Calculate the next sequence number for rotated files
    /// @param historical_files List of existing historical files
    /// @return Next sequence number to use
    int CalculateNextSequenceNumber(const std::vector<std::string>& historical_files) const;

    /// Find the oldest historical file iterator in the vector
    /// @param historical_files List of historical files
    /// @return Iterator to the oldest file
    std::vector<std::string>::iterator FindOldestHistoricalFile(
        std::vector<std::string>& historical_files);

    /// Check if a filename matches the historical log file pattern
    /// @param filename The filename to check
    /// @param base_pattern The base pattern to match against
    /// @return true if it's a valid historical log file
    bool IsHistoricalLogFile(const std::string& filename,
        const std::string& base_pattern) const;

    std::string base_name_;              ///< Base name for log files
    ProcessIdType process_id_;           ///< Process ID
    std::string current_file_name_;      ///< Current log file name
    size_t max_size_ = 10 * 1024 * 1024;  ///< Default 10MB
    size_t retention_count_ = 3;        ///< Default retain 3 historical files
    size_t current_file_size_ = 0;      ///< Current file size
    bool initialized_ = false;          ///< Initialization state
    FILE* file_handle_ = nullptr;       ///< File handle
#ifdef SPECKIT_PLATFORM_WINDOWS
    HANDLE mutex_handle_;                ///< Handle to the mutex for process synchronization
#elif defined(SPECKIT_PLATFORM_MACOS)
    void* mutex_handle_;                 ///< Pointer to semaphore for macOS process synchronization
#else
    void* mutex_handle_;                 ///< Placeholder for POSIX mutex mechanism
#endif
};