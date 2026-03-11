# Building AETHER Media Engine

This document provides detailed build instructions for all supported platforms.

## Build Options

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | Release | Build type (Debug/Release/RelWithDebInfo) |
| `AETHER_WITH_FFMPEG` | ON | Enable FFmpeg support |
| `AETHER_WITH_VULKAN` | ON | Enable Vulkan rendering |
| `AETHER_WITH_ONNX` | ON | Enable ONNX Runtime for AI |
| `AETHER_WITH_CUDA` | OFF | Enable CUDA support |
| `AETHER_WITH_DRM` | ON | Enable DRM support |
| `ENABLE_SANITIZERS` | OFF | Enable address/UB sanitizers |

### Example Configurations

#### Debug Build with Sanitizers
```bash
cmake -B build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_SANITIZERS=ON \
    -DAETHER_WITH_FFMPEG=ON
```

#### Release Build with LTO
```bash
cmake -B build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

#### Minimal Build
```bash
cmake -B build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DAETHER_WITH_FFMPEG=OFF \
    -DAETHER_WITH_VULKAN=OFF \
    -DAETHER_WITH_ONNX=OFF \
    -DAETHER_WITH_DRM=OFF
```

## Platform-Specific Guides

### Ubuntu 22.04+

```bash
# Install build tools
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    git

# Install Qt6
sudo apt-get install -y \
    qt6-base-dev \
    qt6-multimedia-dev \
    qt6-quick-dev \
    qt6-svg-dev

# Install optional dependencies
sudo apt-get install -y \
    libavcodec-dev \
    libavformat-dev \
    libavutil-dev \
    libswresample-dev \
    libswscale-dev \
    libvulkan-dev \
    libonnxruntime-dev

# Configure and build
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

### Fedora 38+

```bash
# Install build tools
sudo dnf install -y \
    cmake \
    ninja-build \
    gcc-c++ \
    git

# Install Qt6
sudo dnf install -y \
    qt6-qtbase-devel \
    qt6-qtmultimedia-devel \
    qt6-qtquickcontrols2-devel

# Install optional dependencies
sudo dnf install -y \
    ffmpeg-devel \
    vulkan-loader-devel \
    onnxruntime-devel

# Configure and build
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

### Windows (MSVC)

```powershell
# Install Visual Studio 2022 with C++ workload
# Install Qt6 using online installer or vcpkg

# Using vcpkg
vcpkg install qt6-multimedia ffmpeg vulkan onnxruntime

# Configure
cmake -B build `
    -G "Visual Studio 17 2022" `
    -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
    -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release -j$env:NUMBER_OF_PROCESSORS
```

### Windows (MinGW)

```powershell
# Install MSYS2 and MinGW-w64
# Install Qt6 for MinGW

pacman -S \
    mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-ninja \
    mingw-w64-x86_64-gcc \
    mingw-w64-x86_64-qt6-multimedia \
    mingw-w64-x86_64-ffmpeg

# Configure
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -j$(nproc)
```

### macOS (Homebrew)

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install dependencies
brew install cmake ninja qt6 ffmpeg vulkan-loader

# Configure
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=$(brew --prefix qt6)

# Build
cmake --build . -j$(sysctl -n hw.ncpu)
```

### macOS (MacPorts)

```bash
# Install dependencies
sudo port install cmake ninja qt6-mac ffmpeg vulkan-headers

# Configure
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=/opt/local

# Build
cmake --build . -j$(sysctl -n hw.ncpu)
```

### Android

```bash
# Set up Android NDK
export ANDROID_NDK=/path/to/android-ndk
export TOOLCHAIN=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64

# Configure for ARM64
cmake -B build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-21

# Build
cmake --build . -j$(nproc)
```

### iOS

```bash
# Configure for iOS
cmake -B build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/ios-arm64.cmake \
    -DIOS_PLATFORM=OS

# Build
cmake --build . -j$(sysctl -n hw.ncpu)
```

### WebAssembly (Emscripten)

```bash
# Source Emscripten environment
source /path/to/emsdk/emsdk_env.sh

# Configure
cmake -B build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/wasm-emscripten.cmake

# Build
cmake --build . -j$(nproc)
```

## Testing

### Run Unit Tests
```bash
cd build
ctest --output-on-failure
```

### Run Specific Test
```bash
ctest -R codec_test --verbose
```

### Run with Coverage
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCODE_COVERAGE=ON
cmake --build build
cd build
ctest
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage
```

## Installation

### Install Locally
```bash
cmake --install build --prefix ~/local
```

### Install System-wide (Linux/macOS)
```bash
sudo cmake --install build
```

### Create Package
```bash
cd build
cpack -G DEB    # Debian/Ubuntu
cpack -G RPM    # Fedora/RHEL
cpack -G TGZ    # Tarball
```

## Troubleshooting

### Common Build Errors

#### CMake can't find Qt6
```
Could not find a package configuration file provided by "Qt6"
```
**Solution:** Set `CMAKE_PREFIX_PATH` to Qt installation directory.

#### Missing FFmpeg headers
```
fatal error: libavcodec/avcodec.h: No such file or directory
```
**Solution:** Install FFmpeg development package.

#### Linker errors on Windows
```
unresolved external symbol
```
**Solution:** Ensure all dependencies are built with the same compiler/runtime.

### Performance Issues

#### Slow builds
- Use Ninja generator
- Enable ccache: `cmake -DCMAKE_CXX_COMPILER_LAUNCHER=ccache`
- Use precompiled headers

#### Large binary size
- Enable LTO: `-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON`
- Strip symbols: `strip mediagui`
- Use Release build type

## Build Output Structure

```
build/
├── mediagui              # Main executable
├── libaether.so          # Core library
├── plugins/              # Plugin directory
├── tests/                # Test executables
├── CMakeFiles/           # Build metadata
├── compile_commands.json # Compilation database
└── docs/                 # Generated documentation
```
