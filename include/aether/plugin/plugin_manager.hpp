// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/plugin/plugin_manager.hpp
// DESCRIPTION: Plugin manager interface
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_PLUGIN_PLUGIN_MANAGER_HPP
#define AETHER_PLUGIN_PLUGIN_MANAGER_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"
#include "aether/plugin/plugin.hpp"

#include <string>
#include <vector>
#include <memory>

namespace aether {

/**
 * @class PluginManager
 * @brief Manage plugin loading and lifecycle
 */
class AETHER_API PluginManager {
public:
    /**
     * @brief Get singleton instance
     */
    static PluginManager& Instance();

    /**
     * @brief Initialize plugin manager
     */
    Result<void> Initialize(const std::string& plugin_dir = "");

    /**
     * @brief Shutdown plugin manager
     */
    void Shutdown();

    /**
     * @brief Load plugin from file
     */
    Result<void> LoadPlugin(const std::string& path);

    /**
     * @brief Unload plugin
     */
    Result<void> UnloadPlugin(const std::string& plugin_id);

    /**
     * @brief Get loaded plugins
     */
    [[nodiscard]] std::vector<PluginInfo> GetLoadedPlugins() const;

    /**
     * @brief Get plugin by ID
     */
    [[nodiscard]] Plugin* GetPlugin(const std::string& plugin_id) const;

    /**
     * @brief Scan plugin directory
     */
    Result<void> ScanPlugins();

    /**
     * @brief Enable plugin
     */
    Result<void> EnablePlugin(const std::string& plugin_id);

    /**
     * @brief Disable plugin
     */
    Result<void> DisablePlugin(const std::string& plugin_id);

    /**
     * @brief Get available plugins
     */
    [[nodiscard]] std::vector<PluginInfo> GetAvailablePlugins() const;

private:
    PluginManager() = default;
    ~PluginManager() = default;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace aether

#endif // AETHER_PLUGIN_PLUGIN_MANAGER_HPP
