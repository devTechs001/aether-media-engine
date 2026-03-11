// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/ai/inference_engine.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/ai/inference_engine.hpp"
#include "aether/utils/logging.hpp"

#include <mutex>
#include <unordered_map>

#ifdef AETHER_HAS_ONNX
#include <onnxruntime_cxx_api.h>
#endif

namespace aether {

class InferenceEngine::Impl {
public:
    std::mutex mutex;
    bool initialized = false;
    InferenceConfig config;
    std::vector<InferenceBackend> available_backends;

#ifdef AETHER_HAS_ONNX
    Ort::Env onnx_env;
    Ort::SessionOptions session_options;
#endif
};

InferenceEngine& InferenceEngine::Instance() {
    static InferenceEngine instance;
    return instance;
}

InferenceEngine::InferenceEngine() : impl_(std::make_unique<Impl>()) {
#ifdef AETHER_HAS_ONNX
    impl_->onnx_env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "AETHER");
    impl_->session_options.SetIntraOpNumThreads(0);
    impl_->session_options.SetInterOpNumThreads(0);
    impl_->session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
#endif
}

InferenceEngine::~InferenceEngine() {
    Shutdown();
}

Result<void> InferenceEngine::Initialize(const InferenceConfig& config) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (impl_->initialized) {
        return Error::Make(ErrorCode::AlreadyInitialized, "Inference engine already initialized");
    }

    impl_->config = config;

    // Detect available backends
    impl_->available_backends.clear();

#ifdef AETHER_HAS_ONNX
    impl_->available_backends.push_back(InferenceBackend::ONNXRuntime);
    GetLogger().Info("ONNX Runtime backend available");
#endif

#ifdef AETHER_HAS_TENSORRT
    impl_->available_backends.push_back(InferenceBackend::TensorRT);
    GetLogger().Info("TensorRT backend available");
#endif

#ifdef AETHER_HAS_COREML
    impl_->available_backends.push_back(InferenceBackend::CoreML);
    GetLogger().Info("CoreML backend available");
#endif

#ifdef AETHER_HAS_OPENVINO
    impl_->available_backends.push_back(InferenceBackend::OpenVINO);
    GetLogger().Info("OpenVINO backend available");
#endif

    if (impl_->available_backends.empty()) {
        GetLogger().Warn("No AI inference backends available");
    } else {
        GetLogger().Info("Initialized {} AI inference backend(s)", impl_->available_backends.size());
    }

    impl_->initialized = true;
    return {};
}

void InferenceEngine::Shutdown() {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (!impl_->initialized) {
        return;
    }

    GetLogger().Info("Shutting down AI inference engine...");

    impl_->initialized = false;
}

bool InferenceEngine::IsInitialized() const {
    return impl_->initialized;
}

Result<std::unique_ptr<InferenceSession>> InferenceEngine::CreateSession() {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (!impl_->initialized) {
        return Error::Make(ErrorCode::NotInitialized, "Inference engine not initialized");
    }

    // Create session based on configured backend
    switch (impl_->config.backend) {
#ifdef AETHER_HAS_ONNX
        case InferenceBackend::ONNXRuntime: {
            auto session = std::make_unique<ONNXInferenceSession>();
            session->Initialize(impl_->config);
            return session;
        }
#endif
        default:
            return Error::Make(ErrorCode::NotSupported, "Requested backend not available");
    }
}

std::vector<InferenceBackend> InferenceEngine::GetAvailableBackends() {
    return InferenceEngine::Instance().impl_->available_backends;
}

