// Log entry formatting implementation

#include "speckit/log/log_entry.h"
#include "speckit/log/log_level.h"
#include <chrono>
#include <ctime>
#include <format>
#include <string>
#include <thread>
#include <sstream>



std::string formatLogEntry(const LogEntry& entry) {
    // Format: [LEVEL] YYYY-MM-DD HH:MM:SS.mmm [PID, TID] [TAG]: Message

    // Convert timestamp to local time
    auto ms_since_epoch = entry.timestamp_ms;
    auto seconds = ms_since_epoch / 1000;
    auto milliseconds = ms_since_epoch % 1000;

    std::time_t time_t_value = static_cast<std::time_t>(seconds);
    std::tm tm_value;
#ifdef SPECKIT_PLATFORM_WINDOWS
    localtime_s(&tm_value, &time_t_value);
#else
    localtime_r(&time_t_value, &tm_value);
#endif

    // Format timestamp: YYYY-MM-DD HH:MM:SS.mmm
    std::string timestamp = std::format(
        "{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}",
        tm_value.tm_year + 1900,
        tm_value.tm_mon + 1,
        tm_value.tm_mday,
        tm_value.tm_hour,
        tm_value.tm_min,
        tm_value.tm_sec,
        milliseconds
    );

    // Convert thread_id to string
    std::string thread_id_str;
#ifdef SPECKIT_PLATFORM_WINDOWS
    thread_id_str = std::to_string(entry.thread_id);
#else
    std::ostringstream oss;
    oss << entry.thread_id;
    thread_id_str = oss.str();
#endif

    // Format: [LEVEL] timestamp [PID, TID] [TAG]: Message
    return std::format(
        "[{}] {} [{}, {}] [{}]: {}\n",
        levelToString(entry.level),
        timestamp,
        entry.process_id,
        thread_id_str,
        entry.tag.data(),
        entry.message.data()
    );
}

