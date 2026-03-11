// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/network/streaming.hpp
// DESCRIPTION: Streaming interface
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_NETWORK_STREAMING_HPP
#define AETHER_NETWORK_STREAMING_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <string>
#include <memory>
#include <functional>

namespace aether {

/**
 * @enum StreamType
 * @brief Type of stream
 */
enum class StreamType : u8 {
    DASH,
    HLS,
    SmoothStreaming,
    RTSP,
    RTMP,
    SRT,
    WebRTC
};

/**
 * @struct StreamInfo
 * @brief Stream information
 */
struct AETHER_API StreamInfo {
    StreamType type = StreamType::HLS;
    std::string url;
    bool is_live = false;
    
    // Quality levels
    struct QualityLevel {
        u32 bitrate = 0;
        u32 width = 0;
        u32 height = 0;
        std::string codec;
        std::string manifest_url;
    };
    std::vector<QualityLevel> quality_levels;
    
    // Current quality
    u32 current_bitrate = 0;
    
    // DVR
    bool dvr_enabled = false;
    i64 dvr_window_ms = 0;
};

/**
 * @class StreamManager
 * @brief Manage media streaming
 */
class AETHER_API StreamManager {
public:
    /**
     * @brief Get singleton instance
     */
    static StreamManager& Instance();

    /**
     * @brief Open stream
     */
    Result<void> OpenStream(const std::string& url);

    /**
     * @brief Close stream
     */
    void CloseStream();

    /**
     * @brief Get stream info
     */
    [[nodiscard]] StreamInfo GetStreamInfo() const;

    /**
     * @brief Switch quality
     */
    Result<void> SwitchQuality(u32 bitrate);

    /**
     * @brief Get available bitrates
     */
    [[nodiscard]] std::vector<u32> GetAvailableBitrates() const;

    /**
     * @brief Get current bandwidth
     */
    [[nodiscard]] f64 GetCurrentBandwidth() const;

private:
    StreamManager() = default;
    ~StreamManager() = default;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

/**
 * @brief Parse manifest URL
 */
AETHER_API Result<StreamInfo> ParseManifest(const std::string& url);

/**
 * @brief Detect stream type from URL
 */
AETHER_API StreamType DetectStreamType(const std::string& url);

} // namespace aether

#endif // AETHER_NETWORK_STREAMING_HPP
