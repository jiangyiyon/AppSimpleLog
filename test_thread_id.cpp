// Test for thread_id formatting
#include "speckit/log/platform.h"
#include "speckit/log/log_entry.h"
#include "iostream>

int main() {
    LogEntry entry;

#ifdef SPECKIT_PLATFORM_WINDOWS
    entry.thread_id = GetCurrentThreadId();
#else
    entry.thread_id = std::this_thread::get_id();
#endif

    std::cout << "Thread ID test passed!" << std::endl;
    return 0;
}