std::string InferenceEngine::GetBackendName(InferenceBackend backend) {
    switch (backend) {
        case InferenceBackend::ONNXRuntime: return "ONNX Runtime";
        case InferenceBackend::TensorRT: return "TensorRT";
        case InferenceBackend::OpenVINO: return "OpenVINO";
        case InferenceBackend::CoreML: return "CoreML";
        case InferenceBackend::DirectML: return "DirectML";
        case InferenceBackend::NNAPI: return "NNAPI";
        default: return "Unknown";
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// ONNX Runtime Session Implementation
// ═══════════════════════════════════════════════════════════════════════════════

#ifdef AETHER_HAS_ONNX

class ONNXInferenceSession : public InferenceSession {
public:
    ONNXInferenceSession() = default;
    ~ONNXInferenceSession() override = default;

    Result<void> Initialize(const InferenceConfig& config) {
        config_ = config;
        return {};
    }

    Result<void> LoadModel(const std::string& model_path) override {
#ifdef AETHER_HAS_ONNX
        try {
            session_ = std::make_unique<Ort::Session>(
                InferenceEngine::Instance().impl_->onnx_env,
                model_path.c_str(),
                InferenceEngine::Instance().Instance().impl_->session_options
            );

            // Get input/output info
            size_t input_count = session_->GetInputCount();
            size_t output_count = session_->GetOutputCount();

            GetLogger().Debug("ONNX model loaded: {} inputs, {} outputs", input_count, output_count);
            return {};
        } catch (const Ort::Exception& e) {
            return Error::Make(ErrorCode::ModelLoadFailed, "Failed to load ONNX model: {}", e.what());
        }
#else
        (void)model_path;
        return Error::Make(ErrorCode::NotSupported, "ONNX Runtime not available");
#endif
    }

    Result<void> LoadModelFromData(std::span<const u8> model_data) override {
#ifdef AETHER_HAS_ONNX
        try {
            // Create session from memory
            session_ = std::make_unique<Ort::Session>(
                InferenceEngine::Instance().impl_->onnx_env,
                model_data.data(),
                model_data.size(),
                InferenceEngine::Instance().impl_->session_options
            );

            return {};
        } catch (const Ort::Exception& e) {
            return Error::Make(ErrorCode::ModelLoadFailed, "Failed to load ONNX model from data: {}", e.what());
        }
#else
        (void)model_data;
        return Error::Make(ErrorCode::NotSupported, "ONNX Runtime not available");
#endif
    }

    Result<void> Run(const std::vector<std::span<const f32>>& inputs,
                    std::vector<std::span<f32>>& outputs) override {
#ifdef AETHER_HAS_ONNX
        if (!session_) {
            return Error::Make(ErrorCode::NotInitialized, "Model not loaded");
        }

        try {
            // Create input/output tensors
            std::vector<Ort::Value> input_tensors;
            std::vector<Ort::Value> output_tensors;

            // Run inference
            session_->Run(Ort::RunOptions{nullptr},
                         input_names_.data(), input_tensors.data(), inputs.size(),
                         output_names_.data(), outputs.size());

            return {};
        } catch (const Ort::Exception& e) {
            return Error::Make(ErrorCode::InferenceError, "Inference failed: {}", e.what());
        }
#else
        (void)inputs;
        (void)outputs;
        return Error::Make(ErrorCode::NotSupported, "ONNX Runtime not available");
#endif
    }

    [[nodiscard]] std::vector<i64> GetInputShape(size_t index) const override {
#ifdef AETHER_HAS_ONNX
        if (!session_ || index >= session_->GetInputCount()) {
            return {};
        }
        // Would extract shape from tensor info
        return {};
#else
        (void)index;
        return {};
#endif
    }

    [[nodiscard]] std::vector<i64> GetOutputShape(size_t index) const override {
#ifdef AETHER_HAS_ONNX
        if (!session_ || index >= session_->GetOutputCount()) {
            return {};
        }
        // Would extract shape from tensor info
        return {};
#else
        (void)index;
        return {};
#endif
    }

private:
    InferenceConfig config_;
    std::unique_ptr<Ort::Session> session_;
    std::vector<const char*> input_names_;
    std::vector<const char*> output_names_;
};

#else

class ONNXInferenceSession : public InferenceSession {
public:
    Result<void> Initialize(const InferenceConfig& config) {
        (void)config;
        return Error::Make(ErrorCode::NotSupported, "ONNX Runtime not available");
    }

    Result<void> LoadModel(const std::string& model_path) override {
        (void)model_path;
        return Error::Make(ErrorCode::NotSupported, "ONNX Runtime not available");
    }

    Result<void> LoadModelFromData(std::span<const u8> model_data) override {
        (void)model_data;
        return Error::Make(ErrorCode::NotSupported, "ONNX Runtime not available");
    }

    Result<void> Run(const std::vector<std::span<const f32>>& inputs,
                    std::vector<std::span<f32>>& outputs) override {
        (void)inputs;
        (void)outputs;
        return Error::Make(ErrorCode::NotSupported, "ONNX Runtime not available");
    }

    [[nodiscard]] std::vector<i64> GetInputShape(size_t index) const override {
        (void)index;
        return {};
    }

    [[nodiscard]] std::vector<i64> GetOutputShape(size_t index) const override {
        (void)index;
        return {};
    }
};

#endif // AETHER_HAS_ONNX

} // namespace aether
