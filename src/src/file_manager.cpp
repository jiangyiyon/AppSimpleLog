// File manager implementation

#include "speckit/log/file_manager.h"
#include <fstream>
#include <filesystem>
#include <format>
#include <algorithm>
#include <sstream>

#undef max
#undef min

#ifdef SPECKIT_PLATFORM_WINDOWS
#include <windows.h>
#include <stringapiset.h>
#endif
#ifndef SPECKIT_PLATFORM_WINDOWS
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

namespace fs = std::filesystem;

namespace {

// Helper function to convert UTF-8 string to wide string for Windows API
std::wstring Utf8ToWideString(const std::string& utf8_string) {
    if (utf8_string.empty()) {
        return std::wstring();
    }

    int wide_char_count = MultiByteToWideChar(CP_UTF8, 0, utf8_string.c_str(), -1, nullptr, 0);
    if (wide_char_count <= 0) {
        throw std::runtime_error("Failed to calculate wide character count");
    }

    std::wstring wide_string(wide_char_count, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8_string.c_str(), -1, wide_string.data(), wide_char_count);
    return wide_string;
}

// Helper function to get sequence number from historical log filename
int ExtractSequenceNumber(const std::string& filename) {
    // Expected format: base_name.N.log
    size_t last_dot_pos = filename.find_last_of('.');
    if (last_dot_pos == std::string::npos) {
        return -1;  // Invalid format
    }

    size_t second_last_dot_pos = filename.find_last_of('.', last_dot_pos - 1);
    if (second_last_dot_pos == std::string::npos) {
        return -1;  // Invalid format
    }

    std::string sequence_str = filename.substr(second_last_dot_pos + 1, 
                                              last_dot_pos - second_last_dot_pos - 1);
    try {
        return std::stoi(sequence_str);
    } catch (const std::exception&) {
        return -1;  // Not a valid number
    }
}

}  // namespace

FileManager::FileManager(const std::string& base_name)
    : base_name_(base_name),
      process_id_(0),
      current_file_name_(""),
      max_size_(10 * 1024 * 1024),  // 10MB default
      retention_count_(3),
      current_file_size_(0),
      initialized_(false),
      file_handle_(nullptr) {
#ifdef SPECKIT_PLATFORM_WINDOWS
    mutex_handle_ = nullptr;
#elif defined(SPECKIT_PLATFORM_MACOS)
    mutex_handle_ = nullptr;
#else
    mutex_handle_ = nullptr;
#endif
}

FileManager::~FileManager() {
    CloseCurrentFile();
    CleanupMutex();
}

void FileManager::CloseCurrentFile() {
    if (file_handle_) {
#ifdef SPECKIT_PLATFORM_WINDOWS
        fclose(file_handle_);
#else
        std::fclose(file_handle_);
#endif
        file_handle_ = nullptr;
    }
}

void FileManager::CleanupMutex() {
    if (mutex_handle_) {
#ifdef SPECKIT_PLATFORM_WINDOWS
        ReleaseMutex(mutex_handle_);
        CloseHandle(mutex_handle_);
        mutex_handle_ = nullptr;
#elif defined(SPECKIT_PLATFORM_MACOS)
        // On macOS, close and potentially unlink the semaphore
        sem_t* sem = static_cast<sem_t*>(mutex_handle_);
        sem_close(sem);
        // Note: We don't call sem_unlink here as other processes may still be using it
        mutex_handle_ = nullptr;
#endif
    }
}

bool FileManager::IsAnotherProcessLoggingPOSIX(const std::string& filename) {
#ifdef SPECKIT_PLATFORM_MACOS
    // On macOS, use named semaphores for process synchronization
    // Named semaphores on macOS need to be less than PSEMNAMLEN characters (typically 31)
    std::string semaphore_name = "/SLM_" + filename;  // Short prefix to save space
    
    if (semaphore_name.length() > 30) {
        // Truncate the name to fit within the limit and add hash
        std::string truncated_name = filename.substr(0, 15);
        std::string hash_part = std::to_string(std::hash<std::string>{}(filename) % 100000);
        semaphore_name = "/SLM_" + truncated_name + "_" + hash_part;
    }
    
    // Attempt to create a named semaphore exclusively
    sem_t* sem = sem_open(semaphore_name.c_str(), O_CREAT | O_EXCL, 0644, 1);
    
    if (sem == SEM_FAILED) {
        if (errno == EEXIST) {
            // Semaphore already exists, indicating another process may be logging
            // Try to acquire it briefly to determine if it's actually in use
            sem_t* existing_sem = sem_open(semaphore_name.c_str(), 0);
            if (existing_sem != SEM_FAILED) {
                // Attempt to acquire the semaphore (non-blocking)
                int result = sem_trywait(existing_sem);
                if (result == 0) {
                    // Successfully acquired, meaning previous holder released it
                    sem_post(existing_sem);  // Release immediately
                    sem_close(existing_sem);
                    // This is likely the first process to get the semaphore after the original process ended
                    // We'll create a new semaphore for this process
                    sem_t* new_sem = sem_open(semaphore_name.c_str(), O_CREAT | O_EXCL, 0644, 1);
                    if (new_sem != SEM_FAILED) {
                        mutex_handle_ = new_sem;
                        return false;  // No other process logging now
                    }
                    return true;  // Failed to create new semaphore
                } else {
                    // Could not acquire, another process is actively using it
                    sem_close(existing_sem);
                    return true;  // Another process is logging
                }
            }
            return true;  // Could not open existing semaphore
        }
        // Other error occurred
        return true;
    }
    
    // Successfully created the semaphore as the first process
    mutex_handle_ = sem;
    return false;
#else
    return false;  // Not implemented for non-macOS POSIX systems
#endif
}

