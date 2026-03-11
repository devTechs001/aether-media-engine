// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/plugin/plugin_manager.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/plugin/plugin_manager.hpp"
#include "aether/utils/logging.hpp"

#include <filesystem>
#include <mutex>
#include <unordered_map>

#ifdef AETHER_PLATFORM_WINDOWS
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

namespace aether {

namespace fs = std::filesystem;

class PluginManager::Impl {
public:
    std::unordered_map<std::string, Unique<Plugin>> plugins;
    std::unordered_map<std::string, fs::path> plugin_paths;
    std::string plugin_directory;
    std::mutex mutex;
    bool initialized = false;
};

PluginManager& PluginManager::Instance() {
    static PluginManager instance;
    return instance;
}

PluginManager::PluginManager() : impl_(std::make_unique<Impl>()) {}

PluginManager::~PluginManager() {
    Shutdown();
}

Result<void> PluginManager::Initialize(const std::string& plugin_dir) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (impl_->initialized) {
        return Error::Make(ErrorCode::AlreadyInitialized, "Plugin manager already initialized");
    }

    // Set plugin directory
    if (!plugin_dir.empty()) {
        impl_->plugin_directory = plugin_dir;
    } else {
        // Default directories
#ifdef AETHER_PLATFORM_WINDOWS
        const char* appdata = std::getenv("APPDATA");
        if (appdata) {
            impl_->plugin_directory = std::string(appdata) + "\\AetherMediaEngine\\plugins";
        }
#elif defined(AETHER_PLATFORM_LINUX)
        const char* xdg_data = std::getenv("XDG_DATA_HOME");
        if (xdg_data) {
            impl_->plugin_directory = std::string(xdg_data) + "/AetherMediaEngine/plugins";
        } else {
            const char* home = std::getenv("HOME");
            if (home) {
                impl_->plugin_directory = std::string(home) + "/.local/share/AetherMediaEngine/plugins";
            }
        }
#elif defined(AETHER_PLATFORM_MACOS)
        const char* home = std::getenv("HOME");
        if (home) {
            impl_->plugin_directory = std::string(home) + "/Library/Application Support/AetherMediaEngine/plugins";
        }
#endif
    }

    GetLogger().Info("Plugin directory: {}", impl_->plugin_directory);

    // Create directory if it doesn't exist
    try {
        fs::create_directories(impl_->plugin_directory);
    } catch (const std::exception& e) {
        GetLogger().Warn("Failed to create plugin directory: {}", e.what());
    }

    impl_->initialized = true;
    return {};
}

void PluginManager::Shutdown() {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    GetLogger().Info("Shutting down plugin manager...");

    // Unload all plugins
    for (auto& [id, plugin] : impl_->plugins) {
        if (plugin) {
            plugin->Shutdown();
        }
    }

    impl_->plugins.clear();
    impl_->plugin_paths.clear();
    impl_->initialized = false;
}

Result<void> PluginManager::LoadPlugin(const std::string& path) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (!fs::exists(path)) {
        return Error::Make(ErrorCode::NotFound, "Plugin file not found: {}", path);
    }

    GetLogger().Info("Loading plugin: {}", path);

    // Load the shared library
#ifdef AETHER_PLATFORM_WINDOWS
    HMODULE handle = LoadLibraryA(path.c_str());
    if (!handle) {
        return Error::Make(ErrorCode::PluginLoadFailed, "Failed to load plugin");
    }

    // Get plugin info function
    using GetInfoFunc = PluginInfo(*)();
    GetInfoFunc get_info = reinterpret_cast<GetInfoFunc>(GetProcAddress(handle, "GetPluginInfo"));
    if (!get_info) {
        FreeLibrary(handle);
        return Error::Make(ErrorCode::PluginLoadFailed, "Plugin missing GetPluginInfo");
    }

    // Get plugin create function
    using CreateFunc = Plugin*(*)();
    CreateFunc create = reinterpret_cast<CreateFunc>(GetProcAddress(handle, "CreatePlugin"));
    if (!create) {
        FreeLibrary(handle);
        return Error::Make(ErrorCode::PluginLoadFailed, "Plugin missing CreatePlugin");
    }

    // Get plugin info
    PluginInfo info = get_info();

    // Create plugin instance
    Plugin* plugin = create();
    if (!plugin) {
        FreeLibrary(handle);
        return Error::Make(ErrorCode::PluginLoadFailed, "Failed to create plugin instance");
    }

    // Initialize plugin
    auto result = plugin->Initialize();
    if (!result) {
        delete plugin;
        FreeLibrary(handle);
        return result;
    }

    // Store plugin
    impl_->plugins[info.id] = Unique<Plugin>(plugin);
    impl_->plugin_paths[info.id] = path;

