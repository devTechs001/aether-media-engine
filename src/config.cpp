// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/config.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/config.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace aether {

namespace fs = std::filesystem;

class Config::Impl {
public:
    std::map<std::string, ConfigValue> dynamic_values;
};

Config::Config() : impl_(std::make_unique<Impl>()) {
    // Initialize with default paths based on platform
#if defined(_WIN32) || defined(_WIN64)
    const char* appdata = std::getenv("APPDATA");
    if (appdata) {
        data_directory = std::string(appdata) + "\\AetherMediaEngine";
    }
    const char* temp = std::getenv("TEMP");
    if (temp) {
        temp_directory = std::string(temp) + "\\AetherMediaEngine";
    }
#elif defined(__APPLE__)
    const char* home = std::getenv("HOME");
    if (home) {
        data_directory = std::string(home) + "/Library/Application Support/AetherMediaEngine";
        temp_directory = "/tmp/AetherMediaEngine";
    }
#elif defined(__linux__)
    const char* xdg_data = std::getenv("XDG_DATA_HOME");
    const char* home = std::getenv("HOME");
    if (xdg_data) {
        data_directory = std::string(xdg_data) + "/AetherMediaEngine";
    } else if (home) {
        data_directory = std::string(home) + "/.local/share/AetherMediaEngine";
    }
    temp_directory = "/tmp/AetherMediaEngine";
#endif

    // Set default directories
    if (!data_directory.empty()) {
        cache.disk_cache_path = data_directory + "/cache";
        ai.models_directory = data_directory + "/models";
    }
}

Config::~Config() = default;

Config::Config(const Config& other)
    : impl_(std::make_unique<Impl>(*other.impl_))
    , log(other.log)
    , video(other.video)
    , audio(other.audio)
    , network(other.network)
    , ai(other.ai)
    , subtitle(other.subtitle)
    , cache(other.cache)
    , plugin(other.plugin)
    , drm(other.drm)
    , analytics(other.analytics)
    , app_name(other.app_name)
    , app_version(other.app_version)
    , data_directory(other.data_directory)
    , temp_directory(other.temp_directory)
    , locale(other.locale) {
}

Config& Config::operator=(const Config& other) {
    if (this != &other) {
        impl_ = std::make_unique<Impl>(*other.impl_);
        log = other.log;
        video = other.video;
        audio = other.audio;
        network = other.network;
        ai = other.ai;
        subtitle = other.subtitle;
        cache = other.cache;
        plugin = other.plugin;
        drm = other.drm;
        analytics = other.analytics;
        app_name = other.app_name;
        app_version = other.app_version;
        data_directory = other.data_directory;
        temp_directory = other.temp_directory;
        locale = other.locale;
    }
    return *this;
}

Config::Config(Config&& other) noexcept = default;
Config& Config::operator=(Config&& other) noexcept = default;

Config Config::Default() {
    return Config{};
}

Config Config::HighPerformance() {
    Config config;

    // Video - optimize for speed
    config.video.enable_hardware_decoding = true;
    config.video.decode_thread_count = 0;  // Auto
    config.video.allow_frame_drop = true;
    config.video.enable_vsync = false;
    config.video.enable_triple_buffering = true;

    // Audio - low latency
    config.audio.buffer_size_ms = 20;
    config.audio.latency_target_ms = 10;
    config.audio.enable_exclusive_mode = true;

    // Network - aggressive prefetching
    config.network.max_connections = 16;
    config.network.prefetch_next_segment = true;

    // AI - disabled for performance
    config.ai.enable_ai_features = false;

    // Cache - large memory cache
    config.cache.memory_cache_size_mb = 1024;
    config.cache.decoded_frame_cache_count = 60;

    return config;
}

Config Config::HighQuality() {
    Config config;

    // Video - best quality
    config.video.enable_hardware_decoding = true;
    config.video.scale_algorithm = "lanczos";
    config.video.enable_deinterlacing = true;
    config.video.enable_hdr_output = true;
    config.video.enable_10bit_output = true;

    // Audio - high quality resampling
    config.audio.resampler_quality = "ultra";
    config.audio.enable_spatial_audio = true;

    // AI - enabled with best quality
    config.ai.enable_ai_features = true;
    config.ai.upscaling_quality = 1.0f;
    config.ai.interpolation_quality = 1.0f;

    // Subtitle - full ASS styling
    config.subtitle.enable_ass_styling = true;

    return config;
}

