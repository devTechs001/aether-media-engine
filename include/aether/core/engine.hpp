// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/core/engine.hpp
// DESCRIPTION: Core engine interface
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_CORE_ENGINE_HPP
#define AETHER_CORE_ENGINE_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"
#include "aether/config.hpp"

#include <memory>
#include <string_view>
#include <vector>
#include <functional>

namespace aether {

// Forward declarations
class Player;
class PluginManager;
class AIEngine;
class Logger;

/**
 * @struct EngineCapabilities
 * @brief Describes engine capabilities
 */
struct EngineCapabilities {
    // Video capabilities
    bool hardware_decoding = false;
    bool hardware_encoding = false;
    std::vector<CodecID> supported_video_codecs;
    std::vector<PixelFormat> supported_pixel_formats;
    u32 max_video_width = 0;
    u32 max_video_height = 0;

    // Audio capabilities
    std::vector<CodecID> supported_audio_codecs;
    std::vector<SampleFormat> supported_sample_formats;
    u32 max_sample_rate = 0;
    u32 max_channels = 0;

    // Rendering capabilities
    bool vulkan_available = false;
    bool metal_available = false;
    bool d3d12_available = false;
    bool opengl_available = false;
    bool hdr_output = false;

    // AI capabilities
    bool ai_upscaling = false;
    bool ai_frame_interpolation = false;
    bool ai_audio_enhancement = false;
    std::vector<std::string> available_ai_backends;

    // Network capabilities
    bool streaming_support = false;
    std::vector<std::string> supported_protocols;

    // DRM capabilities
    bool drm_support = false;
    bool widevine_available = false;
    bool playready_available = false;
    bool fairplay_available = false;
};

/**
 * @struct EngineStatistics
 * @brief Runtime statistics
 */
struct EngineStatistics {
    // Memory
    usize total_memory_used = 0;
    usize video_memory_used = 0;
    usize audio_buffer_used = 0;
    usize cache_memory_used = 0;

    // Performance
    f64 average_decode_time_ms = 0;
    f64 average_render_time_ms = 0;
    f64 cpu_usage_percent = 0;
    f64 gpu_usage_percent = 0;

    // Counts
    u64 frames_decoded = 0;
    u64 frames_dropped = 0;
    u64 samples_processed = 0;
    u64 bytes_downloaded = 0;

    // Network
    f64 download_speed_mbps = 0;
    u32 active_connections = 0;
};

/**
 * @class Engine
 * @brief Core engine singleton managing all media operations
 */
class AETHER_API Engine {
public:
    /**
     * @brief Get the singleton engine instance
     * @return Reference to the engine
     */
    static Engine& Instance();

    /**
     * @brief Initialize the engine with configuration
     * @param config Engine configuration
     * @return Success or error
     */
    Result<void> Initialize(const Config& config = Config::Default());

    /**
     * @brief Shutdown the engine and release all resources
     */
    void Shutdown();

    /**
     * @brief Check if engine is initialized
     */
    [[nodiscard]] bool IsInitialized() const noexcept;

    /**
     * @brief Get current configuration
     */
    [[nodiscard]] const Config& GetConfig() const noexcept;

    /**
     * @brief Update configuration at runtime
     * @param config New configuration
     * @return Success or error
     */
    Result<void> UpdateConfig(const Config& config);

    // ═══════════════════════════════════════════════════════════════════════════
    // Player Management
    // ═══════════════════════════════════════════════════════════════════════════

    /**
     * @brief Create a new player instance
     * @param name Optional player name for identification
     * @return Player instance or error
     */
    Result<Shared<Player>> CreatePlayer(std::string_view name = "");

    /**
     * @brief Destroy a player instance
     * @param player Player to destroy
     */
    void DestroyPlayer(Shared<Player>& player);

    /**
     * @brief Get all active players
     */
    [[nodiscard]] std::vector<Shared<Player>> GetPlayers() const;

    // ═══════════════════════════════════════════════════════════════════════════
    // Subsystems
    // ═══════════════════════════════════════════════════════════════════════════

    /**
     * @brief Get plugin manager
     */
    [[nodiscard]] PluginManager& GetPluginManager();

    /**
     * @brief Get AI engine
     */
    [[nodiscard]] AIEngine& GetAIEngine();

    /**
     * @brief Get logger
     */
    [[nodiscard]] Logger& GetLogger();

    // ═══════════════════════════════════════════════════════════════════════════
    // Capabilities & Statistics
    // ═══════════════════════════════════════════════════════════════════════════

    /**
     * @brief Get engine capabilities
     */
    [[nodiscard]] const EngineCapabilities& GetCapabilities() const;

    /**
     * @brief Get runtime statistics
     */
    [[nodiscard]] EngineStatistics GetStatistics() const;

    /**
     * @brief Reset statistics counters
     */
    void ResetStatistics();

private:
    Engine() = default;
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace aether

#endif // AETHER_CORE_ENGINE_HPP
