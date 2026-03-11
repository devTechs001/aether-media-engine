// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/core/engine/engine.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/core/engine.hpp"
#include "aether/utils/logging.hpp"
#include "aether/utils/memory.hpp"

#include <mutex>
#include <atomic>
#include <unordered_map>

namespace aether {

// ═══════════════════════════════════════════════════════════════════════════════
// Engine Implementation
// ═══════════════════════════════════════════════════════════════════════════════

class Engine::Impl {
public:
    Impl() = default;
    ~Impl() = default;

    Result<void> Initialize(const Config& config) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (initialized_.load()) {
            return Error::Make(ErrorCode::AlreadyInitialized, "Engine already initialized");
        }

        config_ = config;

        // Initialize logging
        auto& logger = Logger::Instance();
        if (auto result = logger.Initialize("aether.log", config.log.level); !result) {
            return result;
        }
        logger_ = &logger;

        logger_->Info("AETHER Media Engine v{} initializing...", VERSION_STRING);
        logger_->Info("  Platform: Linux | Arch: x64");
        logger_->Info("  Build: Debug");

        // Detect capabilities
        DetectCapabilities();

        initialized_.store(true);

        logger_->Info("Engine initialized successfully");
        logger_->Info("  Video codecs: {}", capabilities_.supported_video_codecs.size());
        logger_->Info("  Audio codecs: {}", capabilities_.supported_audio_codecs.size());

        return {};
    }

    void Shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!initialized_.load()) {
            return;
        }

        logger_->Info("Shutting down AETHER Media Engine...");

        // Destroy all players
        for (auto& [id, player] : players_) {
            if (player) {
                player->Stop();
            }
        }
        players_.clear();

        if (logger_) {
            logger_->Info("Engine shutdown complete");
            logger_->Shutdown();
        }

        initialized_.store(false);
    }

    bool IsInitialized() const noexcept {
        return initialized_.load();
    }

    const Config& GetConfig() const noexcept {
        return config_;
    }

    Result<void> UpdateConfig(const Config& config) {
        std::lock_guard<std::mutex> lock(mutex_);

        auto errors = config.Validate();
        if (!errors.empty()) {
            return Error::Make(ErrorCode::InvalidArgument,
                "Invalid configuration: {}", errors[0]);
        }

        // Update log level
        if (logger_ && config.log.level != config_.log.level) {
            logger_->SetLevel(config.log.level);
        }

        config_ = config;
        return {};
    }

    Result<Shared<Player>> CreatePlayer(std::string_view name) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!initialized_.load()) {
            return Error::Make(ErrorCode::NotInitialized, "Engine not initialized");
        }

        std::string player_name;
        if (name.empty()) {
            player_name = "Player_" + std::to_string(next_player_id_);
        } else {
            player_name = std::string(name);
        }

        // Create player (simplified - in production would use factory)
        auto player = std::make_shared<Player>(player_name);
        
        u64 id = next_player_id_++;
        players_[id] = player;

        logger_->Debug("Created player '{}' (id: {})", player_name, id);

        return player;
    }

    void DestroyPlayer(Shared<Player>& player) {
        std::lock_guard<std::mutex> lock(mutex_);

        for (auto it = players_.begin(); it != players_.end(); ++it) {
            if (it->second == player) {
                logger_->Debug("Destroying player '{}'", player->GetName());
                it->second->Stop();
                players_.erase(it);
                player.reset();
                return;
            }
        }
    }

    std::vector<Shared<Player>> GetPlayers() const {
        std::lock_guard<std::mutex> lock(mutex_);

        std::vector<Shared<Player>> result;
        result.reserve(players_.size());
        for (const auto& [id, player] : players_) {
            result.push_back(player);
        }
        return result;
    }

    Logger& GetLogger() {
        return *logger_;
    }

    const EngineCapabilities& GetCapabilities() const {
        return capabilities_;
    }

    EngineStatistics GetStatistics() const {
        std::lock_guard<std::mutex> lock(stats_mutex_);

        auto stats = statistics_;

        // Aggregate player statistics
        for (const auto& [id, player] : players_) {
            auto player_stats = player->GetStatistics();
            stats.frames_decoded += player_stats.frames_decoded;
            stats.frames_rendered += player_stats.frames_rendered;
            stats.frames_dropped += player_stats.frames_dropped;
        }

        // Memory stats
        stats.total_memory_used = GetMemoryUsage();

        return stats;
    }

    void ResetStatistics() {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        statistics_ = EngineStatistics{};
    }

