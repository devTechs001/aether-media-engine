// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/media/format.hpp
// DESCRIPTION: Media format detection
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_MEDIA_FORMAT_HPP
#define AETHER_MEDIA_FORMAT_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <string>
#include <vector>

namespace aether {

/**
 * @struct FormatInfo
 * @brief Media format information
 */
struct AETHER_API FormatInfo {
    ContainerFormat container = ContainerFormat::Unknown;
    std::string format_name;
    std::string format_long_name;
    std::string mime_type;

    // File extensions
    std::vector<std::string> extensions;

    // Stream info
    struct StreamInfo {
        MediaType type = MediaType::Unknown;
        CodecID codec_id = CodecID::Unknown;
        std::string codec_name;
        i32 index = -1;

        // Video specific
        SizeU resolution;
        f64 fps = 0.0;
        PixelFormat pixel_format = PixelFormat::Unknown;

        // Audio specific
        u32 sample_rate = 0;
        u32 channels = 0;
        SampleFormat sample_format = SampleFormat::Unknown;
        u64 channel_layout = 0;

        // Bitrate
        i64 bitrate = 0;
    };

    std::vector<StreamInfo> streams;

    // Duration
    i64 duration_ms = 0;

    // Bitrate
    i64 bitrate = 0;

    // Metadata
    std::string title;
    std::string artist;
    std::string album;
    std::string year;
    std::string genre;
    std::string comment;

    // Chapters
    struct Chapter {
        i64 start_ms = 0;
        i64 end_ms = 0;
        std::string title;
    };
    std::vector<Chapter> chapters;
};

/**
 * @class FormatDetector
 * @brief Detect media format from data
 */
class AETHER_API FormatDetector {
public:
    /**
     * @brief Detect format from file path
     * @param path File path
     * @return Format info or error
     */
    static Result<FormatInfo> DetectFromFile(const std::string& path);

    /**
     * @brief Detect format from data buffer
     * @param data Data buffer
     * @return Format info or error
     */
    static Result<FormatInfo> DetectFromData(std::span<const u8> data);

    /**
     * @brief Get format from extension
     * @param extension File extension (e.g., ".mp4")
     * @return Container format
     */
    static ContainerFormat FromExtension(const std::string& extension);

    /**
     * @brief Get common extensions for format
     * @param format Container format
     * @return List of extensions
     */
    static std::vector<std::string> GetExtensions(ContainerFormat format);

    /**
     * @brief Check if format is video container
     */
    static bool IsVideoContainer(ContainerFormat format);

    /**
     * @brief Check if format is audio container
     */
    static bool IsAudioContainer(ContainerFormat format);
};

} // namespace aether

#endif // AETHER_MEDIA_FORMAT_HPP
