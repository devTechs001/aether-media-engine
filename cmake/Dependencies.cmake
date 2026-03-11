# Dependencies.cmake
# Dependency management for AETHER Media Engine

include(FetchContent)

# Required dependencies
find_package(Threads REQUIRED)

# Qt6 (required for GUI)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Multimedia Widgets)

# Optional dependencies
find_package(FFmpeg QUIET COMPONENTS avcodec avformat avutil)
find_package(Vulkan QUIET)
find_package(ONNXRuntime QUIET)

# Fetch external dependencies
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.1.0
)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)

FetchContent_Declare(
    DearImGui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
)

# Make available
FetchContent_MakeAvailable(spdlog fmt nlohmann_json)

# Options for features
option(AETHER_WITH_FFMPEG "Build with FFmpeg support" ON)
option(AETHER_WITH_VULKAN "Build with Vulkan rendering" ON)
option(AETHER_WITH_ONNX "Build with ONNX Runtime for AI" ON)
option(AETHER_WITH_CUDA "Build with CUDA support" OFF)
option(AETHER_WITH_DRM "Build with DRM support" ON)

# Feature-based dependencies
if(AETHER_WITH_FFMPEG AND FFmpeg_FOUND)
    add_compile_definitions(AETHER_HAS_FFMPEG)
    message(STATUS "FFmpeg support enabled")
endif()

if(AETHER_WITH_VULKAN AND Vulkan_FOUND)
    add_compile_definitions(AETHER_HAS_VULKAN)
    message(STATUS "Vulkan support enabled")
endif()

if(AETHER_WITH_ONNX AND ONNXRuntime_FOUND)
    add_compile_definitions(AETHER_HAS_ONNX)
    message(STATUS "ONNX Runtime support enabled")
endif()

# Platform-specific dependencies
if(WIN32)
    # Windows-specific libraries
    set(PLATFORM_LIBS ws2_32 crypt32 wsock32)
elseif(APPLE)
    # macOS-specific frameworks
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(METAL_FRAMEWORK Metal)
    find_library(METALKIT_FRAMEWORK MetalKit)
    set(PLATFORM_LIBS ${COCOA_FRAMEWORK} ${METAL_FRAMEWORK} ${METALKIT_FRAMEWORK})
else()
    # Linux-specific
    find_package(X11)
    find_library(WAYLAND_CLIENT wayland-client)
endif()
