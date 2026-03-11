// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/plugin/plugin.hpp
// DESCRIPTION: Plugin interface
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_PLUGIN_PLUGIN_HPP
#define AETHER_PLUGIN_PLUGIN_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <string>
#include <memory>
#include <functional>

namespace aether {

/**
 * @enum PluginType
 * @brief Plugin type
 */
enum class PluginType : u8 {
    Demuxer,
    Decoder,
    Encoder,
    Filter,
    Renderer,
    Protocol,
    AI,
    DRM,
    UI,
    Other
};

/**
 * @struct PluginInfo
 * @brief Plugin information
 */
struct AETHER_API PluginInfo {
    std::string id;
    std::string name;
    std::string description;
    std::string version;
    std::string author;
    std::string license;
    
    PluginType type = PluginType::Other;
    
    // Dependencies
    std::vector<std::string> dependencies;
    
    // Capabilities
    std::vector<std::string> capabilities;
    
    // Platform
    std::string platform;
    std::string architecture;
};

/**
 * @class Plugin
 * @brief Base plugin interface
 */
class AETHER_API Plugin {
public:
    virtual ~Plugin() = default;

    /**
     * @brief Get plugin info
     */
    [[nodiscard]] virtual PluginInfo GetInfo() const = 0;

    /**
     * @brief Initialize plugin
     */
    virtual Result<void> Initialize() = 0;

    /**
     * @brief Shutdown plugin
     */
    virtual void Shutdown() = 0;

    /**
     * @brief Get plugin handle
     */
    [[nodiscard]] virtual void* GetHandle() const = 0;
};

/**
 * @brief Plugin factory function type
 */
using PluginFactory = std::function<std::unique_ptr<Plugin>()>;

/**
 * @brief Register plugin type
 */
AETHER_API void RegisterPluginType(PluginType type, const std::string& id, PluginFactory factory);

/**
 * @brief Create plugin by ID
 */
AETHER_API std::unique_ptr<Plugin> CreatePlugin(const std::string& id);

// Plugin registration macro
#define AETHER_REGISTER_PLUGIN(type, id, plugin_class) \
    namespace { \
        struct plugin_class##Registrar { \
            plugin_class##Registrar() { \
                RegisterPluginType(type, id, []() { return std::make_unique<plugin_class>(); }); \
            } \
        } plugin_class##RegistrarInstance; \
    }

} // namespace aether

#endif // AETHER_PLUGIN_PLUGIN_HPP
