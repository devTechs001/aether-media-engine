// ═══════════════════════════════════════════════════════════════════════════════
// FILE: src/utils/threading.cpp
// ═══════════════════════════════════════════════════════════════════════════════

#include "aether/utils/threading.hpp"

#ifdef AETHER_PLATFORM_WINDOWS
    #include <windows.h>
    #include <processthreadsapi.h>
#elif defined(AETHER_PLATFORM_APPLE)
    #include <pthread.h>
#elif defined(AETHER_PLATFORM_LINUX)
    #include <pthread.h>
    #include <sched.h>
    #include <sys/prctl.h>
#endif

namespace aether {

// ═══════════════════════════════════════════════════════════════════════════════
// Thread Utilities Implementation
// ═══════════════════════════════════════════════════════════════════════════════

u32 GetHardwareConcurrency() {
    u32 count = std::thread::hardware_concurrency();
    return count > 0 ? count : 1;
}

void SetThreadName(std::thread& thread, const std::string& name) {
#ifdef AETHER_PLATFORM_WINDOWS
    using SetThreadDescriptionFunc = HRESULT(WINAPI*)(HANDLE, PCWSTR);
    static auto set_thread_description = reinterpret_cast<SetThreadDescriptionFunc>(
        GetProcAddress(GetModuleHandleA("kernel32.dll"), "SetThreadDescription")
    );

    if (set_thread_description) {
        std::wstring wname(name.begin(), name.end());
        set_thread_description(thread.native_handle(), wname.c_str());
    }
#elif defined(AETHER_PLATFORM_LINUX)
    pthread_setname_np(thread.native_handle(), name.c_str());
#else
    (void)thread;
    (void)name;
#endif
}

void SetCurrentThreadName(const std::string& name) {
#ifdef AETHER_PLATFORM_WINDOWS
    using SetThreadDescriptionFunc = HRESULT(WINAPI*)(HANDLE, PCWSTR);
    static auto set_thread_description = reinterpret_cast<SetThreadDescriptionFunc>(
        GetProcAddress(GetModuleHandleA("kernel32.dll"), "SetThreadDescription")
    );

    if (set_thread_description) {
        std::wstring wname(name.begin(), name.end());
        set_thread_description(GetCurrentThread(), wname.c_str());
    }
#elif defined(AETHER_PLATFORM_APPLE)
    pthread_setname_np(name.c_str());
#elif defined(AETHER_PLATFORM_LINUX)
    prctl(PR_SET_NAME, name.c_str(), 0, 0, 0);
#endif
}

std::string GetCurrentThreadName() {
#ifdef AETHER_PLATFORM_WINDOWS
    using GetThreadDescriptionFunc = HRESULT(WINAPI*)(HANDLE, PWSTR*);
    static auto get_thread_description = reinterpret_cast<GetThreadDescriptionFunc>(
        GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetThreadDescription")
    );

    if (get_thread_description) {
        PWSTR name;
        if (SUCCEEDED(get_thread_description(GetCurrentThread(), &name))) {
            std::wstring wname(name);
            LocalFree(name);
            return std::string(wname.begin(), wname.end());
        }
    }
    return "";
#elif defined(AETHER_PLATFORM_APPLE)
    char name[64];
    pthread_getname_np(pthread_self(), name, sizeof(name));
    return name;
#elif defined(AETHER_PLATFORM_LINUX)
    char name[16];
    pthread_getname_np(pthread_self(), name, sizeof(name));
    return name;
#else
    return "";
#endif
}

// ═══════════════════════════════════════════════════════════════════════════════
// Semaphore Implementation
// ═══════════════════════════════════════════════════════════════════════════════

Semaphore::Semaphore(u32 initial_count) : count_(initial_count) {}

void Semaphore::Acquire() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return count_ > 0; });
    --count_;
}

bool Semaphore::TryAcquire() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (count_ > 0) {
        --count_;
        return true;
    }
    return false;
}

bool Semaphore::TryAcquireFor(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!cv_.wait_for(lock, timeout, [this] { return count_ > 0; })) {
        return false;
    }
    --count_;
    return true;
}

void Semaphore::Release(u32 count) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        count_ += count;
    }
    for (u32 i = 0; i < count; ++i) {
        cv_.notify_one();
    }
}

u32 Semaphore::Count() const noexcept {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
    return count_;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Event Implementation
// ═══════════════════════════════════════════════════════════════════════════════

Event::Event(bool manual_reset, bool initial_state)
    : signaled_(initial_state)
    , manual_reset_(manual_reset) {}

void Event::Set() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        signaled_ = true;
    }
    if (manual_reset_) {
        cv_.notify_all();
    } else {
        cv_.notify_one();
    }
}

void Event::Reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    signaled_ = false;
}

void Event::Wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return signaled_; });
    if (!manual_reset_) {
        signaled_ = false;
    }
}

bool Event::Wait(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    bool result = cv_.wait_for(lock, timeout, [this] { return signaled_; });
    if (result && !manual_reset_) {
        signaled_ = false;
    }
    return result;
}

bool Event::IsSet() const noexcept {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
    return signaled_;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Barrier Implementation
// ═══════════════════════════════════════════════════════════════════════════════

Barrier::Barrier(u32 count)
    : total_(count)
    , waiting_(0)
    , generation_(0) {}

void Barrier::Wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    u32 gen = generation_;

    if (++waiting_ == total_) {
        // Last thread arrives, release all
        waiting_ = 0;
        ++generation_;
        cv_.notify_all();
    } else {
        // Wait for others
        cv_.wait(lock, [this, gen] { return gen != generation_; });
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Spinlock Implementation
// ═══════════════════════════════════════════════════════════════════════════════

void Spinlock::lock() {
    while (flag_.test_and_set(std::memory_order_acquire)) {
        // Spin
#ifdef __cpp_lib_hardware_interference_size
        constexpr auto cache_line_size = std::hardware_destructive_interference_size;
#else
        constexpr auto cache_line_size = 64;
#endif
        alignas(cache_line_size) static char padding[cache_line_size];
        (void)padding;
    }
}

void Spinlock::unlock() {
    flag_.clear(std::memory_order_release);
}

bool Spinlock::try_lock() {
    return !flag_.test_and_set(std::memory_order_acquire);
}

} // namespace aether