#else // Linux/macOS
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        return Error::Make(ErrorCode::PluginLoadFailed, "Failed to load plugin: {}", dlerror());
    }

    // Get plugin info function
    using GetInfoFunc = PluginInfo(*)();
    GetInfoFunc get_info = reinterpret_cast<GetInfoFunc>(dlsym(handle, "GetPluginInfo"));
    if (!get_info) {
        dlclose(handle);
        return Error::Make(ErrorCode::PluginLoadFailed, "Plugin missing GetPluginInfo");
    }

    // Get plugin create function
    using CreateFunc = Plugin*(*)();
    CreateFunc create = reinterpret_cast<CreateFunc>(dlsym(handle, "CreatePlugin"));
    if (!create) {
        dlclose(handle);
        return Error::Make(ErrorCode::PluginLoadFailed, "Plugin missing CreatePlugin");
    }

    // Get plugin info
    PluginInfo info = get_info();

    // Create plugin instance
    Plugin* plugin = create();
    if (!plugin) {
        dlclose(handle);
        return Error::Make(ErrorCode::PluginLoadFailed, "Failed to create plugin instance");
    }

    // Initialize plugin
    auto result = plugin->Initialize();
    if (!result) {
        delete plugin;
        dlclose(handle);
        return result;
    }

    // Store plugin
    impl_->plugins[info.id] = Unique<Plugin>(plugin);
    impl_->plugin_paths[info.id] = path;
#endif

    GetLogger().Info("Plugin loaded: {} v{}", info.name, info.version);
    return {};
}

Result<void> PluginManager::UnloadPlugin(const std::string& plugin_id) {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    auto it = impl_->plugins.find(plugin_id);
    if (it == impl_->plugins.end()) {
        return Error::Make(ErrorCode::NotFound, "Plugin not found: {}", plugin_id);
    }

    GetLogger().Info("Unloading plugin: {}", plugin_id);

    // Shutdown plugin
    if (it->second) {
        it->second->Shutdown();
    }

    // Unload library
    auto path_it = impl_->plugin_paths.find(plugin_id);
    if (path_it != impl_->plugin_paths.end()) {
#ifdef AETHER_PLATFORM_WINDOWS
        // Windows: FreeLibrary would be called when plugin is destroyed
#else
        // Unix: dlclose would be called when plugin is destroyed
#endif
        impl_->plugin_paths.erase(path_it);
    }

    impl_->plugins.erase(it);
    return {};
}

std::vector<PluginInfo> PluginManager::GetLoadedPlugins() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    std::vector<PluginInfo> result;
    result.reserve(impl_->plugins.size());

    for (const auto& [id, plugin] : impl_->plugins) {
        if (plugin) {
            result.push_back(plugin->GetInfo());
        }
    }

    return result;
}

Plugin* PluginManager::GetPlugin(const std::string& plugin_id) const {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    auto it = impl_->plugins.find(plugin_id);
    if (it == impl_->plugins.end()) {
        return nullptr;
    }

    return it->second.get();
}

Result<void> PluginManager::ScanPlugins() {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    if (impl_->plugin_directory.empty()) {
        return Error::Make(ErrorCode::InvalidState, "Plugin directory not set");
    }

    GetLogger().Info("Scanning for plugins in: {}", impl_->plugin_directory);

    if (!fs::exists(impl_->plugin_directory)) {
        return {};
    }

    int loaded = 0;
    int failed = 0;

    for (const auto& entry : fs::directory_iterator(impl_->plugin_directory)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        std::string ext = entry.path().extension().string();

        // Check for plugin file extensions
#ifdef AETHER_PLATFORM_WINDOWS
        if (ext != ".dll" && ext != ".asi") {
            continue;
        }
#elif defined(AETHER_PLATFORM_MACOS)
        if (ext != ".dylib" && ext != ".bundle") {
            continue;
        }
#else
        if (ext != ".so") {
            continue;
        }
#endif

        auto result = const_cast<PluginManager*>(this)->LoadPlugin(entry.path().string());
        if (result) {
            loaded++;
        } else {
            GetLogger().Warn("Failed to load plugin {}: {}", entry.path().string(), result.error().message);
            failed++;
        }
    }

    GetLogger().Info("Plugin scan complete: {} loaded, {} failed", loaded, failed);
    return {};
}

Result<void> PluginManager::EnablePlugin(const std::string& plugin_id) {
    // For now, all loaded plugins are enabled
    // In production, this would set a flag in plugin config
    (void)plugin_id;
    return {};
}

Result<void> PluginManager::DisablePlugin(const std::string& plugin_id) {
    // For now, unload the plugin
    return UnloadPlugin(plugin_id);
}

std::vector<PluginInfo> PluginManager::GetAvailablePlugins() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);

    std::vector<PluginInfo> result;

    if (impl_->plugin_directory.empty() || !fs::exists(impl_->plugin_directory)) {
        return result;
    }

    for (const auto& entry : fs::directory_iterator(impl_->plugin_directory)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        // Check if already loaded
        bool already_loaded = false;
        for (const auto& [id, path] : impl_->plugin_paths) {
            if (path == entry.path()) {
                already_loaded = true;
                break;
            }
        }

        if (!already_loaded) {
            // Would load plugin info without loading full plugin
            // For now, just skip
        }
    }

    return result;
}

} // namespace aether
