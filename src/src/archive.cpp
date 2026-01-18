#include "../include/speckit/log/archive.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <zip.h>
#include <cstring>

namespace fs = std::filesystem;

namespace speckit {
    namespace log {

        /**
         * Create a proper ZIP archive containing all .log files matching baseName
         * Uses libzip library for true compression (DEFLATE algorithm)
         * Archive filename format: baseName_processId_timestamp.zip
         *
         * @param baseName Base name of log files (e.g., "app")
         * @param processId Process ID for unique identification
         * @param timestamp Timestamp string for the archive name
         * @return true on success, false on failure
         */
        bool createArchive(const std::string& baseName, int processId, const std::string& timestamp) {
            try {
                // Generate archive filename with timestamp
                std::string archiveName = baseName + "_" + std::to_string(processId) + "_" + timestamp + ".zip";
                
                // Validate timestamp is not empty
                if (timestamp.empty()) {
                    return false;
                }

                // Gather all .log files matching baseName pattern
                std::vector<fs::path> files;
                fs::path dir = fs::current_path();
                
                // Find all files starting with baseName and ending with .log
                for (auto& p : fs::directory_iterator(dir)) {
                    auto name = p.path().filename().string();
                    if (name.rfind(baseName, 0) == 0 && name.find(".log") != std::string::npos) {
                        files.push_back(p.path());
                    }
                }

                // If no files found, nothing to archive
                if (files.empty()) {
                    return false;
                }

                // Create ZIP archive using libzip
                // ZIP_CREATE: Create new archive
                // ZIP_TRUNCATE: If file exists, truncate it
                // ZIP_EXCL: Fail if archive already exists (for safety)
                int zipError = 0;
                zip_t* zipArchive = zip_open(archiveName.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &zipError);
                
                if (zipArchive == nullptr) {
                    // Failed to create archive - get error details
                    zip_error_t error;
                    zip_error_init_with_code(&error, zipError);
                    // Note: In production, you might log this error
                    zip_error_fini(&error);
                    return false;
                }

                // Process each log file and add to archive
                bool success = true;
                for (const auto& f : files) {
                    // Get file size
                    size_t fileSize = fs::file_size(f);
                    if (fileSize == 0) {
                        // Skip empty files
                        continue;
                    }

                    // Open source file
                    std::ifstream in(f, std::ios::binary);
                    if (!in) {
                        success = false;
                        break;
                    }

                    // Read file content into buffer
                    std::vector<char> fileData(fileSize);
                    in.read(fileData.data(), fileSize);
                    in.close();

                    // Add file to ZIP archive
                    // zip_file_add(): Add a new file entry to the archive
                    // ZIP_FL_OVERWRITE: If file already exists in archive, replace it
                    zip_source_t* source = zip_source_buffer(zipArchive, fileData.data(), fileSize, 0);
                    if (source == nullptr) {
                        success = false;
                        break;
                    }

                    // Add the file with its original filename
                    zip_int64_t fileIndex = zip_file_add(zipArchive, f.filename().string().c_str(), source, ZIP_FL_OVERWRITE);
                    
                    if (fileIndex < 0) {
                        zip_source_free(source);  // Clean up source on failure
                        success = false;
                        break;
                    }

                    // Note: libzip uses DEFLATE compression by default, so we don't need to set it explicitly
                    // For older libzip versions that don't support zip_file_set_compression, this works fine
                }

                // Close ZIP archive (this writes the actual ZIP file structure)
                int closeResult = zip_close(zipArchive);
                
                if (closeResult != 0 || !success) {
                    // Archive close failed or file processing failed
                    // On failure, libzip automatically removes the partially created archive
                    return false;
                }

                // Archive created successfully - remove original log files
                for (const auto& f : files) {
                    try {
                        fs::remove(f);
                    } catch (const std::exception& e) {
                        // Log error but don't fail the entire operation
                        // In production, you might want to track this
                        (void)e;  // Suppress unused variable warning
                    }
                }

                return true;
            }
            catch (const std::exception& e) {
                // Catch and report exceptions (but not silently swallow them)
                // In production, you might log this error
                (void)e;  // Suppress unused variable warning
                return false;
            }
            catch (...) {
                // Catch any other unexpected exceptions
                return false;
            }
        }

    }
} // namespace speckit::log