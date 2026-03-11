// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/export.hpp
// DESCRIPTION: Export/import macros for shared library
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_EXPORT_HPP
#define AETHER_EXPORT_HPP

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
    #define AETHER_PLATFORM_WINDOWS
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #define AETHER_PLATFORM_IOS
    #else
        #define AETHER_PLATFORM_MACOS
    #endif
    #define AETHER_PLATFORM_APPLE
#elif defined(__ANDROID__)
    #define AETHER_PLATFORM_ANDROID
#elif defined(__linux__)
    #define AETHER_PLATFORM_LINUX
#elif defined(__EMSCRIPTEN__)
    #define AETHER_PLATFORM_WEB
#else
    #define AETHER_PLATFORM_UNKNOWN
#endif

// Architecture detection
#if defined(__x86_64__) || defined(_M_X64)
    #define AETHER_ARCH_X64
#elif defined(__i386__) || defined(_M_IX86)
    #define AETHER_ARCH_X86
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define AETHER_ARCH_ARM64
#elif defined(__arm__) || defined(_M_ARM)
    #define AETHER_ARCH_ARM
#elif defined(__wasm__)
    #define AETHER_ARCH_WASM
#endif

// Export/Import macros
#if defined(AETHER_PLATFORM_WINDOWS)
    #ifdef AETHER_BUILD_SHARED
        #ifdef AETHER_EXPORT
            #define AETHER_API __declspec(dllexport)
        #else
            #define AETHER_API __declspec(dllimport)
        #endif
    #else
        #define AETHER_API
    #endif
    #define AETHER_LOCAL
#else
    #if __GNUC__ >= 4 || defined(__clang__)
        #define AETHER_API __attribute__((visibility("default")))
        #define AETHER_LOCAL __attribute__((visibility("hidden")))
    #else
        #define AETHER_API
        #define AETHER_LOCAL
    #endif
#endif

// Calling convention
#if defined(AETHER_PLATFORM_WINDOWS)
    #define AETHER_CALL __cdecl
#else
    #define AETHER_CALL
#endif

// Deprecation macros
#if defined(__cplusplus) && __cplusplus >= 201402L
    #define AETHER_DEPRECATED [[deprecated]]
    #define AETHER_DEPRECATED_MSG(msg) [[deprecated(msg)]]
#elif defined(_MSC_VER)
    #define AETHER_DEPRECATED __declspec(deprecated)
    #define AETHER_DEPRECATED_MSG(msg) __declspec(deprecated(msg))
#elif defined(__GNUC__) || defined(__clang__)
    #define AETHER_DEPRECATED __attribute__((deprecated))
    #define AETHER_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#else
    #define AETHER_DEPRECATED
    #define AETHER_DEPRECATED_MSG(msg)
#endif

// Nodiscard
#if defined(__cplusplus) && __cplusplus >= 201703L
    #define AETHER_NODISCARD [[nodiscard]]
#else
    #define AETHER_NODISCARD
#endif

// Force inline
#if defined(_MSC_VER)
    #define AETHER_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define AETHER_FORCE_INLINE __attribute__((always_inline)) inline
#else
    #define AETHER_FORCE_INLINE inline
#endif

// No inline
#if defined(_MSC_VER)
    #define AETHER_NO_INLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
    #define AETHER_NO_INLINE __attribute__((noinline))
#else
    #define AETHER_NO_INLINE
#endif

// Alignment
#define AETHER_ALIGN(x) alignas(x)
#define AETHER_CACHE_LINE_SIZE 64
#define AETHER_CACHE_ALIGNED AETHER_ALIGN(AETHER_CACHE_LINE_SIZE)

// Branch prediction hints
#if defined(__GNUC__) || defined(__clang__)
    #define AETHER_LIKELY(x) __builtin_expect(!!(x), 1)
    #define AETHER_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define AETHER_LIKELY(x) (x)
    #define AETHER_UNLIKELY(x) (x)
#endif

// Restrict keyword
#if defined(_MSC_VER)
    #define AETHER_RESTRICT __restrict
#elif defined(__GNUC__) || defined(__clang__)
    #define AETHER_RESTRICT __restrict__
#else
    #define AETHER_RESTRICT
#endif

#endif // AETHER_EXPORT_HPP
