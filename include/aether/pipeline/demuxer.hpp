// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/pipeline/demuxer.hpp
// DESCRIPTION: Demuxer interface
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_PIPELINE_DEMUXER_HPP
#define AETHER_PIPELINE_DEMUXER_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"
#include "aether/media/packet.hpp"
#include "aether/media/format.hpp"

#include <memory>
#include <string>

namespace aether {

// Forward declaration
class MediaSource;

/**
 * @struct DemuxerInfo
 * @brief Demuxer capabilities
 */
struct AETHER_API DemuxerInfo {
    std::string name;
    std::string long_name;
    std::vector<std::string> extensions;
    std::vector<std::string> mime_types;
    bool supports_seek = true;
};

/**
 * @class Demuxer
 * @brief Abstract demuxer interface
 */
class AETHER_API Demuxer {
public:
    virtual ~Demuxer() = default;

    /**
     * @brief Open demuxer with source
     */
    virtual Result<void> Open(Shared<MediaSource> source) = 0;

    /**
     * @brief Close demuxer
     */
    virtual void Close() = 0;

    /**
     * @brief Get format info
     */
    [[nodiscard]] virtual FormatInfo GetFormatInfo() const = 0;

    /**
     * @brief Read next packet
     */
    virtual Result<PacketPtr> ReadPacket() = 0;

    /**
     * @brief Seek to position
     */
    virtual Result<void> Seek(i64 pts, i32 stream_index = -1) = 0;

    /**
     * @brief Get duration
     */
    [[nodiscard]] virtual i64 GetDuration() const = 0;

    /**
     * @brief Get bitrate
     */
    [[nodiscard]] virtual i64 GetBitrate() const = 0;

    /**
     * @brief Get demuxer info
     */
    [[nodiscard]] virtual DemuxerInfo GetInfo() const = 0;
};

/**
 * @brief Create FFmpeg-based demuxer
 */
AETHER_API std::unique_ptr<Demuxer> CreateFFmpegDemuxer();

/**
 * @brief Create demuxer by format
 */
AETHER_API std::unique_ptr<Demuxer> CreateDemuxer(ContainerFormat format);

/**
 * @brief Create demuxer by name
 */
AETHER_API std::unique_ptr<Demuxer> CreateDemuxerByName(const std::string& name);

} // namespace aether

#endif // AETHER_PIPELINE_DEMUXER_HPP
