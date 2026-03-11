// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/aether.hpp
// DESCRIPTION: Main include header for AETHER Media Engine
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_AETHER_HPP
#define AETHER_AETHER_HPP

// Version Information
#include "aether/version.hpp"

// Configuration
#include "aether/config.hpp"

// Export Macros
#include "aether/export.hpp"

// Core Components
#include "aether/core/engine.hpp"
#include "aether/core/player.hpp"
#include "aether/core/media_source.hpp"
#include "aether/core/event_system.hpp"
#include "aether/core/error_handling.hpp"
#include "aether/core/types.hpp"

// Media Types
#include "aether/media/frame.hpp"
#include "aether/media/packet.hpp"
#include "aether/media/format.hpp"
#include "aether/media/codec.hpp"
#include "aether/media/metadata.hpp"

// Pipeline
#include "aether/pipeline/pipeline.hpp"
#include "aether/pipeline/demuxer.hpp"
#include "aether/pipeline/decoder.hpp"
#include "aether/pipeline/filter.hpp"
#include "aether/pipeline/renderer.hpp"

// Audio
#include "aether/audio/audio_device.hpp"
#include "aether/audio/audio_processor.hpp"

// Video
#include "aether/video/video_frame.hpp"
#include "aether/video/video_renderer.hpp"

// Network
#include "aether/network/streaming.hpp"
#include "aether/network/protocol.hpp"

// AI/ML
#include "aether/ai/inference_engine.hpp"
#include "aether/ai/upscaler.hpp"

// Plugin System
#include "aether/plugin/plugin.hpp"
#include "aether/plugin/plugin_manager.hpp"

// Utilities
#include "aether/utils/logging.hpp"
#include "aether/utils/memory.hpp"
#include "aether/utils/threading.hpp"

/**
 * @namespace aether
 * @brief Root namespace for AETHER Media Engine
 */
namespace aether {

/**
 * @brief Initialize the AETHER Media Engine
 * @param config Optional configuration object
 * @return true if initialization successful
 */
AETHER_API bool Initialize(const Config& config = Config::Default());

/**
 * @brief Shutdown the AETHER Media Engine
 */
AETHER_API void Shutdown();

/**
 * @brief Get the engine version string
 * @return Version string in format "major.minor.patch"
 */
AETHER_API const char* GetVersionString();

/**
 * @brief Get detailed build information
 * @return Build information structure
 */
AETHER_API BuildInfo GetBuildInfo();

} // namespace aether

#endif // AETHER_AETHER_HPP
