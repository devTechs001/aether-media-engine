// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/core/player.hpp
// DESCRIPTION: Player interface
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_CORE_PLAYER_HPP
#define AETHER_CORE_PLAYER_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <memory>
#include <string>
#include <functional>

namespace aether {

// Forward declarations
class MediaSource;
class VideoRenderer;
class AudioRenderer;

/**
 * @struct PlayerState
 * @brief Current player state
 */
struct PlayerState {
    PlaybackState playback_state = PlaybackState::Idle;
    MediaType media_type = MediaType::Unknown;
    
    // Position
    i64 position_ms = 0;
    i64 duration_ms = 0;
    f64 progress = 0.0;
    
    // Playback
    f64 playback_rate = 1.0;
    f32 volume = 1.0f;
    bool muted = false;
    LoopMode loop_mode = LoopMode::None;
    
    // Video
    SizeU video_size;
    PixelFormat pixel_format = PixelFormat::Unknown;
    f64 fps = 0.0;
    
    // Audio
    u32 sample_rate = 0;
    u32 channels = 0;
    SampleFormat sample_format = SampleFormat::Unknown;
    
    // Metadata
    std::string title;
    std::string artist;
    std::string album;
    std::string codec_video;
    std::string codec_audio;
};

/**
 * @class Player
 * @brief Media player interface
 */
class AETHER_API Player {
public:
    virtual ~Player() = default;

    // ═══════════════════════════════════════════════════════════════════════════
    // Source Management
    // ═══════════════════════════════════════════════════════════════════════════

    /**
     * @brief Open media from file path
     * @param path Path to media file
     * @return Success or error
     */
    virtual Result<void> Open(const std::string& path) = 0;

    /**
     * @brief Open media from URL
     * @param url Media URL
     * @return Success or error
     */
    virtual Result<void> OpenUrl(const std::string& url) = 0;

    /**
     * @brief Open media from source
     * @param source Media source
     * @return Success or error
     */
    virtual Result<void> OpenSource(Shared<MediaSource> source) = 0;

    /**
     * @brief Close current media
     */
    virtual void Close() = 0;

    // ═══════════════════════════════════════════════════════════════════════════
    // Playback Control
    // ═══════════════════════════════════════════════════════════════════════════

    /**
     * @brief Start playback
     */
    virtual void Play() = 0;

    /**
     * @brief Pause playback
     */
    virtual void Pause() = 0;

    /**
     * @brief Stop playback
     */
    virtual void Stop() = 0;

    /**
     * @brief Seek to position
     * @param position_ms Position in milliseconds
     * @param mode Seek mode
     * @return Success or error
     */
    virtual Result<void> Seek(i64 position_ms, SeekMode mode = SeekMode::Precise) = 0;

    /**
     * @brief Set playback speed
     * @param rate Playback rate (0.5 - 2.0)
     */
    virtual void SetPlaybackRate(f64 rate) = 0;

    // ═══════════════════════════════════════════════════════════════════════════
    // Audio Control
    // ═══════════════════════════════════════════════════════════════════════════

    /**
     * @brief Set volume
     * @param volume Volume level (0.0 - 1.0)
     */
    virtual void SetVolume(f32 volume) = 0;

    /**
     * @brief Set mute state
     * @param muted Mute state
     */
    virtual void SetMuted(bool muted) = 0;

    // ═══════════════════════════════════════════════════════════════════════════
    // State & Information
    // ═══════════════════════════════════════════════════════════════════════════

    /**
     * @brief Get current player state
     * @return Player state
     */
    [[nodiscard]] virtual PlayerState GetState() const = 0;

    /**
     * @brief Get current playback state
     * @return Playback state
     */
    [[nodiscard]] virtual PlaybackState GetPlaybackState() const = 0;

    /**
     * @brief Get current position
     * @return Position in milliseconds
     */
    [[nodiscard]] virtual i64 GetPosition() const = 0;

    /**
     * @brief Get media duration
     * @return Duration in milliseconds
     */
    [[nodiscard]] virtual i64 GetDuration() const = 0;

    // ═══════════════════════════════════════════════════════════════════════════
    // Callbacks
    // ═══════════════════════════════════════════════════════════════════════════

    using StateCallback = std::function<void(PlaybackState)>;
    using PositionCallback = std::function<void(i64 position_ms)>;
    using ErrorCallback = std::function<void(const Error&)>;

    /**
     * @brief Set state change callback
     */
    virtual void OnStateChanged(StateCallback callback) = 0;

    /**
     * @brief Set position update callback
     */
    virtual void OnPositionUpdate(PositionCallback callback) = 0;

    /**
     * @brief Set error callback
     */
    virtual void OnError(ErrorCallback callback) = 0;

protected:
    Player() = default;
};

} // namespace aether

#endif // AETHER_CORE_PLAYER_HPP
