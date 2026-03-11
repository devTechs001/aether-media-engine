// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/utils/logging.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/utils/logging.hpp"
#include <iostream>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

namespace aether {

// ═══════════════════════════════════════════════════════════════════════════════
// Logger Implementation
// ═══════════════════════════════════════════════════════════════════════════════

class Logger::Impl {
public:
    std::string name;
    LogLevel level = LogLevel::Info;
    bool log_to_console = true;
    bool log_to_file = false;
    std::string log_file_path;
    std::ofstream log_file;
    std::mutex mutex;
    bool initialized = false;
};

Logger::Logger() : impl_(std::make_unique<Impl>()) {
    impl_->name = "aether";
}

Logger::~Logger() {
    Shutdown();
}

Result<void> Logger::Initialize(const std::string& log_file, LogLevel level) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    impl_->level = level;
    impl_->initialized = true;

    if (!log_file.empty()) {
        impl_->log_file_path = log_file;
        impl_->log_file.open(log_file, std::ios::app);
        if (!impl_->log_file.is_open()) {
            return std::unexpected(Error::Make(
                ErrorCode::FileWriteError,
                "Failed to open log file: " + log_file
            ));
        }
        impl_->log_to_file = true;
    }

    return {};
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (impl_->initialized) {
        if (impl_->log_file.is_open()) {
            impl_->log_file.close();
        }
        impl_->initialized = false;
    }
}

void Logger::SetLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->level = level;
}

LogLevel Logger::GetLevel() const {
    return impl_->level;
}

void Logger::Log(LogLevel level, std::string_view message,
                 const std::source_location& loc) {
    if (static_cast<u8>(level) < static_cast<u8>(impl_->level)) {
        return;
    }

    std::lock_guard<std::mutex> lock(impl_->mutex);

    // Format timestamp
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::ostringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    // Level string
    const char* level_str = "";
    switch (level) {
        case LogLevel::Trace:    level_str = "TRACE"; break;
        case LogLevel::Debug:    level_str = "DEBUG"; break;
        case LogLevel::Info:     level_str = "INFO"; break;
        case LogLevel::Warning:  level_str = "WARN"; break;
        case LogLevel::Error:    level_str = "ERROR"; break;
        case LogLevel::Critical: level_str = "CRIT"; break;
    }

    // Thread ID
    auto thread_id = std::this_thread::get_id();

    // Format log entry
    std::string log_line;
    log_line = ss.str();
    log_line += " [" + std::string(level_str) + "]";
    log_line += " [thread " + std::to_string(std::hash<std::thread::id>{}(thread_id)) + "]";
    log_line += " " + std::string(message);

    if (impl_->log_to_console) {
        // Color output for console
        const char* color = "";
        const char* reset = "\033[0m";

#ifdef __linux__
        switch (level) {
            case LogLevel::Trace:    color = "\033[90m"; break;
            case LogLevel::Debug:    color = "\033[36m"; break;
            case LogLevel::Info:     color = "\033[32m"; break;
            case LogLevel::Warning:  color = "\033[33m"; break;
            case LogLevel::Error:    color = "\033[31m"; break;
            case LogLevel::Critical: color = "\033[35;1m"; break;
        }
        std::cout << color << log_line << reset << std::endl;
#else
        std::cout << log_line << std::endl;
#endif
    }

    if (impl_->log_to_file && impl_->log_file.is_open()) {
        impl_->log_file << log_line << std::endl;
        impl_->log_file.flush();
    }
}

Logger& Logger::Instance() {
    static Logger instance;
    return instance;
}

Logger& GetLogger() {
    return Logger::Instance();
}

} // namespace aether
