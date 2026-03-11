// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/core/types.hpp
// DESCRIPTION: Core type definitions for AETHER Media Engine
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_CORE_TYPES_HPP
#define AETHER_CORE_TYPES_HPP

#include <cstdint>
#include <chrono>
#include <string>
#include <string_view>
#include <memory>
#include <optional>
#include <variant>
#include <functional>
#include <span>
#include <expected>

namespace aether {

// ═══════════════════════════════════════════════════════════════════════════════
// Basic Types
// ═══════════════════════════════════════════════════════════════════════════════

using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using f32 = float;
using f64 = double;

using usize = std::size_t;
using isize = std::ptrdiff_t;

// ═══════════════════════════════════════════════════════════════════════════════
// Time Types
// ═══════════════════════════════════════════════════════════════════════════════

using Clock = std::chrono::high_resolution_clock;
using TimePoint = Clock::time_point;
using Duration = Clock::duration;
using Nanoseconds = std::chrono::nanoseconds;
using Microseconds = std::chrono::microseconds;
using Milliseconds = std::chrono::milliseconds;
using Seconds = std::chrono::seconds;

/**
 * @struct Timestamp
 * @brief Represents a media timestamp with timebase
 */
struct Timestamp {
    i64 pts = 0;           // Presentation timestamp
    i64 dts = 0;           // Decoding timestamp
    i64 duration = 0;      // Duration

    // Timebase: pts = seconds * num / den
    i32 timebase_num = 1;
    i32 timebase_den = 1000000;  // Default: microseconds

    // Convert to seconds
    [[nodiscard]] constexpr f64 ToSeconds() const noexcept {
        return static_cast<f64>(pts) * timebase_num / timebase_den;
    }

    // Convert to milliseconds
    [[nodiscard]] constexpr i64 ToMilliseconds() const noexcept {
        return pts * timebase_num * 1000 / timebase_den;
    }

    // Create from seconds
    static constexpr Timestamp FromSeconds(f64 seconds) noexcept {
        Timestamp ts;
        ts.pts = static_cast<i64>(seconds * 1000000);
        ts.timebase_num = 1;
        ts.timebase_den = 1000000;
        return ts;
    }

    // Comparison operators
    auto operator<=>(const Timestamp& other) const {
        return ToSeconds() <=> other.ToSeconds();
    }
    bool operator==(const Timestamp& other) const {
        return ToSeconds() == other.ToSeconds();
    }
};

// ═══════════════════════════════════════════════════════════════════════════════
// Geometry Types
// ═══════════════════════════════════════════════════════════════════════════════

template<typename T>
struct Size2D {
    T width = 0;
    T height = 0;

    [[nodiscard]] constexpr T Area() const noexcept { return width * height; }
    [[nodiscard]] constexpr f64 AspectRatio() const noexcept {
        return height != 0 ? static_cast<f64>(width) / height : 0.0;
    }

    bool operator==(const Size2D&) const = default;
    auto operator<=>(const Size2D&) const = default;
};

template<typename T>
struct Point2D {
    T x = 0;
    T y = 0;

    bool operator==(const Point2D&) const = default;
};

template<typename T>
struct Rect {
    T x = 0;
    T y = 0;
    T width = 0;
    T height = 0;

    [[nodiscard]] constexpr T Left() const noexcept { return x; }
    [[nodiscard]] constexpr T Top() const noexcept { return y; }
    [[nodiscard]] constexpr T Right() const noexcept { return x + width; }
    [[nodiscard]] constexpr T Bottom() const noexcept { return y + height; }
    [[nodiscard]] constexpr T Area() const noexcept { return width * height; }

    [[nodiscard]] constexpr Point2D<T> TopLeft() const noexcept { return {x, y}; }
    [[nodiscard]] constexpr Point2D<T> BottomRight() const noexcept { return {x + width, y + height}; }
    [[nodiscard]] constexpr Size2D<T> Size() const noexcept { return {width, height}; }

    [[nodiscard]] constexpr bool Contains(Point2D<T> point) const noexcept {
        return point.x >= x && point.x < Right() && point.y >= y && point.y < Bottom();
    }

    bool operator==(const Rect&) const = default;
};

using Size = Size2D<i32>;
using SizeU = Size2D<u32>;
using SizeF = Size2D<f32>;
using Point = Point2D<i32>;
using PointF = Point2D<f32>;
using Rectangle = Rect<i32>;
using RectangleF = Rect<f32>;

// ═══════════════════════════════════════════════════════════════════════════════
// Color Types
// ═══════════════════════════════════════════════════════════════════════════════

struct Color {
    f32 r = 0.0f;
    f32 g = 0.0f;
    f32 b = 0.0f;
    f32 a = 1.0f;

    static constexpr Color Black() { return {0, 0, 0, 1}; }
    static constexpr Color White() { return {1, 1, 1, 1}; }
    static constexpr Color Red() { return {1, 0, 0, 1}; }
    static constexpr Color Green() { return {0, 1, 0, 1}; }
    static constexpr Color Blue() { return {0, 0, 1, 1}; }
    static constexpr Color Transparent() { return {0, 0, 0, 0}; }

