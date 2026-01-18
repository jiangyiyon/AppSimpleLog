#pragma once

#include <string>

namespace speckit {
namespace log {

// Create an archive (ZIP) for log files whose filenames start with `base_name`.
// Returns true on success.
bool createArchive(const std::string& base_name, int process_id, const std::string& timestamp);

}  // namespace log
}  // namespace speckit