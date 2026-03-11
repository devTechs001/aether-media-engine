// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/ai/upscaler.hpp
// DESCRIPTION: AI upscaling interface
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_AI_UPSCALER_HPP
#define AETHER_AI_UPSCALER_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"
#include "aether/media/frame.hpp"

#include <memory>
#include <string>

namespace aether {

/**
 * @enum UpscaleModel
 * @brief Upscaling model type
 */
enum class UpscaleModel : u8 {
    ESRGAN,
    RealESRGAN,
    SwinIR,
    BasicVSR,
    EDVR,
    Custom
};

/**
 * @struct UpscalerConfig
 * @brief Upscaler configuration
 */
struct AETHER_API UpscalerConfig {
    UpscaleModel model = UpscaleModel::RealESRGAN;
    u32 scale_factor = 2;
    bool use_gpu = true;
    std::string model_path;
};

/**
 * @class Upscaler
 * @brief AI video upscaler
 */
class AETHER_API Upscaler {
public:
    virtual ~Upscaler() = default;

    /**
     * @brief Initialize upscaler
     */
    virtual Result<void> Initialize(const UpscalerConfig& config) = 0;

    /**
     * @brief Shutdown upscaler
     */
    virtual void Shutdown() = 0;

    /**
     * @brief Upscale video frame
     */
    virtual Result<VideoFrame> Upscale(const VideoFrame& input) = 0;

    /**
     * @brief Get output size
     */
    [[nodiscard]] virtual SizeU GetOutputSize(const SizeU& input_size) const = 0;
};

/**
 * @brief Create AI upscaler
 */
AETHER_API std::unique_ptr<Upscaler> CreateUpscaler(const UpscalerConfig& config = UpscalerConfig());

/**
 * @brief Create ESRGAN upscaler
 */
AETHER_API std::unique_ptr<Upscaler> CreateESRGANUpscaler(u32 scale_factor = 4);

/**
 * @brief Create Real-ESRGAN upscaler
 */
AETHER_API std::unique_ptr<Upscaler> CreateRealESRGANUpscaler(u32 scale_factor = 4);

} // namespace aether

#endif // AETHER_AI_UPSCALER_HPP
