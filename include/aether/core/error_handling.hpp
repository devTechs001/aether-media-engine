// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/core/error_handling.hpp
// DESCRIPTION: Error handling utilities
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_CORE_ERROR_HANDLING_HPP
#define AETHER_CORE_ERROR_HANDLING_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <string>
#include <source_location>
#include <exception>

namespace aether {

/**
 * @brief Get string description of an error code
 */
AETHER_API std::string GetErrorDescription(ErrorCode code);

/**
 * @brief Get error category name
 */
AETHER_API std::string GetErrorCategory(ErrorCode code);

/**
 * @brief Check if error is recoverable
 */
AETHER_API bool IsRecoverableError(ErrorCode code);

/**
 * @brief Exception class for AETHER errors
 */
class AETHER_API AetherException : public std::exception {
public:
    explicit AetherException(Error error)
        : m_error(std::move(error)) {}

    AetherException(ErrorCode code, const std::string& message,
                   const std::source_location& loc = std::source_location::current())
        : m_error(Error{code, message, "", loc.file_name(), static_cast<i32>(loc.line())}) {}

    [[nodiscard]] const char* what() const noexcept override {
        return m_error.message.c_str();
    }

    [[nodiscard]] const Error& GetError() const noexcept {
        return m_error;
    }

    [[nodiscard]] ErrorCode GetCode() const noexcept {
        return m_error.code;
    }

private:
    Error m_error;
};

/**
 * @brief Create an error with source location
 */
inline Error MakeError(ErrorCode code, const std::string& message,
                      const std::source_location& loc = std::source_location::current()) {
    return Error{code, message, "", loc.file_name(), static_cast<i32>(loc.line())};
}

/**
 * @brief Create a result with error
 */
template<typename T>
inline Result<T> MakeErrorResult(ErrorCode code, const std::string& message,
                                 const std::source_location& loc = std::source_location::current()) {
    return std::unexpected(MakeError(code, message, loc));
}

// Error handling macros
#define AETHER_RETURN_IF_ERROR(expr) \
    do { \
        auto _result = (expr); \
        if (!_result) { \
            return _result; \
        } \
    } while (false)

#define AETHER_THROW_IF_ERROR(expr) \
    do { \
        auto _result = (expr); \
        if (!_result) { \
            throw AetherException(_result.error()); \
        } \
    } while (false)

#define AETHER_CHECK_NOTNULL(ptr, msg) \
    do { \
        if ((ptr) == nullptr) { \
            return MakeErrorResult<std::decay_t<decltype(*(ptr))>>( \
                ErrorCode::InvalidArgument, msg); \
        } \
    } while (false)

} // namespace aether

#endif // AETHER_CORE_ERROR_HANDLING_HPP
