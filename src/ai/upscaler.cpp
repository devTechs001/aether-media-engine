// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/ai/upscaler.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/ai/upscaler.hpp"
#include "aether/ai/inference_engine.hpp"
#include "aether/utils/logging.hpp"

#include <mutex>
#include <atomic>

namespace aether {

// ═══════════════════════════════════════════════════════════════════════════════
// AI Upscaler Implementation
// ═══════════════════════════════════════════════════════════════════════════════

class Upscaler::Impl {
public:
    std::mutex mutex;
    bool initialized = false;
    UpscalerConfig config;
    std::atomic<i64> frames_processed{0};
    std::atomic<f64> avg_upscale_time_ms{0.0};
};

Upscaler::Upscaler() : impl_(std::make_unique<Impl>()) {}

Upscaler::~Upscaler() {
    Shutdown();
}

Result<void> Upscaler::Initialize(const UpscalerConfig& config) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (impl_->initialized) {
        return Error::Make(ErrorCode::AlreadyInitialized, "Upscaler already initialized");
    }

    impl_->config = config;

    // Validate model path
    if (config.model_path.empty()) {
        // Use default model
        impl_->config.model_path = GetDefaultModelPath(config.model);
    }

    GetLogger().Info("Initialized AI upscaler: {} (scale: {}x)", 
                    GetModelName(config.model), config.scale_factor);

    impl_->initialized = true;
    return {};
}

void Upscaler::Shutdown() {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->initialized = false;
}

Result<VideoFrame> Upscaler::Upscale(const VideoFrame& input) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (!impl_->initialized) {
        return Error::Make(ErrorCode::NotInitialized, "Upscaler not initialized");
    }

    auto start = std::chrono::steady_clock::now();

    // In production, would run AI inference
    // For now, return input frame (no upscaling)
    VideoFrame output = input;
    output.size.width *= impl_->config.scale_factor;
    output.size.height *= impl_->config.scale_factor;

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration<f64, std::milli>(end - start).count();

    // Update statistics
    impl_->frames_processed++;
    f64 avg = impl_->avg_upscale_time_ms.load();
    impl_->avg_upscale_time_ms = avg + (duration - avg) / impl_->frames_processed;

    return output;
}

SizeU Upscaler::GetOutputSize(const SizeU& input_size) const {
    return SizeU{
        input_size.width * impl_->config.scale_factor,
        input_size.height * impl_->config.scale_factor
    };
}

std::string Upscaler::GetModelName(UpscaleModel model) const {
    switch (model) {
        case UpscaleModel::ESRGAN: return "ESRGAN";
        case UpscaleModel::RealESRGAN: return "Real-ESRGAN";
        case UpscaleModel::SwinIR: return "SwinIR";
        case UpscaleModel::BasicVSR: return "BasicVSR";
        case UpscaleModel::EDVR: return "EDVR";
        case UpscaleModel::Custom: return "Custom";
        default: return "Unknown";
    }
}

std::string Upscaler::GetDefaultModelPath(UpscaleModel model) const {
    // Default model paths
    switch (model) {
        case UpscaleModel::ESRGAN:
            return "models/esrgan/x4.pth";
        case UpscaleModel::RealESRGAN:
            return "models/realesrgan/x4.pth";
        case UpscaleModel::SwinIR:
            return "models/swinir/x4.pth";
        case UpscaleModel::BasicVSR:
            return "models/basicvsr/x4.pth";
        case UpscaleModel::EDVR:
            return "models/edvr/x4.pth";
        default:
            return "";
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Factory Functions
// ═══════════════════════════════════════════════════════════════════════════════

std::unique_ptr<Upscaler> CreateUpscaler(const UpscalerConfig& config) {
    auto upscaler = std::make_unique<Upscaler>();
    upscaler->Initialize(config);
    return upscaler;
}

std::unique_ptr<Upscaler> CreateESRGANUpscaler(u32 scale_factor) {
    UpscalerConfig config;
    config.model = UpscaleModel::ESRGAN;
    config.scale_factor = scale_factor;
    return CreateUpscaler(config);
}

std::unique_ptr<Upscaler> CreateRealESRGANUpscaler(u32 scale_factor) {
    UpscalerConfig config;
    config.model = UpscaleModel::RealESRGAN;
    config.scale_factor = scale_factor;
    return CreateUpscaler(config);
}

} // namespace aether
