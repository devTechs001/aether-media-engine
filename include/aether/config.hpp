// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/config.hpp
// DESCRIPTION: Configuration settings for AETHER Media Engine
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_CONFIG_HPP
#define AETHER_CONFIG_HPP

#include "aether/export.hpp"
#include <string>
#include <optional>

namespace aether {

/**
 * @struct Config
 * @brief Engine configuration options
 */
struct AETHER_API Config {
    // Application settings
    std::string app_name = "Aether Media Engine";
    std::string config_dir;
    std::string cache_dir;
    std::string log_dir;

    // Memory settings
    usize max_memory_usage = 0;  // 0 = unlimited
    usize cache_size_mb = 512;
    bool enable_memory_pooling = true;

    // Threading settings
    u32 worker_threads = 0;  // 0 = auto-detect
    u32 io_threads = 4;

    // Video settings
    bool hardware_decoding = true;
    bool hardware_encoding = false;
    bool hdr_tone_mapping = true;
    bool video_upscaling = false;

    // Audio settings
    u32 audio_sample_rate = 48000;
    u32 audio_buffer_size = 2048;
    bool audio_normalization = false;

    // Network settings
    usize http_cache_size_mb = 1024;
    u32 network_buffer_size = 65536;
    u32 connection_timeout_ms = 30000;

    // AI/ML settings
    bool ai_enhancement = false;
    std::string ai_backend = "onnx";  // onnx, tensorrt, coreml, openvino

    // Logging
    enum class LogLevel {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };
    LogLevel log_level = LogLevel::Info;
    bool log_to_file = true;
    bool log_to_console = true;

    // Platform-specific
    bool use_vulkan = true;
    bool use_metal = true;
    bool use_d3d12 = true;

    /**
     * @brief Create default configuration
     */
    static Config Default() { return Config(); }

    /**
     * @brief Create configuration from file
     * @param path Path to configuration file
     * @return Configuration object
     */
    static Config FromFile(const std::string& path);

    /**
     * @brief Save configuration to file
     * @param path Path to configuration file
     */
    void SaveToFile(const std::string& path) const;
};

} // namespace aether

#endif // AETHER_CONFIG_HPP
