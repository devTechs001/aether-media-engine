// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/drm/drm_manager.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/core/types.hpp"
#include "aether/utils/logging.hpp"

#include <mutex>
#include <unordered_map>
#include <string>

namespace aether {

// ═══════════════════════════════════════════════════════════════════════════════
// DRM Manager Implementation
// ═══════════════════════════════════════════════════════════════════════════════

class DRMManager::Impl {
public:
    std::mutex mutex;
    bool initialized = false;
    std::unordered_map<std::string, std::string> sessions;
    std::string license_storage_path;
};

DRMManager& DRMManager::Instance() {
    static DRMManager instance;
    return instance;
}

DRMManager::DRMManager() : impl_(std::make_unique<Impl>()) {}

DRMManager::~DRMManager() {
    Shutdown();
}

Result<void> DRMManager::Initialize(const std::string& storage_path) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (impl_->initialized) {
        return Error::Make(ErrorCode::AlreadyInitialized, "DRM manager already initialized");
    }

    impl_->license_storage_path = storage_path;
    impl_->initialized = true;

    GetLogger().Info("DRM manager initialized");
    return {};
}

void DRMManager::Shutdown() {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (!impl_->initialized) {
        return;
    }

    // Close all sessions
    for (auto& [id, session] : impl_->sessions) {
        CloseSession(id);
    }
    impl_->sessions.clear();

    impl_->initialized = false;
    GetLogger().Info("DRM manager shutdown");
}

Result<std::string> DRMManager::CreateSession(DRMSystem system) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (!impl_->initialized) {
        return Error::Make(ErrorCode::NotInitialized, "DRM manager not initialized");
    }

    // Generate session ID
    std::string session_id = "session_" + std::to_string(impl_->sessions.size());

    // In production, would create actual DRM session
    // Widevine: CdmSession::Create()
    // PlayReady: IPlayReadySession::Create()
    // FairPlay: FPSessionCreate()

    impl_->sessions[session_id] = "";

    GetLogger().Debug("Created DRM session: {} (system: {})", 
                     session_id, static_cast<int>(system));

    return session_id;
}

Result<void> DRMManager::CloseSession(const std::string& session_id) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    auto it = impl_->sessions.find(session_id);
    if (it == impl_->sessions.end()) {
        return Error::Make(ErrorCode::NotFound, "Session not found");
    }

    // In production, would close actual DRM session
    impl_->sessions.erase(it);

    GetLogger().Debug("Closed DRM session: {}", session_id);
    return {};
}

Result<std::vector<u8>> DRMManager::GenerateChallenge(
    const std::string& session_id,
    const std::vector<u8>& init_data
) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    auto it = impl_->sessions.find(session_id);
    if (it == impl_->sessions.end()) {
        return Error::Make(ErrorCode::NotFound, "Session not found");
    }

    // In production, would generate actual DRM challenge
    // This is a placeholder implementation
    std::vector<u8> challenge = init_data;

    GetLogger().Debug("Generated DRM challenge for session: {}", session_id);
    return challenge;
}

Result<void> DRMManager::ProcessResponse(
    const std::string& session_id,
    const std::vector<u8>& response
) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    auto it = impl_->sessions.find(session_id);
    if (it == impl_->sessions.end()) {
        return Error::Make(ErrorCode::NotFound, "Session not found");
    }

    // In production, would process DRM response and extract keys
    it->second = std::string(response.begin(), response.end());

    GetLogger().Debug("Processed DRM response for session: {}", session_id);
    return {};
}

Result<DRMKeys> DRMManager::GetKeys(const std::string& session_id) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    auto it = impl_->sessions.find(session_id);
    if (it == impl_->sessions.end()) {
        return Error::Make(ErrorCode::NotFound, "Session not found");
    }

    // In production, would return actual decryption keys
    DRMKeys keys;
    keys.key_id.resize(16);
    keys.content_key.resize(16);

    return keys;
}

bool DRMManager::IsSessionActive(const std::string& session_id) const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->sessions.find(session_id) != impl_->sessions.end();
}

// ═══════════════════════════════════════════════════════════════════════════════
// Widevine Implementation
// ═══════════════════════════════════════════════════════════════════════════════

#ifdef AETHER_HAS_WIDEVINE

class WidevineDRM : public DRMManager {
public:
    Result<void> Initialize(const std::string& storage_path) override {
        // Initialize Widevine CDM
        // CdmInitialize()
        return DRMManager::Initialize(storage_path);
    }

    Result<std::string> CreateSession(DRMSystem system) override {
        (void)system;
        // Widevine-specific session creation
        return DRMManager::CreateSession(system);
    }
};

#endif

// ═══════════════════════════════════════════════════════════════════════════════
// Factory Functions
// ═══════════════════════════════════════════════════════════════════════════════

DRMManager& GetDRMManager() {
    return DRMManager::Instance();
}

} // namespace aether
