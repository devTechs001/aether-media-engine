// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/core/player/player.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/core/player.hpp"
#include "aether/utils/logging.hpp"
#include "aether/utils/threading.hpp"

#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <queue>

namespace aether {

// ═══════════════════════════════════════════════════════════════════════════════
// Player Implementation
// ═══════════════════════════════════════════════════════════════════════════════

class Player::Impl {
public:
    explicit Impl(std::string name) : name_(std::move(name)) {}

    ~Impl() {
        Shutdown();
    }

    Result<void> Initialize(const PlayerOptions& options) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (initialized_) {
            return Error::Make(ErrorCode::AlreadyInitialized, "Player already initialized");
        }

        options_ = options;
        volume_ = options.initial_volume;
        speed_ = options.initial_speed;
        loop_mode_ = options.loop_mode;

        initialized_ = true;
        SetState(PlaybackState::Idle);

        GetLogger().Debug("Player '{}' initialized", name_);
        return {};
    }

    void Shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!initialized_) {
            return;
        }

        CloseInternal();
        initialized_ = false;

        GetLogger().Debug("Player '{}' shutdown", name_);
    }

    Result<void> Open(std::string_view url, const PlayerOptions& options) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!initialized_) {
            return Error::Make(ErrorCode::NotInitialized, "Player not initialized");
        }

        CloseInternal();

        options_ = options;
        current_url_ = std::string(url);

        SetState(PlaybackState::Opening);
        GetLogger().Info("Opening media: {}", url);

        // Simplified - in production would use FFmpeg
        media_info_.url = current_url_;
        media_info_.title = url.substr(url.find_last_of("/\\") + 1);
        media_info_.is_seekable = true;

        SetState(PlaybackState::Ready);
        return {};
    }

    void Close() {
        std::lock_guard<std::mutex> lock(mutex_);
        CloseInternal();
    }

    void CloseInternal() {
        if (state_ == PlaybackState::Idle) {
            return;
        }

        GetLogger().Debug("Closing media");

        running_ = false;
        packet_queue_cv_.notify_all();

        if (demux_thread_.joinable()) {
            demux_thread_.join();
        }

        position_ = Timestamp{};
        duration_ = Timestamp{};
        media_info_ = MediaInfo{};
        current_url_.clear();

        SetState(PlaybackState::Idle);
    }

    Result<void> Play() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (state_ == PlaybackState::Idle) {
            return Error::Make(ErrorCode::InvalidState, "No media loaded");
        }

        if (state_ == PlaybackState::Playing) {
            return {};
        }

        if (state_ == PlaybackState::Error) {
            return Error::Make(ErrorCode::InvalidState, "Player in error state");
        }

        paused_ = false;
        SetState(PlaybackState::Playing);

        return {};
    }

    Result<void> Pause() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (state_ != PlaybackState::Playing) {
            return Error::Make(ErrorCode::InvalidState, "Not playing");
        }

        paused_ = true;
        SetState(PlaybackState::Paused);

        return {};
    }

    Result<void> Stop() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (state_ == PlaybackState::Idle) {
            return {};
        }

        Seek(Timestamp{});
        paused_ = true;
        SetState(PlaybackState::Stopped);

        return {};
    }

    Result<void> Seek(Timestamp position) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (state_ == PlaybackState::Idle) {
            return Error::Make(ErrorCode::InvalidState, "No media loaded");
        }

        if (!media_info_.is_seekable) {
            return Error::Make(ErrorCode::NotSupported, "Media is not seekable");
        }

        f64 target_sec = std::clamp(position.ToSeconds(), 0.0, duration_.ToSeconds());
        position_ = Timestamp::FromSeconds(target_sec);

        GetLogger().Debug("Seek to {:.2f}s", target_sec);
        return {};
    }

    PlaybackState GetState() const {
        return state_.load();
    }

    PlaybackStatus GetStatus() const {
        std::lock_guard<std::mutex> lock(mutex_);

        PlaybackStatus status;
        status.state = state_.load();
        status.position = position_;
        status.duration = duration_;
        status.progress = duration_.pts > 0
            ? static_cast<f64>(position_.pts) / duration_.pts
            : 0.0;
        status.volume = volume_;
        status.is_muted = muted_;
        status.speed = speed_;
        status.last_error = last_error_;
        status.statistics = statistics_;

        return status;
    }

    PlayerStatistics GetStatistics() const {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        return statistics_;
    }

    std::optional<MediaInfo> GetMediaInfo() const {
        std::lock_guard<std::mutex> lock(mutex_);

        if (state_ == PlaybackState::Idle) {
            return std::nullopt;
        }

        return media_info_;
    }

    Timestamp GetPosition() const {
        return position_;
    }

    Timestamp GetDuration() const {
        return duration_;
    }

    // Volume control
    Result<void> SetVolume(f32 volume) {
        volume_ = std::clamp(volume, 0.0f, 2.0f);
        return {};
    }

    f32 GetVolume() const {
        return volume_.load();
    }

    Result<void> SetMuted(bool muted) {
        muted_ = muted;
        return {};
    }

    bool IsMuted() const {
        return muted_.load();
    }

    // Speed control
    Result<void> SetSpeed(f32 speed) {
        speed_ = std::clamp(speed, 0.25f, 4.0f);
        speed_ = speed;
        return {};
    }

    f32 GetSpeed() const {
        return speed_;
    }

    // Track selection
    std::vector<VideoTrackInfo> GetVideoTracks() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return media_info_.video_tracks;
    }

    std::vector<AudioTrackInfo> GetAudioTracks() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return media_info_.audio_tracks;
    }

    std::vector<SubtitleTrackInfo> GetSubtitleTracks() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return media_info_.subtitle_tracks;
    }

    Result<void> SelectVideoTrack(i32 index) {
        (void)index;
        return {};
    }

    Result<void> SelectAudioTrack(i32 index) {
        (void)index;
        return {};
    }

    Result<void> SelectSubtitleTrack(i32 index) {
        (void)index;
        return {};
    }

    const std::string& GetName() const {
        return name_;
    }

    const PlayerOptions& GetOptions() const {
        return options_;
    }

    Error GetLastError() const {
        return last_error_;
    }