    [[nodiscard]] u32 ToRGBA8() const noexcept {
        return (static_cast<u32>(r * 255) << 24) |
               (static_cast<u32>(g * 255) << 16) |
               (static_cast<u32>(b * 255) << 8) |
               static_cast<u32>(a * 255);
    }

    static constexpr Color FromRGBA8(u32 rgba) noexcept {
        return {
            static_cast<f32>((rgba >> 24) & 0xFF) / 255.0f,
            static_cast<f32>((rgba >> 16) & 0xFF) / 255.0f,
            static_cast<f32>((rgba >> 8) & 0xFF) / 255.0f,
            static_cast<f32>(rgba & 0xFF) / 255.0f
        };
    }
};

// ═══════════════════════════════════════════════════════════════════════════════
// Media Types Enumerations
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @enum MediaType
 * @brief Type of media content
 */
enum class MediaType : u8 {
    Unknown = 0,
    Video,
    Audio,
    Subtitle,
    Data,
    Attachment
};

/**
 * @enum PixelFormat
 * @brief Video pixel formats
 */
enum class PixelFormat : u32 {
    Unknown = 0,

    // Packed RGB formats
    RGB24,
    BGR24,
    RGBA32,
    BGRA32,
    ARGB32,
    ABGR32,
    RGB565,
    RGB555,
    RGB48,
    RGBA64,

    // Planar YUV formats
    YUV420P,      // I420
    YUV422P,
    YUV444P,
    YUV420P10,    // 10-bit
    YUV420P12,    // 12-bit
    YUV420P16,    // 16-bit
    YUV422P10,
    YUV444P10,
    YUV444P16,

    // Semi-planar formats (NV12, P010, etc.)
    NV12,         // 4:2:0 semi-planar
    NV21,
    P010,         // 10-bit semi-planar
    P016,         // 16-bit semi-planar

    // Hardware formats
    D3D11,
    DXVA2,
    VAAPI,
    VDPAU,
    VideoToolbox,
    MediaCodec,
    CUDA,
    QSV,
    Vulkan,

    // HDR formats
    YUV420P10_HDR10,
    YUV420P10_HLG,
    YUV420P10_DolbyVision,

    // Other
    Gray8,
    Gray16,
    GrayF32
};

/**
 * @enum SampleFormat
 * @brief Audio sample formats
 */
enum class SampleFormat : u8 {
    Unknown = 0,

    // Integer formats
    U8,           // Unsigned 8-bit
    S16,          // Signed 16-bit
    S32,          // Signed 32-bit
    S64,          // Signed 64-bit

    // Floating point formats
    F32,          // 32-bit float
    F64,          // 64-bit double

    // Planar versions
    U8P,
    S16P,
    S32P,
    F32P,
    F64P
};

/**
 * @enum ChannelLayout
 * @brief Audio channel layouts
 */
enum class ChannelLayout : u32 {
    Unknown = 0,
    Mono = 1,
    Stereo = 2,
    Surround21 = 3,
    Surround30 = 4,
    Surround31 = 5,
    Quad = 6,
    Surround50 = 7,
    Surround51 = 8,
    Surround61 = 9,
    Surround71 = 10,
    Surround714 = 11,   // Dolby Atmos 7.1.4
    Surround916 = 12,   // 9.1.6 immersive
};

/**
 * @enum ColorSpace
 * @brief Video color spaces
 */
enum class ColorSpace : u8 {
    Unknown = 0,
    BT601,
    BT709,
    BT2020,
    SMPTE170M,
    SMPTE240M,
    BT2100_PQ,    // HDR10
    BT2100_HLG,   // HLG
    SRGB,
    AdobeRGB,
    DCI_P3,
    DisplayP3
};

/**
 * @enum ColorRange
 * @brief Video color range
 */
enum class ColorRange : u8 {
    Unknown = 0,
    Limited,      // 16-235 (TV)
    Full          // 0-255 (PC)
};

/**
 * @enum HDRType
 * @brief HDR format types
 */
enum class HDRType : u8 {
    None = 0,
    HDR10,
    HDR10Plus,
    DolbyVision,
    HLG,
    AdvancedHDR
};

/**
 * @enum CodecID
 * @brief Codec identifiers
 */
enum class CodecID : u32 {
    Unknown = 0,

    // Video codecs
    H264 = 0x1000,
    H265,
    H266,
    VP8,
    VP9,
    AV1,
    MPEG2,
    MPEG4,
    ProRes,
    DNxHD,
    MJPEG,
    Theora,
    WMV3,
    VC1,

    // Audio codecs
    AAC = 0x2000,
    MP3,
    AC3,
    EAC3,
    DTS,
    DTSHD,
    TrueHD,
    FLAC,
    ALAC,
    Vorbis,
    Opus,
    PCM_S16LE,
    PCM_S16BE,
    PCM_S24LE,
    PCM_S32LE,
    PCM_F32LE,
    WMA,

    // Subtitle codecs
    SRT = 0x3000,
    ASS,
    VTT,
    TTML,
    DVDSUB,
    DVBSUB,
    PGS,
    MOV_TEXT
};

/**
 * @enum ContainerFormat
 * @brief Media container formats
 */
enum class ContainerFormat : u16 {
    Unknown = 0,

