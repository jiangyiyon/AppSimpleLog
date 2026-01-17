// File manager implementation

#include "speckit/log/file_manager.h"
#include <fstream>
#include <filesystem>
#include <format>
#include <algorithm>

#undef max
#undef min

#ifdef SPECKIT_PLATFORM_WINDOWS
#include <windows.h>
#include <stringapiset.h>
#endif

namespace fs = std::filesystem;

FileManager::FileManager(const std::string& base_name)
    : base_name_(base_name), file_handle_(nullptr), mutex_handle_(nullptr) {
}

FileManager::~FileManager() {
    if (file_handle_) {
#ifdef SPECKIT_PLATFORM_WINDOWS
        fclose(file_handle_);
#else
        std::fclose(file_handle_);
#endif
        file_handle_ = nullptr;
    }
    
    // Clean up mutex handle if it exists
    if (mutex_handle_) {
#ifdef SPECKIT_PLATFORM_WINDOWS
        ReleaseMutex(mutex_handle_);
        CloseHandle(mutex_handle_);
        mutex_handle_ = nullptr;
#elif defined(SPECKIT_PLATFORM_MACOS)
        // On macOS, close and potentially unlink the semaphore
        sem_t *sem = static_cast<sem_t*>(mutex_handle_);
        sem_close(sem);
        // Note: We don't call sem_unlink here as other processes may still be using it
        mutex_handle_ = nullptr;
#endif
    }
}

bool FileManager::isAnotherProcessLoggingPOSIX(const std::string& filename) {
#ifdef SPECKIT_PLATFORM_MACOS
    // On macOS, use named semaphores for process synchronization
    // Named semaphores on macOS need to be less than PSEMNAMLEN characters (typically 31)
    std::string semaphore_name = "/SLM_" + filename;  // Short prefix to save space
    
    if(semaphore_name.length() > 30) {
        // Truncate the name to fit within the limit and add hash
        std::string truncated_name = filename.substr(0, 15);
        std::string hash_part = std::to_string(std::hash<std::string>{}(filename) % 100000);
        semaphore_name = "/SLM_" + truncated_name + "_" + hash_part;
    }
    
    // Attempt to create a named semaphore exclusively
    sem_t *sem = sem_open(semaphore_name.c_str(), O_CREAT | O_EXCL, 0644, 1);
    
    if (sem == SEM_FAILED) {
        if (errno == EEXIST) {
            // Semaphore already exists, indicating another process may be logging
            // Try to acquire it briefly to determine if it's actually in use
            sem_t *existing_sem = sem_open(semaphore_name.c_str(), 0);
            if (existing_sem != SEM_FAILED) {
                // Attempt to acquire the semaphore (non-blocking)
                int result = sem_trywait(existing_sem);
                if (result == 0) {
                    // Successfully acquired, meaning previous holder released it
                    sem_post(existing_sem); // Release immediately
                    sem_close(existing_sem);
                    // This is likely the first process to get the semaphore after the original process ended
                    // We'll create a new semaphore for this process
                    sem_t *new_sem = sem_open(semaphore_name.c_str(), O_CREAT | O_EXCL, 0644, 1);
                    if (new_sem != SEM_FAILED) {
                        mutex_handle_ = new_sem;
                        return false; // No other process logging now
                    }
                    return true; // Failed to create new semaphore
                } else {
                    // Could not acquire, another process is actively using it
                    sem_close(existing_sem);
                    return true; // Another process is logging
                }
            }
            return true; // Could not open existing semaphore
        }
        // Other error occurred
        return true;
    }
    
    // Successfully created the semaphore as the first process
    mutex_handle_ = sem;
    return false;
#endif
    return false;
}