private:
    void SetState(PlaybackState state) {
        PlaybackState old_state = state_.exchange(state);
        if (old_state != state) {
            GetLogger().Debug("Player '{}' state: {} -> {}", name_,
                ToString(old_state), ToString(state));
        }
    }

    // Member variables
    std::string name_;
    std::atomic<bool> initialized_{false};
    std::atomic<PlaybackState> state_{PlaybackState::Idle};

    PlayerOptions options_;
    std::string current_url_;
    MediaInfo media_info_;

    Timestamp position_;
    Timestamp duration_;

    std::atomic<f32> volume_{1.0f};
    std::atomic<bool> muted_{false};
    std::atomic<f32> speed_{1.0f};
    LoopMode loop_mode_ = LoopMode::None;

    std::atomic<bool> running_{false};
    std::atomic<bool> paused_{false};
    std::atomic<bool> seeking_{false};
    std::atomic<bool> buffering_{false};
    std::atomic<bool> eof_reached_{false};

    Error last_error_;
    PlayerStatistics statistics_;

    // Threading
    mutable std::mutex mutex_;
    mutable std::mutex stats_mutex_;
    std::condition_variable packet_queue_cv_;

    // Packet queues (simplified)
    std::queue<AVPacket*> video_packet_queue_;
    std::queue<AVPacket*> audio_packet_queue_;
    std::queue<AVFrame*> video_frame_queue_;
    std::queue<AVFrame*> audio_frame_queue_;

    // Threads
    std::thread demux_thread_;
    std::thread decode_video_thread_;
    std::thread decode_audio_thread_;

    // FFmpeg contexts (would be used in full implementation)
    AVFormatContext* format_ctx_ = nullptr;
    AVCodecContext* video_codec_ctx_ = nullptr;
    AVCodecContext* audio_codec_ctx_ = nullptr;

    int video_stream_index_ = -1;
    int audio_stream_index_ = -1;
    int subtitle_stream_index_ = -1;

    // Constants
    static constexpr usize MAX_PACKET_QUEUE_SIZE = 50;
    static constexpr usize MAX_FRAME_QUEUE_SIZE = 10;
};

// ═══════════════════════════════════════════════════════════════════════════════
// Player Public Interface
// ═══════════════════════════════════════════════════════════════════════════════

Player::Player(std::string name)
    : impl_(std::make_unique<Impl>(std::move(name))) {}

Player::~Player() = default;

Result<void> Player::Open(std::string_view url, const PlayerOptions& options) {
    return impl_->Open(url, options);
}

void Player::Close() {
    impl_->Close();
}

Result<void> Player::Play() {
    return impl_->Play();
}

Result<void> Player::Pause() {
    return impl_->Pause();
}

Result<void> Player::Stop() {
    return impl_->Stop();
}

Result<void> Player::Seek(Timestamp position) {
    return impl_->Seek(position);
}

PlaybackState Player::GetState() const {
    return impl_->GetState();
}

PlaybackStatus Player::GetStatus() const {
    return impl_->GetStatus();
}

PlayerStatistics Player::GetStatistics() const {
    return impl_->GetStatistics();
}

std::optional<MediaInfo> Player::GetMediaInfo() const {
    return impl_->GetMediaInfo();
}

Timestamp Player::GetPosition() const {
    return impl_->GetPosition();
}

Timestamp Player::GetDuration() const {
    return impl_->GetDuration();
}

Result<void> Player::SetVolume(f32 volume) {
    return impl_->SetVolume(volume);
}

f32 Player::GetVolume() const {
    return impl_->GetVolume();
}

Result<void> Player::SetMuted(bool muted) {
    return impl_->SetMuted(muted);
}

bool Player::IsMuted() const {
    return impl_->IsMuted();
}

Result<void> Player::SetSpeed(f32 speed) {
    return impl_->SetSpeed(speed);
}

f32 Player::GetSpeed() const {
    return impl_->GetSpeed();
}

std::vector<VideoTrackInfo> Player::GetVideoTracks() const {
    return impl_->GetVideoTracks();
}

std::vector<AudioTrackInfo> Player::GetAudioTracks() const {
    return impl_->GetAudioTracks();
}

std::vector<SubtitleTrackInfo> Player::GetSubtitleTracks() const {
    return impl_->GetSubtitleTracks();
}

Result<void> Player::SelectVideoTrack(i32 index) {
    return impl_->SelectVideoTrack(index);
}

Result<void> Player::SelectAudioTrack(i32 index) {
    return impl_->SelectAudioTrack(index);
}

Result<void> Player::SelectSubtitleTrack(i32 index) {
    return impl_->SelectSubtitleTrack(index);
}

std::string_view Player::GetName() const {
    return impl_->GetName();
}

const PlayerOptions& Player::GetOptions() const {
    return impl_->GetOptions();
}

Error Player::GetLastError() const {
    return impl_->GetLastError();
}

} // namespace aether
