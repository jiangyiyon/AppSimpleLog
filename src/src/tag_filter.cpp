#include "../include/speckit/log/tag_filter.h"

using namespace speckit::log;

TagFilter::TagFilter() {}

void TagFilter::setTagEnabled(const std::string& tag, bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    enabled_[tag] = enabled;
}

bool TagFilter::isTagEnabled(const std::string& tag) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = enabled_.find(tag);
    if (it == enabled_.end()) return true; // default enabled
    return it->second;
}

void TagFilter::setTagLevel(const std::string& tag, LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    levels_[tag] = level;
}

LogLevel TagFilter::getTagLevel(const std::string& tag) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = levels_.find(tag);
    if (it == levels_.end()) return LogLevel::kLogLevelDebug;
    return it->second;
}
