// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/audio/audio_device.hpp
// DESCRIPTION: Audio device interface
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_AUDIO_AUDIO_DEVICE_HPP
#define AETHER_AUDIO_AUDIO_DEVICE_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <string>
#include <vector>

namespace aether {

/**
 * @enum DeviceType
 * @brief Audio device type
 */
enum class DeviceType : u8 {
    Input,
    Output,
    FullDuplex
};

/**
 * @struct AudioDeviceInfo
 * @brief Audio device information
 */
struct AETHER_API AudioDeviceInfo {
    std::string id;
    std::string name;
    std::string description;
    DeviceType type = DeviceType::Output;
    
    // Capabilities
    u32 min_sample_rate = 0;
    u32 max_sample_rate = 0;
    u32 min_channels = 0;
    u32 max_channels = 0;
    u32 default_sample_rate = 48000;
    u32 default_channels = 2;
    
    // Default device flags
    bool is_default = false;
    bool is_system_default = false;
};

/**
 * @class AudioDeviceManager
 * @brief Manage audio devices
 */
class AETHER_API AudioDeviceManager {
public:
    /**
     * @brief Get singleton instance
     */
    static AudioDeviceManager& Instance();

    /**
     * @brief Get all audio devices
     */
    [[nodiscard]] std::vector<AudioDeviceInfo> GetDevices() const;

    /**
     * @brief Get output devices
     */
    [[nodiscard]] std::vector<AudioDeviceInfo> GetOutputDevices() const;

    /**
     * @brief Get input devices
     */
    [[nodiscard]] std::vector<AudioDeviceInfo> GetInputDevices() const;

    /**
     * @brief Get default output device
     */
    [[nodiscard]] AudioDeviceInfo GetDefaultOutputDevice() const;

    /**
     * @brief Get default input device
     */
    [[nodiscard]] AudioDeviceInfo GetDefaultInputDevice() const;

    /**
     * @brief Set default output device
     */
    Result<void> SetDefaultOutputDevice(const std::string& device_id);

    /**
     * @brief Set default input device
     */
    Result<void> SetDefaultInputDevice(const std::string& device_id);

    /**
     * @brief Refresh device list
     */
    void RefreshDevices();

private:
    AudioDeviceManager() = default;
    ~AudioDeviceManager() = default;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace aether

#endif // AETHER_AUDIO_AUDIO_DEVICE_HPP
