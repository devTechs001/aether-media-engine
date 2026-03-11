// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/ai/inference_engine.hpp
// DESCRIPTION: AI inference engine interface
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_AI_INFERENCE_ENGINE_HPP
#define AETHER_AI_INFERENCE_ENGINE_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <memory>
#include <string>
#include <vector>
#include <span>

namespace aether {

/**
 * @enum InferenceBackend
 * @brief AI inference backend
 */
enum class InferenceBackend : u8 {
    ONNXRuntime,
    TensorRT,
    OpenVINO,
    CoreML,
    DirectML,
    NNAPI
};

/**
 * @struct InferenceConfig
 * @brief Inference engine configuration
 */
struct AETHER_API InferenceConfig {
    InferenceBackend backend = InferenceBackend::ONNXRuntime;
    
    // Device
    bool use_gpu = true;
    i32 gpu_index = 0;
    
    // Performance
    u32 num_threads = 0;  // 0 = auto
    bool enable_profiling = false;
    
    // Memory
    usize memory_limit_mb = 0;  // 0 = unlimited
    bool enable_memory_arena = true;
};

/**
 * @class InferenceSession
 * @brief AI inference session
 */
class AETHER_API InferenceSession {
public:
    virtual ~InferenceSession() = default;

    /**
     * @brief Load model from file
     */
    virtual Result<void> LoadModel(const std::string& model_path) = 0;

    /**
     * @brief Load model from data
     */
    virtual Result<void> LoadModelFromData(std::span<const u8> model_data) = 0;

    /**
     * @brief Run inference
     */
    virtual Result<void> Run(const std::vector<std::span<const f32>>& inputs,
                            std::vector<std::span<f32>>& outputs) = 0;

    /**
     * @brief Get input shape
     */
    [[nodiscard]] virtual std::vector<i64> GetInputShape(size_t index) const = 0;

    /**
     * @brief Get output shape
     */
    [[nodiscard]] virtual std::vector<i64> GetOutputShape(size_t index) const = 0;
};

/**
 * @class InferenceEngine
 * @brief AI inference engine manager
 */
class AETHER_API InferenceEngine {
public:
    /**
     * @brief Get singleton instance
     */
    static InferenceEngine& Instance();

    /**
     * @brief Initialize engine
     */
    Result<void> Initialize(const InferenceConfig& config = InferenceConfig());

    /**
     * @brief Shutdown engine
     */
    void Shutdown();

    /**
     * @brief Check if initialized
     */
    [[nodiscard]] bool IsInitialized() const;

    /**
     * @brief Create inference session
     */
    Result<std::unique_ptr<InferenceSession>> CreateSession();

    /**
     * @brief Get available backends
     */
    [[nodiscard]] static std::vector<InferenceBackend> GetAvailableBackends();

    /**
     * @brief Get backend name
     */
    [[nodiscard]] static std::string GetBackendName(InferenceBackend backend);

private:
    InferenceEngine() = default;
    ~InferenceEngine() = default;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace aether

#endif // AETHER_AI_INFERENCE_ENGINE_HPP