Config Config::LowPower() {
    Config config;

    // Video - power saving
    config.video.enable_hardware_decoding = true;
    config.video.decode_thread_count = 2;
    config.video.allow_frame_drop = true;
    config.video.max_video_width = 1920;
    config.video.max_video_height = 1080;
    config.video.scale_algorithm = "bilinear";
    config.video.enable_deinterlacing = false;

    // Audio - simpler processing
    config.audio.buffer_size_ms = 100;
    config.audio.resampler_quality = "low";
    config.audio.enable_spatial_audio = false;

    // Network - bandwidth limited
    config.network.max_bitrate = 5000000;  // 5 Mbps

    // AI - disabled
    config.ai.enable_ai_features = false;

    // Cache - reduced
    config.cache.memory_cache_size_mb = 128;
    config.cache.decoded_frame_cache_count = 10;

    return config;
}

Config Config::Minimal() {
    Config config;

    config.video.enable_hardware_decoding = false;
    config.video.decode_thread_count = 1;

    config.audio.enable_spatial_audio = false;

    config.ai.enable_ai_features = false;

    config.plugin.enable_plugins = false;

    config.analytics.enable_analytics = false;

    config.cache.enable_disk_cache = false;
    config.cache.memory_cache_size_mb = 64;

    return config;
}

Result<Config> Config::LoadFromFile(std::string_view path) {
    std::ifstream file(std::string(path));
    if (!file.is_open()) {
        return std::unexpected(Error::Make(ErrorCode::FileNotFound,
            "Config file not found: {}", path));
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    fs::path filepath(path);
    std::string ext = filepath.extension().string();

    if (ext == ".json") {
        return LoadFromJSON(content);
    } else if (ext == ".toml") {
        return LoadFromTOML(content);
    } else {
        // Try JSON first
        auto result = LoadFromJSON(content);
        if (result) return result;
        return LoadFromTOML(content);
    }
}

Result<Config> Config::LoadFromJSON(std::string_view json_str) {
    // Simple JSON parsing - in production would use nlohmann/json
    Config config;
    // TODO: Implement full JSON parsing
    return config;
}

Result<Config> Config::LoadFromTOML(std::string_view toml_str) {
    Config config;
    // TODO: Implement TOML parsing
    return config;
}

Result<void> Config::SaveToFile(std::string_view path) const {
    std::ofstream file(std::string(path));
    if (!file.is_open()) {
        return std::unexpected(Error::Make(ErrorCode::FileWriteError,
            "Failed to open config file for writing: {}", path));
    }

    // TODO: Implement JSON serialization
    file << "{\n";
    file << "  // Configuration saved\n";
    file << "}\n";

    return {};
}

std::vector<std::string> Config::Validate() const {
    std::vector<std::string> errors;

    // Validate log config
    if (log.log_to_file && log.log_file_path.empty()) {
        errors.push_back("Log file path is empty but log_to_file is enabled");
    }

    // Validate video config
    if (video.decode_thread_count > 32) {
        errors.push_back("Decode thread count too high (max 32)");
    }

    // Validate audio config
    if (audio.buffer_size_ms < 10) {
        errors.push_back("Audio buffer size too small (min 10ms)");
    }
    if (audio.buffer_size_ms > 1000) {
        errors.push_back("Audio buffer size too large (max 1000ms)");
    }

    // Validate network config
    if (network.max_connections > 64) {
        errors.push_back("Max connections too high (max 64)");
    }

    // Validate cache config
    if (cache.memory_cache_size_mb > 4096) {
        errors.push_back("Memory cache size too large (max 4GB)");
    }

    return errors;
}

bool Config::IsValid() const {
    return Validate().empty();
}

void Config::Merge(const Config& other, bool override_existing) {
    if (override_existing || app_name.empty()) {
        app_name = other.app_name;
    }
    if (override_existing || app_version.empty()) {
        app_version = other.app_version;
    }
    if (override_existing || data_directory.empty()) {
        data_directory = other.data_directory;
    }
    if (override_existing || temp_directory.empty()) {
        temp_directory = other.temp_directory;
    }
    if (override_existing || locale.empty()) {
        locale = other.locale;
    }

    // Merge sub-configs
    // (In production would merge each sub-config individually)
}

Config Config::Merged(const Config& other, bool override_existing) const {
    Config result = *this;
    result.Merge(other, override_existing);
    return result;
}

void Config::Reset() {
    *this = Default();
}

void Config::ResetSection(std::string_view section) {
    Config defaults = Default();

    if (section == "log") {
        log = defaults.log;
    } else if (section == "video") {
        video = defaults.video;
    } else if (section == "audio") {
        audio = defaults.audio;
    } else if (section == "network") {
        network = defaults.network;
    } else if (section == "ai") {
        ai = defaults.ai;
    } else if (section == "subtitle") {
        subtitle = defaults.subtitle;
    } else if (section == "cache") {
        cache = defaults.cache;
    } else if (section == "plugin") {
        plugin = defaults.plugin;
    } else if (section == "drm") {
        drm = defaults.drm;
    } else if (section == "analytics") {
        analytics = defaults.analytics;
    }
}

} // namespace aether