bool FileManager::IsAnotherProcessLogging(const std::string& base_name) {
    // Extract just the base filename from the full path for consistent naming
    fs::path path_obj(base_name);
    std::string filename = path_obj.filename().string();
    
#ifdef SPECKIT_PLATFORM_WINDOWS
    // On Windows, create a global named mutex using the base filename only
    std::string mutex_name = "Global\\SpeckitLogMutex_" + filename;
    
    try {
        std::wstring wide_mutex_name = Utf8ToWideString(mutex_name);

        // Create the mutex
        HANDLE hMutex = CreateMutexW(NULL, FALSE, wide_mutex_name.c_str());
        
        if (hMutex == NULL) {
            // Could not create mutex due to error
            return true;
        }
        
        // Check if the mutex already existed (indicates another process is logging)
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            // Another process created this mutex first, so close our handle and return true
            CloseHandle(hMutex);
            return true;
        }
        
        // We successfully created the mutex as the first process
        // Store the handle for later cleanup
        mutex_handle_ = hMutex;
        return false;
    } catch (const std::exception&) {
        return true;  // Error during conversion
    }
#elif defined(SPECKIT_PLATFORM_MACOS) || defined(SPECKIT_PLATFORM_LINUX)
    // On POSIX systems (macOS/Linux), use named semaphores for process synchronization
    return IsAnotherProcessLoggingPOSIX(filename);
#else
    // Fallback for other platforms: check if base log file exists
    std::string first_process_file = base_name + ".log";
    return fs::exists(first_process_file);
#endif
}

bool FileManager::Initialize(ProcessIdType process_id) {
    if (initialized_) {
        return true;  // Already initialized
    }

    process_id_ = process_id;

    // Convert path to absolute path
    fs::path base_path(base_name_);
    if (!base_path.is_absolute()) {
        base_path = fs::absolute(base_path);
    }

    // Update base_name_ to absolute path
    base_name_ = base_path.string();

    // Ensure parent directory exists
    if (!EnsureParentDirectoryExists(base_path)) {
        return false;
    }

    // Use the mutex-based function to check if another process is already logging
    bool first_process = !IsAnotherProcessLogging(base_name_);

    // Generate file name based on whether we're first process
    current_file_name_ = GenerateFileName(first_process);

    // Open file for appending with Unicode support for Windows
    if (!OpenLogFile()) {
        return false;
    }

    // Get current file size if file exists
    UpdateCurrentFileSize();

    initialized_ = true;
    return true;
}

bool FileManager::EnsureParentDirectoryExists(const fs::path& base_path) {
    fs::path parent_dir = base_path.parent_path();
    if (!parent_dir.empty() && !fs::exists(parent_dir)) {
        std::error_code ec;
        fs::create_directories(parent_dir, ec);
        if (ec) {
            return false;  // Failed to create directory
        }
    }
    return true;
}

bool FileManager::OpenLogFile() {
#ifdef SPECKIT_PLATFORM_WINDOWS
    try {
        std::wstring wide_path = Utf8ToWideString(current_file_name_);
        errno_t err = _wfopen_s(&file_handle_, wide_path.c_str(), L"a");
        if (err != 0) {
            file_handle_ = nullptr;
            return false;
        }
    } catch (const std::exception&) {
        file_handle_ = nullptr;
        return false;
    }
#else
    file_handle_ = std::fopen(current_file_name_.c_str(), "a");
#endif

    return file_handle_ != nullptr;
}

void FileManager::UpdateCurrentFileSize() {
    if (fs::exists(current_file_name_)) {
        current_file_size_ = fs::file_size(current_file_name_);
    }
}

bool FileManager::Write(const std::string& data) {
    if (!file_handle_) {
        return false;
    }
    
    // Write data to file
    size_t written = std::fwrite(data.c_str(), sizeof(char), data.size(), file_handle_);
    
    if (written != data.size()) {
        return false;  // Write failed
    }
    
    current_file_size_ += data.size();
    
    // Flush to disk periodically
    if (ShouldFlushToFile()) {
        return FlushToFile();
    }
    
    return true;
}

