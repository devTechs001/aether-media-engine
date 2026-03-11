// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/render/subtitle/subtitle_renderer.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/core/types.hpp"
#include "aether/utils/logging.hpp"

#include <mutex>
#include <string>
#include <vector>

namespace aether {

// ═══════════════════════════════════════════════════════════════════════════════
// Subtitle Renderer Implementation
// ═══════════════════════════════════════════════════════════════════════════════

class SubtitleRenderer::Impl {
public:
    std::mutex mutex;
    bool initialized = false;
    
    // Current subtitle
    std::string current_text;
    i64 start_pts = 0;
    i64 end_pts = 0;
    
    // Style settings
    std::string font_family = "Arial";
    f32 font_size = 24.0f;
    Color font_color{1.0f, 1.0f, 1.0f, 1.0f};
    Color bg_color{0.0f, 0.0f, 0.0f, 0.7f};
    f32 outline_width = 2.0f;
    Color outline_color{0.0f, 0.0f, 0.0f, 1.0f};
    
    // Position
    enum class Position {
        Bottom,
        Top,
        Center
    } position = Position::Bottom;
    
    f32 margin_bottom = 50.0f;
    f32 margin_left = 50.0f;
    f32 margin_right = 50.0f;
    
    // Scale
    f32 scale = 1.0f;
    
    bool enabled = true;
};

SubtitleRenderer::SubtitleRenderer() : impl_(std::make_unique<Impl>()) {}

SubtitleRenderer::~SubtitleRenderer() = default;

Result<void> SubtitleRenderer::Initialize(const SubtitleConfig& config) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (impl_->initialized) {
        return Error::Make(ErrorCode::AlreadyInitialized, "Subtitle renderer already initialized");
    }

    impl_->font_family = config.font_family;
    impl_->font_size = config.font_size;
    impl_->font_color = config.font_color;
    impl_->bg_color = config.bg_color;
    impl_->outline_width = config.outline_width;
    impl_->outline_color = config.outline_color;
    impl_->position = static_cast<Impl::Position>(config.position);
    impl_->margin_bottom = config.margin_bottom;
    impl_->scale = config.scale;
    impl_->enabled = config.enabled;

    impl_->initialized = true;
    GetLogger().Info("Subtitle renderer initialized");
    return {};
}

void SubtitleRenderer::Shutdown() {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->initialized = false;
}

Result<void> SubtitleRenderer::Render(const SubtitleFrame& frame, void* target) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (!impl_->initialized || !impl_->enabled) {
        return {};
    }

    if (frame.type == SubtitleFrame::Type::Text) {
        return RenderTextSubtitle(frame, target);
    } else if (frame.type == SubtitleFrame::Type::Bitmap) {
        return RenderBitmapSubtitle(frame, target);
    }

    return {};
}

Result<void> SubtitleRenderer::RenderTextSubtitle(const SubtitleFrame& frame, void* target) {
    (void)target;

    // In production, would render text to surface
    // Using FreeType/Harfbuzz for text shaping
    // Blending with video frame

    impl_->current_text = frame.text;
    impl_->start_pts = frame.pts;
    impl_->end_pts = frame.end_pts;

    GetLogger().Debug("Rendered subtitle: {}", frame.text);
    return {};
}

Result<void> SubtitleRenderer::RenderBitmapSubtitle(const SubtitleFrame& frame, void* target) {
    (void)target;

    // In production, would blend bitmap subtitle with video
    // PGS, DVDSUB, etc.

    GetLogger().Debug("Rendered bitmap subtitle: {}x{}", 
                     frame.size.width, frame.size.height);
    return {};
}

Result<void> SubtitleRenderer::UpdateStyle(const SubtitleConfig& config) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    impl_->font_family = config.font_family;
    impl_->font_size = config.font_size;
    impl_->font_color = config.font_color;
    impl_->bg_color = config.bg_color;
    impl_->outline_width = config.outline_width;
    impl_->outline_color = config.outline_color;
    impl_->position = static_cast<Impl::Position>(config.position);
    impl_->margin_bottom = config.margin_bottom;
    impl_->scale = config.scale;

    return {};
}

Result<void> SubtitleRenderer::SetEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->enabled = enabled;
    return {};
}

bool SubtitleRenderer::IsEnabled() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->enabled;
}

Result<void> SubtitleRenderer::SetFont(const std::string& family, f32 size) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->font_family = family;
    impl_->font_size = size;
    return {};
}

Result<void> SubtitleRenderer::SetColors(Color font, Color bg, Color outline) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->font_color = font;
    impl_->bg_color = bg;
    impl_->outline_color = outline;
    return {};
}

Result<void> SubtitleRenderer::SetPosition(SubtitlePosition pos, f32 margin) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->position = static_cast<Impl::Position>(pos);
    impl_->margin_bottom = margin;
    return {};
}

Result<void> SubtitleRenderer::SetScale(f32 scale) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->scale = scale;
    return {};
}

std::unique_ptr<SubtitleRenderer> CreateSubtitleRenderer() {
    return std::make_unique<SubtitleRenderer>();
}

} // namespace aether
