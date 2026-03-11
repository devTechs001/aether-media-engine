// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/video/video_renderer.hpp
// DESCRIPTION: Video renderer interface
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_VIDEO_VIDEO_RENDERER_HPP
#define AETHER_VIDEO_VIDEO_RENDERER_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"
#include "aether/media/frame.hpp"

#include <memory>

namespace aether {

/**
 * @struct VideoRendererConfig
 * @brief Video renderer configuration
 */
struct AETHER_API VideoRendererConfig {
    // Output
    void* window_handle = nullptr;
    SizeU output_size;
    
    // Rendering
    bool vsync = true;
    bool hdr = false;
    
    // Scaling
    ScaleMode scale_mode = ScaleMode::Fit;
    
    // Color
    ColorSpace color_space = ColorSpace::BT709;
    ColorRange color_range = ColorRange::Limited;
    
    // Deinterlacing
    bool deinterlace = false;
};

/**
 * @class VideoRenderer
 * @brief Video rendering interface
 */
class AETHER_API VideoRenderer {
public:
    virtual ~VideoRenderer() = default;

    /**
     * @brief Initialize renderer
     */
    virtual Result<void> Initialize(const VideoRendererConfig& config) = 0;

    /**
     * @brief Shutdown renderer
     */
    virtual void Shutdown() = 0;

    /**
     * @brief Render video frame
     */
    virtual Result<void> Render(const VideoFrame& frame) = 0;

    /**
     * @brief Present rendered frame
     */
    virtual Result<void> Present() = 0;

    /**
     * @brief Set output size
     */
    virtual void SetOutputSize(const SizeU& size) = 0;

    /**
     * @brief Set color space
     */
    virtual void SetColorSpace(ColorSpace cs) = 0;

    /**
     * @brief Enable/disable HDR
     */
    virtual void SetHDREnabled(bool enabled) = 0;
};

/**
 * @brief Create platform-optimized video renderer
 */
AETHER_API std::unique_ptr<VideoRenderer> CreateVideoRenderer(const VideoRendererConfig& config = VideoRendererConfig());

} // namespace aether

#endif // AETHER_VIDEO_VIDEO_RENDERER_HPP
