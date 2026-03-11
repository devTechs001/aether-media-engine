// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/network/protocol.hpp
// DESCRIPTION: Protocol handlers
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_NETWORK_PROTOCOL_HPP
#define AETHER_NETWORK_PROTOCOL_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <string>
#include <memory>
#include <span>
#include <unordered_map>

namespace aether {

/**
 * @enum ProtocolType
 * @brief Protocol types
 */
enum class ProtocolType : u8 {
    File,
    HTTP,
    HTTPS,
    FTP,
    RTSP,
    RTMP,
    SRT,
    RTP,
    UDP,
    TCP
};

/**
 * @class ProtocolHandler
 * @brief Protocol handler interface
 */
class AETHER_API ProtocolHandler {
public:
    virtual ~ProtocolHandler() = default;

    /**
     * @brief Open connection
     */
    virtual Result<void> Open(const std::string& url) = 0;

    /**
     * @brief Close connection
     */
    virtual void Close() = 0;

    /**
     * @brief Read data
     */
    virtual Result<usize> Read(void* buffer, usize size) = 0;

    /**
     * @brief Write data
     */
    virtual Result<usize> Write(const void* buffer, usize size) = 0;

    /**
     * @brief Seek to position
     */
    virtual Result<void> Seek(i64 position) = 0;

    /**
     * @brief Get current position
     */
    [[nodiscard]] virtual i64 Tell() const = 0;

    /**
     * @brief Get size
     */
    [[nodiscard]] virtual i64 Size() const = 0;

    /**
     * @brief Get protocol type
     */
    [[nodiscard]] virtual ProtocolType GetType() const = 0;
};

/**
 * @brief Create protocol handler
 */
AETHER_API std::unique_ptr<ProtocolHandler> CreateProtocolHandler(ProtocolType type);

/**
 * @brief Create protocol handler from URL
 */
AETHER_API std::unique_ptr<ProtocolHandler> CreateProtocolHandlerForURL(const std::string& url);

/**
 * @brief Get protocol type from URL
 */
AETHER_API ProtocolType GetProtocolTypeFromURL(const std::string& url);

/**
 * @brief Register custom protocol handler
 */
AETHER_API void RegisterProtocolHandler(const std::string& scheme, 
                                        std::function<std::unique_ptr<ProtocolHandler>()> factory);

} // namespace aether

#endif // AETHER_NETWORK_PROTOCOL_HPP
