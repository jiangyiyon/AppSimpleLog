// Crash handler implementation

#include "speckit/log/crash_handler.h"
#include "speckit/log/platform.h"
#include <csignal>
#include <iostream>



namespace {

// Signals we handle. Kept in an array for iteration.
constexpr int kHandledSignals[] = {
#ifdef SPECKIT_PLATFORM_WINDOWS
    SIGSEGV,
    SIGABRT,
#else
    SIGSEGV,
    SIGABRT,
#endif
};

}  // namespace

// Static members
CrashHandler::FlushCallback CrashHandler::flush_callback_ = nullptr;
std::atomic<bool> CrashHandler::initialized_{false};
std::atomic<bool> CrashHandler::flush_requested_{false};

CrashHandler::CrashHandler() { initializeHandlers(); }

CrashHandler::~CrashHandler() { cleanupHandlers(); }

void CrashHandler::startMonitor() {
    stop_monitor_.store(false);
    monitor_thread_ = std::thread([this]() {
        while (!stop_monitor_.load()) {
            if (flush_requested_.exchange(false)) {
                // Perform emergency flush outside of signal context
                emergencyFlush();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
}

void CrashHandler::stopMonitor() {
    stop_monitor_.store(true);
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }
}

void CrashHandler::setFlushCallback(FlushCallback callback) { flush_callback_ = std::move(callback); }

void CrashHandler::emergencyFlush() {
    // Execute user-provided flush callback. Do not allow exceptions to escape.
    if (flush_callback_) {
        try {
            flush_callback_();
        } catch (...) {
            // Swallow all exceptions in emergency path.
        }
    }
}

void CrashHandler::signalHandler(int signal) {
    // Prevent recursive handling for the same signal.
    std::signal(signal, SIG_IGN);

    // Signal the background monitor to perform the flush. This is async-signal-safe
    // because it uses an atomic flag.
    flush_requested_.store(true);

    // Restore default handler and re-raise the signal so process terminates as expected.
    std::signal(signal, SIG_DFL);
    std::raise(signal);
}

void CrashHandler::initializeHandlers() {
    bool expected = false;
    if (!initialized_.compare_exchange_strong(expected, true)) {
        return;  // Already initialized.
    }

#ifdef SPECKIT_PLATFORM_WINDOWS
    // Use simple signal() API on Windows.
    for (int sig : kHandledSignals) {
        std::signal(sig, signalHandler);
    }
    // Start background monitor thread
    startMonitor();
#else
    struct sigaction sa{};
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;

    for (int sig : kHandledSignals) {
        sigaction(sig, &sa, nullptr);
    }
#endif
}

void CrashHandler::cleanupHandlers() {
    bool expected = true;
    if (!initialized_.compare_exchange_strong(expected, false)) {
        return;  // Not initialized.
    }

#ifdef SPECKIT_PLATFORM_WINDOWS
    for (int sig : kHandledSignals) {
        std::signal(sig, SIG_DFL);
    }
    // Stop background monitor thread
    stopMonitor();
#else
    struct sigaction sa{};
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;

    for (int sig : kHandledSignals) {
        sigaction(sig, &sa, nullptr);
    }
#endif
}