    // Video containers
    MP4,
    MOV,
    MKV,
    WebM,
    AVI,
    WMV,
    FLV,
    TS,
    M2TS,
    OGG,
    ASF,

    // Audio containers
    MP3_Container,
    FLAC_Container,
    WAV,
    AIFF,
    M4A,
    OGG_Audio,

    // Streaming formats
    DASH,
    HLS,
    SmoothStreaming
};

/**
 * @enum PlaybackState
 * @brief Player state enumeration
 */
enum class PlaybackState : u8 {
    Idle = 0,
    Opening,
    Buffering,
    Playing,
    Paused,
    Seeking,
    Stopped,
    EndOfMedia,
    Error
};

/**
 * @enum SeekMode
 * @brief Seeking behavior modes
 */
enum class SeekMode : u8 {
    Precise = 0,      // Frame-accurate seek
    Keyframe,         // Seek to nearest keyframe
    Fast              // Fastest possible seek
};

/**
 * @enum LoopMode
 * @brief Playback loop modes
 */
enum class LoopMode : u8 {
    None = 0,
    Single,           // Loop single item
    All,              // Loop entire playlist
    AB                // A-B loop
};

/**
 * @enum ScaleMode
 * @brief Video scaling modes
 */
enum class ScaleMode : u8 {
    Fit = 0,          // Fit within bounds (letterbox/pillarbox)
    Fill,             // Fill bounds (may crop)
    Stretch,          // Stretch to fill (may distort)
    Original,         // Original size
    Zoom,             // Zoom with crop
    Custom            // Custom scale factor
};

// ═══════════════════════════════════════════════════════════════════════════════
// Result Types
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @enum ErrorCode
 * @brief Error codes for the engine
 */
enum class ErrorCode : u32 {
    Success = 0,

    // General errors
    Unknown = 1,
    InvalidArgument,
    InvalidState,
    NotInitialized,
    NotSupported,
    NotFound,
    AlreadyExists,
    OutOfMemory,
    PermissionDenied,
    Timeout,
    Cancelled,

    // File/IO errors
    FileNotFound = 100,
    FileAccessDenied,
    FileCorrupt,
    FileReadError,
    FileWriteError,

    // Media errors
    UnsupportedFormat = 200,
    UnsupportedCodec,
    DecoderError,
    EncoderError,
    DemuxerError,
    RenderError,

    // Network errors
    NetworkError = 300,
    ConnectionFailed,
    ConnectionTimeout,
    ProtocolError,
    AuthenticationFailed,

    // DRM errors
    DRMError = 400,
    LicenseExpired,
    LicenseNotFound,

    // Plugin errors
    PluginError = 500,
    PluginNotFound,
    PluginLoadFailed,

    // AI/ML errors
    InferenceError = 600,
    ModelNotFound,
    ModelLoadFailed
};

/**
 * @struct Error
 * @brief Error information structure
 */
struct Error {
    ErrorCode code = ErrorCode::Success;
    std::string message;
    std::string details;
    std::string source;  // File/function where error occurred
    i32 line = 0;

    [[nodiscard]] bool IsSuccess() const noexcept { return code == ErrorCode::Success; }
    [[nodiscard]] explicit operator bool() const noexcept { return !IsSuccess(); }

    static Error Success() { return {ErrorCode::Success, "", "", "", 0}; }
    static Error Make(ErrorCode code, std::string_view msg, std::string_view src = "", i32 ln = 0) {
        return {code, std::string(msg), "", std::string(src), ln};
    }
};

/**
 * @brief Result type for operations that can fail
 */
template<typename T>
using Result = std::expected<T, Error>;

// ═══════════════════════════════════════════════════════════════════════════════
// Smart Pointers and Handles
// ═══════════════════════════════════════════════════════════════════════════════

template<typename T>
using Unique = std::unique_ptr<T>;

template<typename T>
using Shared = std::shared_ptr<T>;

template<typename T>
using Weak = std::weak_ptr<T>;

template<typename T>
using Ref = std::reference_wrapper<T>;

// Handle types (opaque pointers)
struct HandleBase {
    void* ptr = nullptr;
    u64 id = 0;
    u32 generation = 0;

    [[nodiscard]] bool IsValid() const noexcept { return ptr != nullptr; }
    [[nodiscard]] explicit operator bool() const noexcept { return IsValid(); }
};

using PlayerHandle = HandleBase;
using SourceHandle = HandleBase;
using TrackHandle = HandleBase;
using DecoderHandle = HandleBase;
using RendererHandle = HandleBase;
using PluginHandle = HandleBase;

// ═══════════════════════════════════════════════════════════════════════════════
// Callback Types
// ═══════════════════════════════════════════════════════════════════════════════

using ProgressCallback = std::function<void(f64 progress)>;
using CompletionCallback = std::function<void(const Error& error)>;
using DataCallback = std::function<void(std::span<const u8> data)>;

} // namespace aether

#endif // AETHER_CORE_TYPES_HPP