bool FileManager::ShouldFlushToFile() const {
    // Flush every 1MB to prevent data loss
    return current_file_size_ % (1024 * 1024) == 0;
}

bool FileManager::FlushToFile() {
#ifdef SPECKIT_PLATFORM_WINDOWS
    return std::fflush(file_handle_) == 0;
#else
    if (std::fflush(file_handle_) != 0) return false;
    int fd = fileno(file_handle_);
    if (fd < 0) return false;
    return fsync(fd) == 0;
#endif
}

bool FileManager::NeedsRotation() const {
    return current_file_size_ >= max_size_;
}

bool FileManager::Rotate() {
    if (!file_handle_) {
        return false;
    }
    
    // Close current file
    CloseCurrentFile();
    
    // Get existing historical files
    auto historical_files = GetHistoricalFiles();
    
    // Determine next sequence number
    int next_sequence = CalculateNextSequenceNumber(historical_files);
    
    if (!RenameWithSequence(next_sequence)) {
        return false;  // Failed to rename
    }
    
    // Delete oldest files beyond retention limit
    DeleteOldestFiles();
    
    // Create new file with first process naming since we're rotating
    current_file_name_ = GenerateFileName(true);
    
    // Reopen the new file
    if (!OpenLogFile()) {
        return false;
    }
    
    current_file_size_ = 0;
    return true;
}

int FileManager::CalculateNextSequenceNumber(const std::vector<std::string>& historical_files) const {
    if (historical_files.empty()) {
        return 1;
    }
    
    // Find highest sequence number
    int max_sequence = 0;
    for (const auto& file : historical_files) {
        int sequence = ExtractSequenceNumber(file);
        if (sequence > max_sequence) {
            max_sequence = sequence;
        }
    }
    
    return max_sequence + 1;
}

void FileManager::DeleteOldestFiles() {
    auto historical_files = GetHistoricalFiles();
    
    // Delete files beyond retention limit
    while (historical_files.size() > retention_count_) {
        // Find oldest file (lowest sequence number)
        auto oldest_it = FindOldestHistoricalFile(historical_files);
        
        if (oldest_it != historical_files.end()) {
            std::error_code ec;
            fs::remove(*oldest_it, ec);
            historical_files.erase(oldest_it);
        }
    }
}

std::vector<std::string>::iterator FileManager::FindOldestHistoricalFile(
    std::vector<std::string>& historical_files) {
    return std::min_element(historical_files.begin(), historical_files.end(),
        [](const std::string& a, const std::string& b) {
            int seq_a = ExtractSequenceNumber(a);
            int seq_b = ExtractSequenceNumber(b);
            return seq_a < seq_b;
        });
}

void FileManager::SetMaxFileSize(size_t max_size) {
    max_size_ = max_size;
}

void FileManager::SetRetentionCount(size_t count) {
    retention_count_ = count;
}

std::string FileManager::GetLogFileName() const {
    return current_file_name_;
}

std::string FileManager::GenerateFileName(bool first_process) const {
    if (first_process) {
        // First process: base_name.log
        return base_name_ + ".log";
    } else {
        // Subsequent process: base_name_PID.log
        return std::format("{}_{}.log", base_name_, process_id_);
    }
}

bool FileManager::RenameWithSequence(int sequence) {
    std::string new_name = std::format("{}.{}.log", base_name_, sequence);
    
    std::error_code ec;
    fs::rename(current_file_name_, new_name, ec);
    
    return !ec;  // Return true if rename succeeded
}

std::vector<std::string> FileManager::GetHistoricalFiles() const {
    std::vector<std::string> files;
    
    // Pattern to match: base_name.N.log
    std::string base_pattern = base_name_ + ".";
    
    // Iterate through current directory
    for (const auto& entry : fs::directory_iterator(fs::current_path())) {
        std::string filename = entry.path().filename().string();
        
        // Check if file matches pattern (base_name.N.log)
        if (IsHistoricalLogFile(filename, base_pattern)) {
            files.push_back(entry.path().string());
        }
    }
    
    return files;
}

bool FileManager::IsHistoricalLogFile(const std::string& filename, 
                                     const std::string& base_pattern) const {
    // Check if filename starts with base pattern and ends with .log
    if (filename.find(base_pattern) != 0 || 
        filename.find(".log") != filename.length() - 4) {
        return false;
    }
    
    // Additional validation: ensure there's a number between the base pattern and .log
    std::string remaining = filename.substr(base_pattern.length());
    size_t dot_pos = remaining.find('.');
    if (dot_pos == std::string::npos) {
        return false;  // No sequence number
    }
    
    std::string sequence_str = remaining.substr(0, dot_pos);
    if (sequence_str.empty()) {
        return false;  // Empty sequence
    }
    
    // Verify sequence is numeric
    for (char c : sequence_str) {
        if (!std::isdigit(c)) {
            return false;
        }
    }
    
    return true;
}

bool FileManager::Flush() {
    if (!file_handle_) return false;
    return FlushToFile();
}