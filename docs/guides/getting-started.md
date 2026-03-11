# Getting Started with AETHER Media Engine

This guide will help you get up and running with AETHER Media Engine.

## Prerequisites

### Required Dependencies

- **CMake** 3.25+
- **C++20** compatible compiler (GCC 11+, Clang 14+, MSVC 2022+)
- **Qt 6.5+** with Multimedia module

### Optional Dependencies

- **FFmpeg** 6.0+ (for codec support)
- **Vulkan SDK** (for hardware acceleration)
- **ONNX Runtime** (for AI features)
- **Ninja** (for faster builds)

## Quick Start

### 1. Clone the Repository

```bash
git clone https://github.com/devTechs001/aether-media-engine.git
cd aether-media-engine
git submodule update --init --recursive
```

### 2. Configure Build

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
```

### 3. Build

```bash
# Build the project
cmake --build . --config Release -j$(nproc)
```

### 4. Run

```bash
# Run the media player
./mediagui
```

## Platform-Specific Instructions

### Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt-get install -y \
    cmake \
    ninja-build \
    qt6-base-dev \
    qt6-multimedia-dev \
    qt6-quick-dev \
    libvulkan-dev \
    libavcodec-dev \
    libavformat-dev \
    libavutil-dev

# Build
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

### Windows

```powershell
# Install dependencies using vcpkg
vcpkg install qt6-multimedia ffmpeg vulkan onnxruntime

# Configure
cmake -B build -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build --config Release
```

### macOS

```bash
# Install dependencies using Homebrew
brew install cmake qt6 ffmpeg

# Configure
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -j$(sysctl -n hw.ncpu)
```

## Next Steps

- [Configuration Guide](configuration.md) - Learn about configuration options
- [Plugin Development](plugins.md) - Create your own plugins
- [Architecture Overview](../architecture/overview.md) - Understand the system design

## Troubleshooting

### Common Issues

#### Qt not found
```
CMake Error: Could not find Qt6
```
**Solution:** Install Qt6 Multimedia module and ensure CMAKE_PREFIX_PATH includes Qt installation.

#### FFmpeg not found
```
CMake Warning: FFmpeg not found, some features will be disabled
```
**Solution:** Install FFmpeg development packages or set FFmpeg_DIR.

#### Build fails with C++20 errors
**Solution:** Ensure your compiler supports C++20. Update to a newer version if needed.

## Getting Help

- Check the [FAQ](../guides/faq.md)
- Open an [Issue](https://github.com/devTechs001/aether-media-engine/issues)
- Join [Discussions](https://github.com/devTechs001/aether-media-engine/discussions)