bool FileManager::isAnotherProcessLogging(const std::string& base_name) {
    // Extract just the base filename from the full path for consistent naming
    fs::path path_obj(base_name);
    std::string filename = path_obj.filename().string();
    
#ifdef SPECKIT_PLATFORM_WINDOWS
    // On Windows, create a global named mutex using the base filename only
    std::string mutex_name = "Global\\SpeckitLogMutex_" + filename;
    
    // Convert to wide string for Windows API
    int wide_char_count = MultiByteToWideChar(CP_UTF8, 0, mutex_name.c_str(), -1, nullptr, 0);
    if (wide_char_count <= 0) {
        return true; // Error in conversion, assume another process is logging
    }

    std::wstring wide_mutex_name(wide_char_count, 0);
    MultiByteToWideChar(CP_UTF8, 0, mutex_name.c_str(), -1, wide_mutex_name.data(), wide_char_count);

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
#elif defined(SPECKIT_PLATFORM_MACOS) || defined(SPECKIT_PLATFORM_LINUX)
    // On POSIX systems (macOS/Linux), use named semaphores for process synchronization
    return isAnotherProcessLoggingPOSIX(filename);
#else
    // Fallback for other platforms: check if base log file exists
    std::string first_process_file = base_name + ".log";
    return fs::exists(first_process_file);
#endif
}

bool FileManager::initialize(ProcessIdType process_id) {
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
    fs::path parent_dir = base_path.parent_path();
    if (!parent_dir.empty() && !fs::exists(parent_dir)) {
        std::error_code ec;
        fs::create_directories(parent_dir, ec);
        if (ec) {
            return false;  // Failed to create directory
        }
    }

    // Use the mutex-based function to check if another process is already logging
    bool first_process = !isAnotherProcessLogging(base_name_);

    // Generate file name based on whether we're first process
    current_file_name_ = generateFileName(first_process);

    // Open file for appending with Unicode support for Windows
#ifdef SPECKIT_PLATFORM_WINDOWS
    // Convert UTF-8 path to UTF-16 for Windows API
    int wide_char_count = MultiByteToWideChar(CP_UTF8, 0, current_file_name_.c_str(), -1, nullptr, 0);
    if (wide_char_count <= 0) {
        return false;  // Conversion failed
    }

    std::wstring wide_path(wide_char_count, 0);
    MultiByteToWideChar(CP_UTF8, 0, current_file_name_.c_str(), -1, wide_path.data(), wide_char_count);

    errno_t err = _wfopen_s(&file_handle_, wide_path.c_str(), L"a");
    if (err != 0) {
        file_handle_ = nullptr;
        return false;
    }
#else
    file_handle_ = std::fopen(current_file_name_.c_str(), "a");
#endif

    if (!file_handle_) {
        return false;  // Failed to open file
    }

    // Get current file size if file exists
    if (fs::exists(current_file_name_)) {
        current_file_size_ = fs::file_size(current_file_name_);
    }

    initialized_ = true;
    return true;
}

bool FileManager::write(const std::string& data) {
    if (!file_handle_) {
        return false;
    }
    
    // Write data to file
#ifdef SPECKIT_PLATFORM_WINDOWS
    size_t written = std::fwrite(data.c_str(), sizeof(char), data.size(), file_handle_);
#else
    size_t written = std::fwrite(data.c_str(), sizeof(char), data.size(), file_handle_);
#endif
    
    if (written != data.size()) {
        return false;  // Write failed
    }
    
    current_file_size_ += data.size();
    
    // Flush to disk periodically
    if (current_file_size_ % (1024 * 1024) == 0) {  // Flush every 1MB
#ifdef SPECKIT_PLATFORM_WINDOWS
        std::fflush(file_handle_);
#else
        std::fflush(file_handle_);
#endif
    }
    
    return true;
}

bool FileManager::needsRotation() const {
    return current_file_size_ >= max_size_;
}

