// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/utils/logging.hpp
// DESCRIPTION: Logging utilities
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_UTILS_LOGGING_HPP
#define AETHER_UTILS_LOGGING_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <string>
#include <string_view>
#include <source_location>

namespace aether {

/**
 * @enum LogLevel
 * @brief Log severity levels
 */
enum class LogLevel : u8 {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Critical = 5,
    Off = 6
};

/**
 * @class Logger
 * @brief Logging system
 */
class AETHER_API Logger {
public:
    /**
     * @brief Get singleton instance
     */
    static Logger& Instance();

    /**
     * @brief Initialize logger
     */
    Result<void> Initialize(const std::string& log_file = "", LogLevel level = LogLevel::Info);

    /**
     * @brief Shutdown logger
     */
    void Shutdown();

    /**
     * @brief Set log level
     */
    void SetLevel(LogLevel level);

    /**
     * @brief Get current log level
     */
    [[nodiscard]] LogLevel GetLevel() const;

    /**
     * @brief Log message
     */
    void Log(LogLevel level, std::string_view message,
             const std::source_location& loc = std::source_location::current());

    /**
     * @brief Log formatted message
     */
    template<typename... Args>
    void LogFormatted(LogLevel level, std::string_view format, Args&&... args);

    /**
     * @brief Add log sink
     */
    using LogSink = std::function<void(LogLevel, std::string_view)>;
    void AddSink(LogSink sink);

    /**
     * @brief Remove log sink
     */
    void RemoveSink(LogSink sink);

private:
    Logger() = default;
    ~Logger() = default;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

// Convenience macros
#define AETHER_LOG_TRACE(msg) ::aether::Logger::Instance().Log(::aether::LogLevel::Trace, msg, std::source_location::current())
#define AETHER_LOG_DEBUG(msg) ::aether::Logger::Instance().Log(::aether::LogLevel::Debug, msg, std::source_location::current())
#define AETHER_LOG_INFO(msg) ::aether::Logger::Instance().Log(::aether::LogLevel::Info, msg, std::source_location::current())
#define AETHER_LOG_WARNING(msg) ::aether::Logger::Instance().Log(::aether::LogLevel::Warning, msg, std::source_location::current())
#define AETHER_LOG_ERROR(msg) ::aether::Logger::Instance().Log(::aether::LogLevel::Error, msg, std::source_location::current())
#define AETHER_LOG_CRITICAL(msg) ::aether::Logger::Instance().Log(::aether::LogLevel::Critical, msg, std::source_location::current())

// Get logger
AETHER_API Logger& GetLogger();

} // namespace aether

#endif // AETHER_UTILS_LOGGING_HPP
