// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/pipeline/pipeline.hpp
// DESCRIPTION: Media pipeline interface
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_PIPELINE_PIPELINE_HPP
#define AETHER_PIPELINE_PIPELINE_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <memory>
#include <string>
#include <vector>

namespace aether {

// Forward declarations
class Demuxer;
class Decoder;
class Filter;
class Renderer;

/**
 * @enum PipelineState
 * @brief Pipeline execution state
 */
enum class PipelineState : u8 {
    Created,
    Configured,
    Running,
    Paused,
    Stopped,
    Error
};

/**
 * @struct PipelineConfig
 * @brief Pipeline configuration
 */
struct AETHER_API PipelineConfig {
    // Video pipeline
    bool enable_video = true;
    bool hardware_decode = true;
    std::vector<std::string> video_filters;

    // Audio pipeline
    bool enable_audio = true;
    bool hardware_resample = true;
    std::vector<std::string> audio_filters;

    // Subtitle pipeline
    bool enable_subtitles = true;
    bool bitmap_subtitles = true;

    // Threading
    u32 decode_threads = 0;  // 0 = auto
    u32 frame_buffer_size = 10;
};

/**
 * @class Pipeline
 * @brief Media processing pipeline
 */
class AETHER_API Pipeline {
public:
    virtual ~Pipeline() = default;

    /**
     * @brief Configure pipeline
     */
    virtual Result<void> Configure(const PipelineConfig& config) = 0;

    /**
     * @brief Start pipeline execution
     */
    virtual Result<void> Start() = 0;

    /**
     * @brief Pause pipeline
     */
    virtual void Pause() = 0;

    /**
     * @brief Resume pipeline
     */
    virtual void Resume() = 0;

    /**
     * @brief Stop pipeline
     */
    virtual void Stop() = 0;

    /**
     * @brief Get current state
     */
    [[nodiscard]] virtual PipelineState GetState() const = 0;

    /**
     * @brief Add demuxer
     */
    virtual Result<void> AddDemuxer(std::unique_ptr<Demuxer> demuxer) = 0;

    /**
     * @brief Add decoder
     */
    virtual Result<void> AddDecoder(std::unique_ptr<Decoder> decoder) = 0;

    /**
     * @brief Add filter
     */
    virtual Result<void> AddFilter(std::unique_ptr<Filter> filter) = 0;

    /**
     * @brief Add renderer
     */
    virtual Result<void> AddRenderer(std::unique_ptr<Renderer> renderer) = 0;

    /**
     * @brief Process single frame
     */
    virtual Result<void> ProcessFrame() = 0;

    /**
     * @brief Flush pipeline
     */
    virtual Result<void> Flush() = 0;

    /**
     * @brief Reset pipeline
     */
    virtual Result<void> Reset() = 0;
};

/**
 * @brief Create default pipeline
 */
AETHER_API std::unique_ptr<Pipeline> CreatePipeline(const PipelineConfig& config = PipelineConfig());

} // namespace aether

#endif // AETHER_PIPELINE_PIPELINE_HPP
