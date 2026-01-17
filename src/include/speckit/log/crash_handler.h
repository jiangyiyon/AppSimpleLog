// Crash handler for emergency log flush on program crash
// Registers signal handlers and triggers emergency writes

#pragma once

#include <functional>
#include <atomic>
#include <string>



/// Crash handler for emergency log flush
/// Registers signal handlers and triggers callback on crash
class CrashHandler {
public:
    /// Type for emergency flush callback
    using FlushCallback = std::function<void()>;

    /// Constructor
    CrashHandler();
    
    /// Destructor - unregisters signal handlers
    ~CrashHandler();
    
    /// Set the emergency flush callback
    /// @param callback Function to call on crash
    void setFlushCallback(FlushCallback callback);
    
    /// Trigger emergency flush manually
    static void emergencyFlush();

private:
    /// Static crash signal handler
    static void signalHandler(int signal);

    /// Initialize signal handlers
    void initializeHandlers();

    /// Cleanup signal handlers
    void cleanupHandlers();

    static FlushCallback flush_callback_;          ///< Emergency flush callback
    static std::atomic<bool> initialized_;        ///< Handler initialization state
};


