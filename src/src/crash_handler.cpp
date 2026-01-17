// Crash handler implementation

#include "speckit/log/crash_handler.h"
#include "speckit/log/platform.h"
#include <csignal>
#include <iostream>



// Static members
CrashHandler::FlushCallback CrashHandler::flush_callback_ = nullptr;
std::atomic<bool> CrashHandler::initialized_(false);

CrashHandler::CrashHandler() {
    initializeHandlers();
}

CrashHandler::~CrashHandler() {
    cleanupHandlers();
}

void CrashHandler::setFlushCallback(FlushCallback callback) {
    flush_callback_ = callback;
}

void CrashHandler::emergencyFlush() {
    if (flush_callback_) {
        try {
            flush_callback_();
        } catch (...) {
            // Ignore exceptions during crash handling
        }
    }
}

void CrashHandler::signalHandler(int signal) {
    // Ignore further signals
    std::signal(signal, SIG_IGN);
    
    std::cerr << "Crash signal received: " << signal << std::endl;
    std::cerr << "Attempting emergency flush..." << std::endl;
    
    // Trigger emergency flush
    emergencyFlush();
    
    // Re-raise signal to allow default handler to run
    std::signal(signal, SIG_DFL);
    std::raise(signal);
}

void CrashHandler::initializeHandlers() {
    if (initialized_.exchange(true)) {
        return;  // Already initialized
    }
    
    // Register signal handlers for crash scenarios
#ifdef SPECKIT_PLATFORM_WINDOWS
    signal(SIGSEGV, signalHandler);
    signal(SIGABRT, signalHandler);
#else
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
#endif
    
    initialized_.store(true);
}

void CrashHandler::cleanupHandlers() {
    if (!initialized_.exchange(false)) {
        return;  // Not initialized
    }
    
    // Restore default signal handlers
#ifdef SPECKIT_PLATFORM_WINDOWS
    signal(SIGSEGV, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
#else
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
#endif
}


