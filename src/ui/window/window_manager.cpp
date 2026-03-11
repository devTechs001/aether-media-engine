// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/ui/window/window_manager.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/core/types.hpp"
#include "aether/utils/logging.hpp"

#include <mutex>
#include <unordered_map>
#include <functional>

namespace aether {

// ═══════════════════════════════════════════════════════════════════════════════
// Window Manager Implementation
// ═══════════════════════════════════════════════════════════════════════════════

class WindowManager::Impl {
public:
    std::mutex mutex;
    bool initialized = false;
    std::unordered_map<u64, void*> windows;  // Platform-specific window handles
    u64 next_window_id = 1;
    
    // Event callbacks
    std::function<void(u64)> on_resize;
    std::function<void(u64)> on_close;
    std::function<void(u64, i32, i32)> on_mouse_move;
    std::function<void(u64, i32, i32)> on_mouse_click;
    std::function<void(u64, i32)> on_key_press;
};

WindowManager& WindowManager::Instance() {
    static WindowManager instance;
    return instance;
}

WindowManager::WindowManager() : impl_(std::make_unique<Impl>()) {}

WindowManager::~WindowManager() {
    Shutdown();
}

Result<void> WindowManager::Initialize() {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (impl_->initialized) {
        return Error::Make(ErrorCode::AlreadyInitialized, "Window manager already initialized");
    }

    // Initialize platform-specific windowing system
    // SDL, GLFW, or native (Win32, Cocoa, X11/Wayland)

    impl_->initialized = true;
    GetLogger().Info("Window manager initialized");
    return {};
}

void WindowManager::Shutdown() {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (!impl_->initialized) {
        return;
    }

    // Destroy all windows
    for (auto& [id, handle] : impl_->windows) {
        DestroyWindow(id);
    }

    impl_->initialized = false;
    GetLogger().Info("Window manager shutdown");
}

Result<u64> WindowManager::CreateWindow(const WindowConfig& config) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (!impl_->initialized) {
        return Error::Make(ErrorCode::NotInitialized, "Window manager not initialized");
    }

    u64 window_id = impl_->next_window_id++;

    // In production, would create actual window
    // SDL_CreateWindow, glfwCreateWindow, or native

    impl_->windows[window_id] = nullptr;  // Placeholder

    GetLogger().Info("Created window {} ({}x{})", window_id, config.width, config.height);
    return window_id;
}

Result<void> WindowManager::DestroyWindow(u64 window_id) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    auto it = impl_->windows.find(window_id);
    if (it == impl_->windows.end()) {
        return Error::Make(ErrorCode::NotFound, "Window not found");
    }

    // In production, would destroy actual window

    impl_->windows.erase(it);
    GetLogger().Debug("Destroyed window {}", window_id);
    return {};
}

Result<void> WindowManager::ShowWindow(u64 window_id) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    auto it = impl_->windows.find(window_id);
    if (it == impl_->windows.end()) {
        return Error::Make(ErrorCode::NotFound, "Window not found");
    }

    // In production, would show window

    return {};
}

Result<void> WindowManager::HideWindow(u64 window_id) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    auto it = impl_->windows.find(window_id);
    if (it == impl_->windows.end()) {
        return Error::Make(ErrorCode::NotFound, "Window not found");
    }

    // In production, would hide window

    return {};
}

Result<void> WindowManager::ResizeWindow(u64 window_id, u32 width, u32 height) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    auto it = impl_->windows.find(window_id);
    if (it == impl_->windows.end()) {
        return Error::Make(ErrorCode::NotFound, "Window not found");
    }

    // In production, would resize window

    GetLogger().Debug("Resized window {} to {}x{}", window_id, width, height);
    return {};
}

Result<void> WindowManager::SetTitle(u64 window_id, const std::string& title) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    auto it = impl_->windows.find(window_id);
    if (it == impl_->windows.end()) {
        return Error::Make(ErrorCode::NotFound, "Window not found");
    }

    // In production, would set window title

    return {};
}

Result<void> WindowManager::SetFullscreen(u64 window_id, bool fullscreen) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    auto it = impl_->windows.find(window_id);
    if (it == impl_->windows.end()) {
        return Error::Make(ErrorCode::NotFound, "Window not found");
    }

    // In production, would toggle fullscreen

    GetLogger().Debug("Window {} fullscreen: {}", window_id, fullscreen);
    return {};
}

Result<void*> WindowManager::GetWindowHandle(u64 window_id) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    auto it = impl_->windows.find(window_id);
    if (it == impl_->windows.end()) {
        return Error::Make(ErrorCode::NotFound, "Window not found");
    }

    return it->second;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Event Handling
// ═══════════════════════════════════════════════════════════════════════════════

void WindowManager::SetOnResize(std::function<void(u64)> callback) {
    impl_->on_resize = std::move(callback);
}

void WindowManager::SetOnClose(std::function<void(u64)> callback) {
    impl_->on_close = std::move(callback);
}

void WindowManager::SetOnMouseMove(std::function<void(u64, i32, i32)> callback) {
    impl_->on_mouse_move = std::move(callback);
}

void WindowManager::SetOnMouseClick(std::function<void(u64, i32, i32)> callback) {
    impl_->on_mouse_click = std::move(callback);
}

void WindowManager::SetOnKeyPress(std::function<void(u64, i32)> callback) {
    impl_->on_key_press = std::move(callback);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Event Pump
// ═══════════════════════════════════════════════════════════════════════════════

void WindowManager::PollEvents() {
    // In production, would poll for window events
    // SDL_PollEvent, glfwPollEvents, or native event loop
}

void WindowManager::WaitEvents() {
    // In production, would wait for window events
}

// ═══════════════════════════════════════════════════════════════════════════════
// Factory Functions
// ═══════════════════════════════════════════════════════════════════════════════

WindowManager& GetWindowManager() {
    return WindowManager::Instance();
}

} // namespace aether