private:
    void DetectCapabilities() {
        // Detect available codecs (simplified)
        capabilities_.supported_video_codecs = {
            CodecID::H264,
            CodecID::H265,
            CodecID::VP8,
            CodecID::VP9,
            CodecID::AV1,
            CodecID::MPEG2,
            CodecID::MPEG4
        };

        capabilities_.supported_audio_codecs = {
            CodecID::AAC,
            CodecID::MP3,
            CodecID::AC3,
            CodecID::FLAC,
            CodecID::Opus,
            CodecID::Vorbis,
            CodecID::PCM_S16LE
        };

        capabilities_.supported_protocols = {
            "file", "http", "https", "ftp", "rtmp", "rtsp", "udp", "tcp"
        };

        capabilities_.hardware_decoding = true;
        capabilities_.hardware_encoding = false;
        capabilities_.vulkan_available = false;
        capabilities_.metal_available = false;
        capabilities_.d3d12_available = false;
        capabilities_.opengl_available = true;
    }

    mutable std::mutex mutex_;
    mutable std::mutex stats_mutex_;
    std::atomic<bool> initialized_{false};
    
    Config config_;
    Logger* logger_ = nullptr;
    
    EngineCapabilities capabilities_;
    EngineStatistics statistics_;
    
    std::unordered_map<u64, Shared<Player>> players_;
    u64 next_player_id_ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════════
// Engine Singleton
// ═══════════════════════════════════════════════════════════════════════════════

Engine::Engine() : impl_(std::make_unique<Impl>()) {}

Engine::~Engine() = default;

Engine& Engine::Instance() {
    static Engine instance;
    return instance;
}

Result<void> Engine::Initialize(const Config& config) {
    return impl_->Initialize(config);
}

void Engine::Shutdown() {
    impl_->Shutdown();
}

bool Engine::IsInitialized() const noexcept {
    return impl_->IsInitialized();
}

const Config& Engine::GetConfig() const noexcept {
    return impl_->GetConfig();
}

Result<void> Engine::UpdateConfig(const Config& config) {
    return impl_->UpdateConfig(config);
}

Result<Shared<Player>> Engine::CreatePlayer(std::string_view name) {
    return impl_->CreatePlayer(name);
}

void Engine::DestroyPlayer(Shared<Player>& player) {
    impl_->DestroyPlayer(player);
}

std::vector<Shared<Player>> Engine::GetPlayers() const {
    return impl_->GetPlayers();
}

PluginManager& Engine::GetPluginManager() {
    static PluginManager instance;
    return instance;
}

AIEngine& Engine::GetAIEngine() {
    static AIEngine instance;
    return instance;
}

Logger& Engine::GetLogger() {
    return impl_->GetLogger();
}

const EngineCapabilities& Engine::GetCapabilities() const {
    return impl_->GetCapabilities();
}

EngineStatistics Engine::GetStatistics() const {
    return impl_->GetStatistics();
}

void Engine::ResetStatistics() {
    impl_->ResetStatistics();
}

// ═══════════════════════════════════════════════════════════════════════════════
// Global Functions
// ═══════════════════════════════════════════════════════════════════════════════

bool Initialize(const Config& config) {
    return Engine::Instance().Initialize(config).has_value();
}

void Shutdown() {
    Engine::Instance().Shutdown();
}

const char* GetVersionString() {
    return VERSION_STRING;
}

BuildInfo GetBuildInfo() {
    BuildInfo info;
    info.version_major = VERSION_MAJOR;
    info.version_minor = VERSION_MINOR;
    info.version_patch = VERSION_PATCH;
    info.version_string = VERSION_STRING;
    info.codename = "Prometheus";
    return info;
}

} // namespace aether
