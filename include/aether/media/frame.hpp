// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/media/frame.hpp
// DESCRIPTION: Media frame types
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_MEDIA_FRAME_HPP
#define AETHER_MEDIA_FRAME_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <memory>
#include <span>
#include <array>

namespace aether {

/**
 * @struct VideoFrame
 * @brief Video frame data
 */
struct AETHER_API VideoFrame {
    SizeU size;
    PixelFormat format = PixelFormat::Unknown;
    i64 pts = 0;
    i64 dts = 0;
    i64 duration = 0;
    bool is_keyframe = false;

    // Planar data (up to 4 planes for YUV444P16, etc.)
    std::array<u8*, 4> data{};
    std::array<i32, 4> linesize{};

    /**
     * @brief Get plane count for format
     */
    [[nodiscard]] int GetPlaneCount() const;

    /**
     * @brief Get bytes per pixel
     */
    [[nodiscard]] int GetBytesPerPixel() const;
};

/**
 * @struct AudioFrame
 * @brief Audio frame data
 */
struct AETHER_API AudioFrame {
    SampleFormat format = SampleFormat::Unknown;
    ChannelLayout channel_layout = ChannelLayout::Unknown;
    u32 sample_rate = 0;
    u32 nb_samples = 0;
    i64 pts = 0;
    i64 dts = 0;
    i64 duration = 0;

    // Audio data pointer
    u8* data = nullptr;
    i32 linesize = 0;

    /**
     * @brief Get channel count
     */
    [[nodiscard]] int GetChannelCount() const;

    /**
     * @brief Get bytes per sample
     */
    [[nodiscard]] int GetBytesPerSample() const;
};

/**
 * @struct SubtitleFrame
 * @brief Subtitle frame data
 */
struct AETHER_API SubtitleFrame {
    enum class Type : u8 {
        Text,
        Bitmap
    };

    Type type = Type::Text;
    i64 pts = 0;
    i64 dts = 0;
    i64 duration = 0;
    i64 end_pts = 0;

    // Text subtitle
    std::string text;
    std::string ass;  // ASS formatted text

    // Bitmap subtitle
    SizeU size;
    std::span<u8> bitmap_data;
};

/**
 * @class Frame
 * @brief Unified media frame class
 */
class AETHER_API Frame {
public:
    enum class Type : u8 {
        Video,
        Audio,
        Subtitle,
        Data
    };

    Frame();
    ~Frame();

    // Move operations
    Frame(Frame&& other) noexcept;
    Frame& operator=(Frame&& other) noexcept;

    // Copy operations (deleted for efficiency)
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;

    [[nodiscard]] Type GetType() const { return m_type; }
    [[nodiscard]] bool IsValid() const { return m_valid; }

    [[nodiscard]] VideoFrame& AsVideo();
    [[nodiscard]] const VideoFrame& AsVideo() const;

    [[nodiscard]] AudioFrame& AsAudio();
    [[nodiscard]] const AudioFrame& AsAudio() const;

    [[nodiscard]] SubtitleFrame& AsSubtitle();
    [[nodiscard]] const SubtitleFrame& AsSubtitle() const;

    /**
     * @brief Create video frame
     */
    static Frame CreateVideo(const SizeU& size, PixelFormat format);

    /**
     * @brief Create audio frame
     */
    static Frame CreateAudio(SampleFormat format, ChannelLayout layout, 
                            u32 sample_rate, u32 nb_samples);

    /**
     * @brief Create subtitle frame
     */
    static Frame CreateSubtitle(const std::string& text, i64 duration);

private:
    Type m_type = Type::Video;
    bool m_valid = false;
    
    union {
        VideoFrame m_video;
        AudioFrame m_audio;
        SubtitleFrame m_subtitle;
    };
};

using FramePtr = std::unique_ptr<Frame>;

} // namespace aether

#endif // AETHER_MEDIA_FRAME_HPP
