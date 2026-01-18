#include "speckit_logger.h"
#include "speckit/log/file_manager.h"
#include "speckit/log/tag_filter.h"
#include "speckit/log/archive.h"

#include <memory>
#include <string>
#include <chrono>
#include <sstream>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

struct SpeckitLoggerImpl {
    std::unique_ptr<FileManager> fm;
    std::unique_ptr<speckit::log::TagFilter> tagFilter;
    speckit::log::LogLevel level = speckit::log::LogLevel::kLogLevelDebug;
    std::string baseName;
    int processId = 0;
};

extern "C" {

SpeckitLogger* speckit_logger_create(const char* config) {
    if (!config) return nullptr;
    auto impl = new SpeckitLoggerImpl();
    impl->baseName = std::string(config);

#ifdef _WIN32
    impl->processId = static_cast<int>(GetCurrentProcessId());
#else
    impl->processId = static_cast<int>(getpid());
#endif

    impl->fm = std::make_unique<FileManager>(impl->baseName);
    impl->fm->Initialize(impl->processId);
    impl->tagFilter = std::make_unique<speckit::log::TagFilter>();

    return reinterpret_cast<SpeckitLogger*>(impl);
}

SpeckitErrorCode speckit_logger_log(SpeckitLogger* logger, int level,
                                     const char* tag, const char* message) {
    if (!logger || !message) return SPECKIT_ERROR_NULL_POINTER;
    auto impl = reinterpret_cast<SpeckitLoggerImpl*>(logger);

    // Tag filtering
    std::string tagStr = tag ? tag : "";
    if (!impl->tagFilter->isTagEnabled(tagStr)) return SPECKIT_SUCCESS;

    // Level filtering per tag
    speckit::log::LogLevel tagLevel = impl->tagFilter->getTagLevel(tagStr);
    speckit::log::LogLevel msgLevel = static_cast<speckit::log::LogLevel>(level);
    if (static_cast<int>(msgLevel) < static_cast<int>(tagLevel)) return SPECKIT_SUCCESS;

    // Basic formatting: [LEVEL] message\n
    std::string out = message;
    out.push_back('\n');

    bool ok = impl->fm->Write(out);
    if (!ok) return SPECKIT_ERROR_FILE_IO;

    if (impl->fm->NeedsRotation()) {
        impl->fm->Rotate();
    }

    return SPECKIT_SUCCESS;
}

SpeckitErrorCode speckit_logger_set_log_level(SpeckitLogger* logger, int level) {
    if (!logger) return SPECKIT_ERROR_NULL_POINTER;
    auto impl = reinterpret_cast<SpeckitLoggerImpl*>(logger);
    impl->level = static_cast<speckit::log::LogLevel>(level);
    return SPECKIT_SUCCESS;
}

SpeckitErrorCode speckit_logger_destroy(SpeckitLogger* logger) {
    if (!logger) return SPECKIT_ERROR_NULL_POINTER;
    auto impl = reinterpret_cast<SpeckitLoggerImpl*>(logger);
    // flush and cleanup handled in FileManager destructor
    delete impl;
    return SPECKIT_SUCCESS;
}

// P2/P3 helpers
void speckit_logger_set_max_file_size(unsigned long long bytes) {
    // global setter for default file manager (not instance-specific)
    (void)bytes; // stub - instance-level API preferred
}

void speckit_logger_set_retention_count(int count) {
    (void)count; // stub
}

void speckit_logger_set_tag_enabled(const char* tag, int enabled) {
    (void)tag; (void)enabled; // stubs: per-instance API exists via speckit_logger_create
}

void speckit_logger_set_tag_level(const char* tag, int level) {
    (void)tag; (void)level;
}

int speckit_logger_archive(const char* baseName) {
    if (!baseName) return -1;
#ifdef _WIN32
    int pid = static_cast<int>(GetCurrentProcessId());
#else
    int pid = static_cast<int>(getpid());
#endif
    // timestamp simple
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y%m%d%H%M%S", &tm);

    bool ok = speckit::log::createArchive(baseName, pid, buf);
    return ok ? 0 : -1;
}

void speckit_logger_set_auto_archive_config(int enabled) {
    (void)enabled; // stub
}

} // extern "C"
