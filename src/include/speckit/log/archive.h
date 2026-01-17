#pragma once

#include <string>

namespace speckit { namespace log {

// Create an archive (ZIP) for baseName log files. Returns true on success.
bool createArchive(const std::string& baseName, int processId, const std::string& timestamp);

}} // namespace speckit::log
