// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/media/codec.hpp
// DESCRIPTION: Codec information
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_MEDIA_CODEC_HPP
#define AETHER_MEDIA_CODEC_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <string>
#include <vector>

namespace aether {

/**
 * @enum CodecType
 * @brief Type of codec
 */
enum class CodecType : u8 {
    Video,
    Audio,
    Subtitle,
    Data
};

/**
 * @struct CodecInfo
 * @brief Information about a codec
 */
struct AETHER_API CodecInfo {
    CodecID id = CodecID::Unknown;
    CodecType type = CodecType::Video;
    std::string name;
    std::string long_name;
    const char* mime_type = nullptr;

    // Capabilities
    bool is_encoder = false;
    bool is_decoder = false;
    bool is_hardware = false;
    bool is_lossy = false;
    bool is_lossless = false;

    // Supported formats
    std::vector<PixelFormat> video_formats;
    std::vector<SampleFormat> audio_formats;

    // Limits
    u32 max_width = 0;
    u32 max_height = 0;
    u32 max_sample_rate = 0;
    u32 max_channels = 0;
};

/**
 * @class CodecRegistry
 * @brief Registry of available codecs
 */
class AETHER_API CodecRegistry {
public:
    /**
     * @brief Get codec by ID
     */
    static const CodecInfo* GetCodec(CodecID id);

    /**
     * @brief Get codec by name
     */
    static const CodecInfo* GetCodecByName(const std::string& name);

    /**
     * @brief Get all video codecs
     */
    static std::vector<const CodecInfo*> GetVideoCodecs();

    /**
     * @brief Get all audio codecs
     */
    static std::vector<const CodecInfo*> GetAudioCodecs();

    /**
     * @brief Get all hardware codecs
     */
    static std::vector<const CodecInfo*> GetHardwareCodecs();

    /**
     * @brief Check if codec is available
     */
    static bool IsCodecAvailable(CodecID id);

    /**
     * @brief Get codec description
     */
    static std::string GetCodecDescription(CodecID id);
};

} // namespace aether

#endif // AETHER_MEDIA_CODEC_HPP
