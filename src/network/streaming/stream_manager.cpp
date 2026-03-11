// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/network/streaming/stream_manager.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/network/streaming.hpp"
#include "aether/utils/logging.hpp"

#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>

namespace aether {

class StreamManager::Impl {
public:
    std::mutex mutex;
    StreamInfo current_stream;
    std::atomic<bool> streaming{false};
    std::atomic<f64> current_bandwidth{0.0};
    std::atomic<i64> bytes_downloaded{0};
    std::queue<std::vector<u8>> buffer_queue;
    std::condition_variable buffer_cv;
};

StreamManager& StreamManager::Instance() {
    static StreamManager instance;
    return instance;
}

StreamManager::StreamManager() : impl_(std::make_unique<Impl>()) {}

StreamManager::~StreamManager() = default;

Result<void> StreamManager::OpenStream(const std::string& url) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    GetLogger().Info("Opening stream: {}", url);

    // Detect stream type
    impl_->current_stream.type = DetectStreamType(url);
    impl_->current_stream.url = url;

    // Parse manifest if applicable
    if (impl_->current_stream.type == StreamType::DASH ||
        impl_->current_stream.type == StreamType::HLS) {
        auto result = ParseManifest(url);
        if (result) {
            impl_->current_stream.quality_levels = result->quality_levels;
            impl_->current_stream.is_live = result->is_live;
            impl_->current_stream.dvr_enabled = result->dvr_enabled;
        }
    }

    impl_->streaming = true;
    return {};
}

void StreamManager::CloseStream() {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->streaming = false;
    impl_->current_stream = StreamInfo{};
}

StreamInfo StreamManager::GetStreamInfo() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->current_stream;
}

Result<void> StreamManager::SwitchQuality(u32 bitrate) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    // Find quality level
    for (const auto& level : impl_->current_stream.quality_levels) {
        if (level.bitrate == bitrate) {
            impl_->current_stream.current_bitrate = bitrate;
            GetLogger().Info("Switched to quality: {} kbps", bitrate / 1000);
            return {};
        }
    }

    return Error::Make(ErrorCode::InvalidArgument, "Quality level not found");
}

std::vector<u32> StreamManager::GetAvailableBitrates() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    std::vector<u32> bitrates;
    for (const auto& level : impl_->current_stream.quality_levels) {
        bitrates.push_back(level.bitrate);
    }
    return bitrates;
}

f64 StreamManager::GetCurrentBandwidth() const {
    return impl_->current_bandwidth.load();
}

// ═══════════════════════════════════════════════════════════════════════════════
// Stream Type Detection
// ═══════════════════════════════════════════════════════════════════════════════

StreamType DetectStreamType(const std::string& url) {
    if (url.find(".m3u8") != std::string::npos ||
        url.find("hls") != std::string::npos) {
        return StreamType::HLS;
    }

    if (url.find(".mpd") != std::string::npos ||
        url.find("dash") != std::string::npos) {
        return StreamType::DASH;
    }

    if (url.find("rtsp://") == 0) {
        return StreamType::RTSP;
    }

    if (url.find("rtmp://") == 0) {
        return StreamType::RTMP;
    }

    if (url.find("srt://") == 0) {
        return StreamType::SRT;
    }

    if (url.find("webrtc") != std::string::npos) {
        return StreamType::WebRTC;
    }

    // Default to HLS for unknown streaming URLs
    return StreamType::HLS;
}

Result<StreamInfo> ParseManifest(const std::string& url) {
    StreamInfo info;
    info.url = url;

    // In production, this would:
    // 1. Download the manifest file (.m3u8 for HLS, .mpd for DASH)
    // 2. Parse the manifest to extract quality levels
    // 3. Detect if stream is live or VOD
    // 4. Extract DVR window information

    // Simplified implementation - would be replaced with actual parsing
    GetLogger().Debug("Parsing manifest: {}", url);

    // Example quality levels (would be extracted from manifest)
    info.quality_levels = {
        {500000, 640, 360, "avc1.42001e", url + "/360p.m3u8"},
        {1000000, 854, 480, "avc1.42001e", url + "/480p.m3u8"},
        {2000000, 1280, 720, "avc1.42001f", url + "/720p.m3u8"},
        {4000000, 1920, 1080, "avc1.420028", url + "/1080p.m3u8"}
    };

    info.is_live = url.find("live") != std::string::npos;
    info.dvr_enabled = !info.is_live;
    info.dvr_window_ms = 7200000;  // 2 hours

    return info;
}

} // namespace aether
