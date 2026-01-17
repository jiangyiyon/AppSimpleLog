#pragma once

#include "log_level.h"
#include <string>
#include <unordered_map>
#include <mutex>

namespace speckit { namespace log {

class TagFilter {
public:
    TagFilter();

    void setTagEnabled(const std::string& tag, bool enabled);
    bool isTagEnabled(const std::string& tag) const;

    void setTagLevel(const std::string& tag, LogLevel level);
    LogLevel getTagLevel(const std::string& tag) const;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, bool> enabled_;
    std::unordered_map<std::string, LogLevel> levels_;
};

}} // namespace speckit::log
