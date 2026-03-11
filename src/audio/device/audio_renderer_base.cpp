// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/audio/device/audio_renderer_base.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/audio/audio_device.hpp"
#include "aether/core/types.hpp"

#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <vector>

namespace aether {

/**
 * @class AudioRendererBase
 * @brief Base class for audio renderers with common functionality
 */
class AudioRendererBase : public AudioRenderer {
public:
    AudioRendererBase() = default;
    ~AudioRendererBase() override = default;

    Result<void> SetFormat(const AudioFormat& format) override {
        std::lock_guard<std::mutex> lock(mutex_);
        format_ = format;
        return ConfigureFormat();
    }

    [[nodiscard]] AudioFormat GetFormat() const override {
        return format_;
    }

    Result<void> SetVolume(f32 volume) override {
        volume_.store(std::clamp(volume, 0.0f, 2.0f));
        return {};
    }

    [[nodiscard]] f32 GetVolume() const override {
        return volume_.load();
    }

    Result<void> SetMuted(bool muted) override {
        muted_.store(muted);
        return {};
    }

    [[nodiscard]] bool IsMuted() const override {
        return muted_.load();
    }

    Result<void> Write(std::span<const f32> samples) override {
        std::lock_guard<std::mutex> lock(queue_mutex_);

        // Apply volume and mute
        f32 volume = muted_.load() ? 0.0f : volume_.load();

        std::vector<f32> processed(samples.size());
        for (usize i = 0; i < samples.size(); ++i) {
            processed[i] = samples[i] * volume;
        }

        // Add to queue
        audio_queue_.push(std::move(processed));
        queue_cv_.notify_one();

        return {};
    }

    [[nodiscard]] u32 GetLatency() const override {
        return latency_ms_.load();
    }

    [[nodiscard]] u32 GetBufferedSamples() const override {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        u32 total = 0;
        for (const auto& buffer : audio_queue_) {
            total += static_cast<u32>(buffer.size());
        }
        return total;
    }

    Result<void> Flush() override {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        while (!audio_queue_.empty()) {
            audio_queue_.pop();
        }
        return {};
    }

protected:
    virtual Result<void> ConfigureFormat() = 0;

    // Get next audio buffer for playback
    std::optional<std::vector<f32>> GetNextBuffer() {
        std::unique_lock<std::mutex> lock(queue_mutex_);

        if (audio_queue_.empty()) {
            return std::nullopt;
        }

        auto buffer = std::move(audio_queue_.front());
        audio_queue_.pop();

        return buffer;
    }

protected:
    AudioFormat format_;
    std::atomic<f32> volume_{1.0f};
    std::atomic<bool> muted_{false};
    std::atomic<u32> latency_ms_{0};

    mutable std::mutex mutex_;
    mutable std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::queue<std::vector<f32>> audio_queue_;
};

// ═══════════════════════════════════════════════════════════════════════════════
// Simple Software Audio Renderer
// ═══════════════════════════════════════════════════════════════════════════════

class SoftwareAudioRenderer : public AudioRendererBase {
public:
    SoftwareAudioRenderer() = default;
    ~SoftwareAudioRenderer() override {
        Stop();
    }

    Result<void> Initialize(const AudioDeviceConfig& config) override {
        config_ = config;
        return {};
    }

    void Shutdown() override {
        Stop();
    }

    Result<void> Start() override {
        running_.store(true);
        return {};
    }

    void Stop() override {
        running_.store(false);
        queue_cv_.notify_all();
    }

    [[nodiscard]] bool IsRunning() const override {
        return running_.load();
    }

protected:
    Result<void> ConfigureFormat() override {
        // Software renderer accepts any format
        return {};
    }

private:
    AudioDeviceConfig config_;
    std::atomic<bool> running_{false};
};

// ═══════════════════════════════════════════════════════════════════════════════
// Factory Functions
// ═══════════════════════════════════════════════════════════════════════════════

Unique<AudioRenderer> CreateAudioRenderer() {
    return std::make_unique<SoftwareAudioRenderer>();
}

Unique<AudioRenderer> CreateWASAPIRenderer() {
#ifdef AETHER_PLATFORM_WINDOWS
    return std::make_unique<WASAPIAudioRenderer>();
#else
    return nullptr;
#endif
}

Unique<AudioRenderer> CreateCoreAudioRenderer() {
#ifdef AETHER_PLATFORM_MACOS
    return std::make_unique<CoreAudioRenderer>();
#else
    return nullptr;
#endif
}

Unique<AudioRenderer> CreateALSARenderer() {
#ifdef AETHER_PLATFORM_LINUX
    return std::make_unique<ALSARenderer>();
#else
    return nullptr;
#endif
}

} // namespace aether