bool FileManager::rotate() {
    if (!file_handle_) {
        return false;
    }
    
    // Close current file
#ifdef SPECKIT_PLATFORM_WINDOWS
    std::fclose(file_handle_);
#else
    std::fclose(file_handle_);
#endif
    file_handle_ = nullptr;
    
    // Get existing historical files
    auto historical_files = getHistoricalFiles();
    
    // Rename current file with appropriate sequence number
    int sequence = 1;
    if (!historical_files.empty()) {
        // Find highest sequence number
        for (const auto& file : historical_files) {
            // Extract sequence from filename (A.1.log, A.2.log, etc.)
            size_t pos = file.find_last_of('.');
            size_t prev_pos = file.find_last_of('.', pos - 1);
            if (prev_pos != std::string::npos && pos != std::string::npos) {
                int seq = std::stoi(file.substr(prev_pos + 1, pos - prev_pos - 1));
                sequence = std::max(sequence, seq + 1);
            }
        }
    }
    
    if (!renameWithSequence(sequence)) {
        return false;  // Failed to rename
    }
    
    // Delete oldest files beyond retention limit
    deleteOldestFiles();
    
    // Create new file
    bool first_process = current_file_name_.find('_') == std::string::npos;
    current_file_name_ = generateFileName(first_process);
    
#ifdef SPECKIT_PLATFORM_WINDOWS
    // Convert UTF-8 path to UTF-16 for Windows API to support Chinese paths
    int wide_char_count = MultiByteToWideChar(CP_UTF8, 0, current_file_name_.c_str(), -1, nullptr, 0);
    if (wide_char_count <= 0) {
        return false;
    }
    std::wstring wide_new_path(wide_char_count, 0);
    MultiByteToWideChar(CP_UTF8, 0, current_file_name_.c_str(), -1, wide_new_path.data(), wide_char_count);

    errno_t err = _wfopen_s(&file_handle_, wide_new_path.c_str(), L"w");
    if (err != 0) {
        file_handle_ = nullptr;
        return false;
    }
#else
    file_handle_ = std::fopen(current_file_name_.c_str(), "w");
#endif
    
    if (!file_handle_) {
        return false;
    }
    
    current_file_size_ = 0;
    return true;
}

void FileManager::setMaxFileSize(size_t max_size) {
    max_size_ = max_size;
}

void FileManager::setRetentionCount(size_t count) {
    retention_count_ = count;
}

std::string FileManager::getLogFileName() const {
    return current_file_name_;
}

std::string FileManager::generateFileName(bool first_process) const {
    if (first_process) {
        // First process: base_name.log
        return base_name_ + ".log";
    } else {
        // Subsequent process: base_name_PID.log
        return std::format("{}_{}.log", base_name_, process_id_);
    }
}

bool FileManager::renameWithSequence(int sequence) {
    std::string new_name = std::format("{}.{}.log", base_name_, sequence);
    
    std::error_code ec;
    fs::rename(current_file_name_, new_name, ec);
    
    return !ec;  // Return true if rename succeeded
}

void FileManager::deleteOldestFiles() {
    auto historical_files = getHistoricalFiles();
    
    // Delete files beyond retention limit
    while (historical_files.size() > retention_count_) {
        // Find oldest file (lowest sequence number)
        auto oldest_it = std::min_element(historical_files.begin(), historical_files.end(),
            [](const std::string& a, const std::string& b) {
                // Extract and compare sequence numbers
                size_t pos_a = a.find_last_of('.');
                size_t pos_b = b.find_last_of('.');
                size_t prev_a = a.find_last_of('.', pos_a - 1);
                size_t prev_b = b.find_last_of('.', pos_b - 1);
                int seq_a = std::stoi(a.substr(prev_a + 1, pos_a - prev_a - 1));
                int seq_b = std::stoi(b.substr(prev_b + 1, pos_b - prev_b - 1));
                return seq_a < seq_b;
            });
        
        if (oldest_it != historical_files.end()) {
            std::error_code ec;
            fs::remove(*oldest_it, ec);
            historical_files.erase(oldest_it);
        }
    }
}

std::vector<std::string> FileManager::getHistoricalFiles() const {
    std::vector<std::string> files;
    
    // Get base name without extension
    std::string base_pattern = base_name_ + ".";
    
    // Iterate through directory
    for (const auto& entry : fs::directory_iterator(fs::current_path())) {
        std::string filename = entry.path().filename().string();
        
        // Check if file matches pattern (base_name.N.log)
        if (filename.find(base_pattern) == 0 && 
            filename.find(".log") == filename.length() - 4) {
            files.push_back(entry.path().string());
        }
    }
    
    return files;
}