// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/analytics/analytics_engine.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/core/types.hpp"
#include "aether/utils/logging.hpp"

#include <mutex>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <vector>

namespace aether {

// ═══════════════════════════════════════════════════════════════════════════════
// Analytics Engine Implementation
// ═══════════════════════════════════════════════════════════════════════════════

class AnalyticsEngine::Impl {
public:
    std::mutex mutex;
    bool initialized = false;
    std::atomic<bool> enabled{true};
    std::string endpoint;
    
    // Metrics storage
    std::unordered_map<std::string, i64> counters;
    std::unordered_map<std::string, f64> gauges;
    std::unordered_map<std::string, std::vector<f64>> histograms;
    
    // Session info
    std::string session_id;
    std::chrono::steady_clock::time_point session_start;
};

AnalyticsEngine& AnalyticsEngine::Instance() {
    static AnalyticsEngine instance;
    return instance;
}

AnalyticsEngine::AnalyticsEngine() : impl_(std::make_unique<Impl>()) {}

AnalyticsEngine::~AnalyticsEngine() {
    Shutdown();
}

Result<void> AnalyticsEngine::Initialize(const AnalyticsConfig& config) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (impl_->initialized) {
        return Error::Make(ErrorCode::AlreadyInitialized, "Analytics engine already initialized");
    }

    impl_->endpoint = config.endpoint;
    impl_->enabled = config.enabled;
    impl_->session_id = GenerateSessionId();
    impl_->session_start = std::chrono::steady_clock::now();

    impl_->initialized = true;

    if (impl_->enabled) {
        GetLogger().Info("Analytics engine initialized");
    }

    return {};
}

void AnalyticsEngine::Shutdown() {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (!impl_->initialized) {
        return;
    }

    // Flush remaining metrics
    Flush();

    impl_->initialized = false;
    GetLogger().Info("Analytics engine shutdown");
}

void AnalyticsEngine::Enable() {
    impl_->enabled = true;
}

void AnalyticsEngine::Disable() {
    impl_->enabled = false;
}

bool AnalyticsEngine::IsEnabled() const {
    return impl_->enabled.load();
}

// ═══════════════════════════════════════════════════════════════════════════════
// Counter Metrics
// ═══════════════════════════════════════════════════════════════════════════════

void AnalyticsEngine::IncrementCounter(const std::string& name, i64 value) {
    if (!impl_->enabled) return;

    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->counters[name] += value;
}

void AnalyticsEngine::SetCounter(const std::string& name, i64 value) {
    if (!impl_->enabled) return;

    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->counters[name] = value;
}

i64 AnalyticsEngine::GetCounter(const std::string& name) const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    auto it = impl_->counters.find(name);
    return (it != impl_->counters.end()) ? it->second : 0;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Gauge Metrics
// ═══════════════════════════════════════════════════════════════════════════════

void AnalyticsEngine::SetGauge(const std::string& name, f64 value) {
    if (!impl_->enabled) return;

    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->gauges[name] = value;
}

f64 AnalyticsEngine::GetGauge(const std::string& name) const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    auto it = impl_->gauges.find(name);
    return (it != impl_->gauges.end()) ? it->second : 0.0;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Histogram Metrics
// ═══════════════════════════════════════════════════════════════════════════════

void AnalyticsEngine::RecordHistogram(const std::string& name, f64 value) {
    if (!impl_->enabled) return;

    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->histograms[name].push_back(value);

    // Limit histogram size
    auto& hist = impl_->histograms[name];
    if (hist.size() > 10000) {
        hist.erase(hist.begin());
    }
}

std::vector<f64> AnalyticsEngine::GetHistogram(const std::string& name) const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    auto it = impl_->histograms.find(name);
    return (it != impl_->histograms.end()) ? it->second : std::vector<f64>{};
}

// ═══════════════════════════════════════════════════════════════════════════════
// Playback Events
// ═══════════════════════════════════════════════════════════════════════════════

void AnalyticsEngine::TrackPlaybackStart(const std::string& content_id, const std::string& source) {
    if (!impl_->enabled) return;

    IncrementCounter("playback_starts");
    IncrementCounter("playback_starts_" + source);

    GetLogger().Debug("Tracked playback start: {} ({})", content_id, source);
}

void AnalyticsEngine::TrackPlaybackEnd(const std::string& content_id, f64 duration_seconds, bool completed) {
    if (!impl_->enabled) return;

    IncrementCounter("playback_ends");
    RecordHistogram("playback_duration", duration_seconds);

    if (completed) {
        IncrementCounter("playback_completions");
    }

    GetLogger().Debug("Tracked playback end: {} ({}s, completed={})", 
                     content_id, duration_seconds, completed);
}

void AnalyticsEngine::TrackBuffering(i64 duration_ms, i64 position_ms) {
    if (!impl_->enabled) return;

    IncrementCounter("buffering_events");
    RecordHistogram("buffering_duration", static_cast<f64>(duration_ms));

    GetLogger().Debug("Tracked buffering: {}ms at {}ms", duration_ms, position_ms);
}

void AnalyticsEngine::TrackError(const std::string& error_code, const std::string& context) {
    if (!impl_->enabled) return;

    IncrementCounter("errors");
    IncrementCounter("errors_" + error_code);

    GetLogger().Debug("Tracked error: {} ({})", error_code, context);
}

void AnalyticsEngine::TrackQualityChange(const std::string& from_quality, const std::string& to_quality) {
    if (!impl_->enabled) return;

    IncrementCounter("quality_changes");

    GetLogger().Debug("Tracked quality change: {} -> {}", from_quality, to_quality);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Flush and Export
// ═══════════════════════════════════════════════════════════════════════════════

void AnalyticsEngine::Flush() {
    if (!impl_->enabled) return;

    std::lock_guard<std::mutex> lock(impl_->mutex);

    // In production, would send metrics to endpoint
    // For now, just log summary

    GetLogger().Debug("Analytics flush: {} counters, {} gauges, {} histograms",
                     impl_->counters.size(), impl_->gauges.size(), impl_->histograms.size());

    // Clear histograms after flush
    impl_->histograms.clear();
}

std::string AnalyticsEngine::GenerateSessionId() const {
    // Generate unique session ID
    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
    
    return "sess_" + std::to_string(millis);
}

AnalyticsConfig AnalyticsEngine::GetConfig() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    AnalyticsConfig config;
    config.enabled = impl_->enabled.load();
    config.endpoint = impl_->endpoint;
    return config;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Factory Functions
// ═══════════════════════════════════════════════════════════════════════════════

AnalyticsEngine& GetAnalyticsEngine() {
    return AnalyticsEngine::Instance();
}

} // namespace aether
