// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/pipeline/renderer.hpp
// DESCRIPTION: Renderer interface
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_PIPELINE_RENDERER_HPP
#define AETHER_PIPELINE_RENDERER_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"
#include "aether/media/frame.hpp"

#include <memory>

namespace aether {

/**
 * @enum RendererType
 * @brief Type of renderer
 */
enum class RendererType : u8 {
    Vulkan,
    Metal,
    D3D12,
    OpenGL,
    Software
};

/**
 * @struct RendererConfig
 * @brief Renderer configuration
 */
struct AETHER_API RendererConfig {
    RendererType type = RendererType::Vulkan;
    
    // Window/surface handle (platform-specific)
    void* window_handle = nullptr;
    
    // Video output
    SizeU output_size;
    bool fullscreen = false;
    bool vsync = true;
    
    // HDR
    bool hdr_output = false;
    f32 hdr_peak_luminance = 1000.0f;
    
    // Color management
    ColorSpace output_colorspace = ColorSpace::BT709;
    bool color_management = true;
};

/**
 * @class Renderer
 * @brief Abstract renderer interface
 */
class AETHER_API Renderer {
public:
    virtual ~Renderer() = default;

    /**
     * @brief Initialize renderer
     */
    virtual Result<void> Initialize(const RendererConfig& config) = 0;

    /**
     * @brief Shutdown renderer
     */
    virtual void Shutdown() = 0;

    /**
     * @brief Render video frame
     */
    virtual Result<void> Render(const VideoFrame& frame) = 0;

    /**
     * @brief Present frame
     */
    virtual Result<void> Present() = 0;

    /**
     * @brief Resize output
     */
    virtual Result<void> Resize(const SizeU& size) = 0;

    /**
     * @brief Set viewport
     */
    virtual void SetViewport(const Rectangle& viewport) = 0;

    /**
     * @brief Get renderer type
     */
    [[nodiscard]] virtual RendererType GetType() const = 0;

    /**
     * @brief Check if HDR is active
     */
    [[nodiscard]] virtual bool IsHDREnabled() const = 0;
};

/**
 * @brief Create Vulkan renderer
 */
AETHER_API std::unique_ptr<Renderer> CreateVulkanRenderer();

/**
 * @brief Create Metal renderer
 */
AETHER_API std::unique_ptr<Renderer> CreateMetalRenderer();

/**
 * @brief Create D3D12 renderer
 */
AETHER_API std::unique_ptr<Renderer> CreateD3D12Renderer();

/**
 * @brief Create OpenGL renderer
 */
AETHER_API std::unique_ptr<Renderer> CreateOpenGLRenderer();

/**
 * @brief Create software renderer
 */
AETHER_API std::unique_ptr<Renderer> CreateSoftwareRenderer();

/**
 * @brief Create renderer by type
 */
AETHER_API std::unique_ptr<Renderer> CreateRenderer(RendererType type);

} // namespace aether

#endif // AETHER_PIPELINE_RENDERER_HPP
