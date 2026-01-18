#include "../include/speckit/log/archive.h"

#include <filesystem>
#include <vector>
#include <string>
#include <zip.h>
#include <system_error>

namespace fs = std::filesystem;

namespace speckit {
namespace log {

namespace {  // internal helpers

// Collect all regular files in `dir` that have extension ".log" and whose
// filename (without directory) starts with the provided base filename.
std::vector<fs::path> CollectLogFiles(const fs::path& dir, const std::string& base_name) {
    std::vector<fs::path> result;
    const std::string base_fname = fs::path(base_name).filename().string();
    try {
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            const fs::path& p = entry.path();
            if (p.extension() != ".log") {
                continue;
            }
            const std::string fname = p.filename().string();
            // Match files whose filename starts with base filename.
            if (fname.rfind(base_fname, 0) == 0) {
                result.push_back(p);
            }
        }
    }
    catch (const std::exception&) {
        // On error (permission, etc.) return what we have (possibly empty).
    }
    return result;
}

// (Removed) ReadFileToBuffer: we now use libzip file-backed sources to avoid
// loading file contents into memory.

// RAII wrapper for libzip archive handle: discard on destruction unless closed.
struct ZipGuard {
    explicit ZipGuard(zip_t* z) noexcept : zip(z), closed(false) {}
    ~ZipGuard() noexcept {
        if (zip == nullptr) return;
        if (!closed) {
            zip_discard(zip);
        }
    }
    // Close (write) the archive. Returns zip_close() result or -1 if no zip.
    int Close() noexcept {
        if (zip == nullptr) return -1;
        const int res = zip_close(zip);
        closed = (res == 0);
        return res;
    }
    zip_t* zip;
private:
    bool closed;
};

}  // namespace (helpers)

bool createArchive(const std::string& base_name, int process_id, const std::string& timestamp) {
    // Validate inputs early.
    if (timestamp.empty() || base_name.empty()) {
        return false;
    }

    // Build archive filename.
    const std::string archive_name = base_name + "_" + std::to_string(process_id) + "_" + timestamp + ".zip";

    // Collect candidate log files from current directory that start with base_name.
    const fs::path cwd = fs::current_path();
    const std::vector<fs::path> candidates = CollectLogFiles(cwd, base_name);
    if (candidates.empty()) {
        return false;
    }

    // Open ZIP archive. Create/truncate existing archive.
    int zip_error = 0;
    zip_t* raw_zip = zip_open(archive_name.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &zip_error);
    if (raw_zip == nullptr) {
        // Creation failed (could inspect zip_error for logging).
        return false;
    }
    ZipGuard zip(raw_zip);

    bool had_nonempty = false;
    for (const auto& f : candidates) {
        std::error_code ec;
        const auto sz = fs::file_size(f, ec);
        if (ec) {
            // Skip unreadable file.
            continue;
        }
        if (sz == 0) {
            // Ignore empty files per tests/requirements.
            continue;
        }

        // Use libzip's file-backed source to avoid loading entire file into memory.
        // zip_source_file(zip_t*, const char* path, zip_uint64_t start, zip_int64_t len)
        // Passing 0 and -1 reads the whole file.
        zip_source_t* source = zip_source_file(zip.zip, f.string().c_str(), 0, -1);
        if (source == nullptr) {
            return false;
        }

        const std::string filename_in_zip = f.filename().string();
        zip_int64_t idx = zip_file_add(zip.zip, filename_in_zip.c_str(), source, ZIP_FL_OVERWRITE);
        if (idx < 0) {
            zip_source_free(source);
            return false;
        }

        had_nonempty = true;
    }

    if (!had_nonempty) {
        // No non-empty files to archive -> do not create archive.
        return false;
    }

    // Close (write) the archive. If this fails, ZipGuard will discard in destructor.
    if (zip.Close() != 0) {
        return false;
    }

    // NOTE: 按用户要求，打包完成后不要删除原始文件。保留原文件供后续分析/调试。

    return true;
}

}  // namespace log
}  // namespace speckit