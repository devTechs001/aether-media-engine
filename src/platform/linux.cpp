// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/platform/linux.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/core/types.hpp"
#include "aether/utils/logging.hpp"

#ifdef AETHER_PLATFORM_LINUX

#include <unistd.h>
#include <sys/utsname.h>
#include <cstring>

namespace aether {
namespace platform {

// ═══════════════════════════════════════════════════════════════════════════════
// Platform Detection
// ═══════════════════════════════════════════════════════════════════════════════

std::string GetPlatformName() {
    return "Linux";
}

std::string GetPlatformVersion() {
    struct utsname buf;
    if (uname(&buf) == 0) {
        return std::string(buf.release);
    }
    return "unknown";
}

std::string GetArchitecture() {
    struct utsname buf;
    if (uname(&buf) == 0) {
        return std::string(buf.machine);
    }
    return "unknown";
}

// ═══════════════════════════════════════════════════════════════════════════════
// System Information
// ═══════════════════════════════════════════════════════════════════════════════

u32 GetCPUCount() {
    return static_cast<u32>(sysconf(_SC_NPROCESSORS_ONLN));
}

u64 GetTotalMemory() {
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    if (pages > 0 && page_size > 0) {
        return static_cast<u64>(pages) * static_cast<u64>(page_size);
    }
    return 0;
}

u64 GetAvailableMemory() {
    long pages = sysconf(_SC_AVPHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    if (pages > 0 && page_size > 0) {
        return static_cast<u64>(pages) * static_cast<u64>(page_size);
    }
    return 0;
}

// ═══════════════════════════════════════════════════════════════════════════════
// File System
// ═══════════════════════════════════════════════════════════════════════════════

std::string GetConfigDirectory() {
    const char* xdg_config = std::getenv("XDG_CONFIG_HOME");
    if (xdg_config && xdg_config[0] != '\0') {
        return std::string(xdg_config) + "/AetherMediaEngine";
    }
    
    const char* home = std::getenv("HOME");
    if (home && home[0] != '\0') {
        return std::string(home) + "/.config/AetherMediaEngine";
    }
    
    return "/tmp/AetherMediaEngine";
}

std::string GetDataDirectory() {
    const char* xdg_data = std::getenv("XDG_DATA_HOME");
    if (xdg_data && xdg_data[0] != '\0') {
        return std::string(xdg_data) + "/AetherMediaEngine";
    }
    
    const char* home = std::getenv("HOME");
    if (home && home[0] != '\0') {
        return std::string(home) + "/.local/share/AetherMediaEngine";
    }
    
    return "/tmp/AetherMediaEngine";
}

std::string GetCacheDirectory() {
    const char* xdg_cache = std::getenv("XDG_CACHE_HOME");
    if (xdg_cache && xdg_cache[0] != '\0') {
        return std::string(xdg_cache) + "/AetherMediaEngine";
    }
    
    const char* home = std::getenv("HOME");
    if (home && home[0] != '\0') {
        return std::string(home) + "/.cache/AetherMediaEngine";
    }
    
    return "/tmp/AetherMediaEngine";
}

std::string GetTempDirectory() {
    const char* tmp = std::getenv("TMPDIR");
    if (tmp && tmp[0] != '\0') {
        return std::string(tmp) + "/AetherMediaEngine";
    }
    return "/tmp/AetherMediaEngine";
}

// ═══════════════════════════════════════════════════════════════════════════════
// Display Information
// ═══════════════════════════════════════════════════════════════════════════════

struct DisplayInfo {
    u32 width = 0;
    u32 height = 0;
    u32 refresh_rate = 0;
    bool hdr_capable = false;
};

DisplayInfo GetPrimaryDisplayInfo() {
    DisplayInfo info;
    
    // In production, would query X11/Wayland for display info
    // For now, return defaults
    info.width = 1920;
    info.height = 1080;
    info.refresh_rate = 60;
    info.hdr_capable = false;
    
    return info;
}

bool IsWaylandAvailable() {
    const char* session = std::getenv("XDG_SESSION_TYPE");
    return session && std::strcmp(session, "wayland") == 0;
}

bool IsX11Available() {
    const char* display = std::getenv("DISPLAY");
    return display && display[0] != '\0';
}

} // namespace platform
} // namespace aether

#endif // AETHER_PLATFORM_LINUX
