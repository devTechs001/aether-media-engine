// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/utils/memory.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/utils/memory.hpp"
#include <cstdlib>
#include <cstring>
#include <algorithm>

#ifdef AETHER_PLATFORM_WINDOWS
    #include <windows.h>
    #include <psapi.h>
#elif defined(AETHER_PLATFORM_APPLE)
    #include <mach/mach.h>
    #include <sys/sysctl.h>
#elif defined(AETHER_PLATFORM_LINUX)
    #include <unistd.h>
    #include <sys/resource.h>
    #include <fstream>
#endif

namespace aether {

// ═══════════════════════════════════════════════════════════════════════════════
// Global Memory Statistics
// ═══════════════════════════════════════════════════════════════════════════════

static std::atomic<usize> g_total_allocated{0};
static std::atomic<usize> g_total_freed{0};
static std::atomic<usize> g_current_usage{0};
static std::atomic<usize> g_peak_usage{0};
static std::atomic<usize> g_allocation_count{0};
static std::atomic<usize> g_deallocation_count{0};

static void TrackAllocation(usize size) {
    g_total_allocated += size;
    g_allocation_count++;
    usize current = (g_current_usage += size);

    usize peak = g_peak_usage.load();
    while (current > peak && !g_peak_usage.compare_exchange_weak(peak, current)) {}
}

static void TrackDeallocation(usize size) {
    g_total_freed += size;
    g_deallocation_count++;
    g_current_usage -= size;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Memory Statistics
// ═══════════════════════════════════════════════════════════════════════════════

usize GetMemoryUsage() {
    return g_current_usage.load();
}

usize GetVideoMemoryUsage() {
    // TODO: Implement GPU memory tracking
    return 0;
}

MemoryStats GetMemoryStats() {
    MemoryStats stats;
    stats.total_allocated = g_total_allocated.load();
    stats.total_freed = g_total_freed.load();
    stats.current_usage = g_current_usage.load();
    stats.peak_usage = g_peak_usage.load();
    stats.allocation_count = g_allocation_count.load();
    stats.deallocation_count = g_deallocation_count.load();
    return stats;
}

void ResetMemoryStats() {
    g_total_allocated = 0;
    g_total_freed = 0;
    g_current_usage = 0;
    g_peak_usage = 0;
    g_allocation_count = 0;
    g_deallocation_count = 0;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Memory Utilities
// ═══════════════════════════════════════════════════════════════════════════════

usize GetPageSize() {
#ifdef AETHER_PLATFORM_WINDOWS
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwPageSize;
#else
    return sysconf(_SC_PAGESIZE);
#endif
}

usize GetTotalPhysicalMemory() {
#ifdef AETHER_PLATFORM_WINDOWS
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
#elif defined(AETHER_PLATFORM_APPLE)
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    usize size = 0;
    size_t len = sizeof(size);
    sysctl(mib, 2, &size, &len, nullptr, 0);
    return size;
#else
    return sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE);
#endif
}

usize GetAvailablePhysicalMemory() {
#ifdef AETHER_PLATFORM_WINDOWS
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullAvailPhys;
#elif defined(AETHER_PLATFORM_APPLE)
    vm_statistics64_data_t vm_stats;
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    host_info64_t host_info = reinterpret_cast<host_info64_t>(&vm_stats);
    if (host_statistics64(mach_host_self(), HOST_VM_INFO64, host_info, &count) == KERN_SUCCESS) {
        return vm_stats.free_count * vm_kernel_page_size;
    }
    return 0;
#else
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    while (std::getline(meminfo, line)) {
        if (line.find("MemAvailable:") == 0) {
            usize kb = 0;
            std::sscanf(line.c_str(), "MemAvailable: %zu", &kb);
            return kb * 1024;
        }
    }
    return 0;
#endif
}

usize GetProcessMemoryUsage() {
#ifdef AETHER_PLATFORM_WINDOWS
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
    return 0;
#elif defined(AETHER_PLATFORM_APPLE)
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count) == KERN_SUCCESS) {
        return t_info.resident_size;
    }
    return 0;
#else
    std::ifstream status("/proc/self/status");
    std::string line;
    while (std::getline(status, line)) {
        if (line.find("VmRSS:") == 0) {
            usize kb = 0;
            std::sscanf(line.c_str(), "VmRSS: %zu", &kb);
            return kb * 1024;
        }
    }
    return 0;
#endif
}

} // namespace aether
