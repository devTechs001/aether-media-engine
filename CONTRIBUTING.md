# CONTRIBUTING to AETHER Media Engine

Thank you for your interest in contributing to AETHER Media Engine! This document provides guidelines and instructions for contributing.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Making Changes](#making-changes)
- [Pull Request Guidelines](#pull-request-guidelines)
- [Coding Standards](#coding-standards)
- [Testing](#testing)
- [Documentation](#documentation)

---

## Code of Conduct

Please read and follow our [Code of Conduct](CODE_OF_CONDUCT.md). Be respectful and inclusive in all interactions.

---

## Getting Started

### 1. Fork the Repository

```bash
# Click "Fork" on GitHub, then clone your fork
git clone https://github.com/YOUR_USERNAME/aether-media-engine.git
cd aether-media-engine
```

### 2. Set Up Development Environment

```bash
# Install dependencies (Ubuntu example)
sudo apt-get install -y \
    cmake ninja-build \
    qt6-base-dev qt6-multimedia-dev \
    libvulkan-dev libgtest-dev

# Create build directory
mkdir build && cd build

# Configure with debug options
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
```

### 3. Create a Branch

```bash
git checkout -b feature/your-feature-name
```

---

## Development Setup

### Required Tools

- CMake 3.25+
- C++20 compiler (GCC 11+, Clang 14+, MSVC 2022+)
- Ninja (recommended) or Make
- Git

### Recommended Tools

- clangd (for IDE integration)
- clang-format (for code formatting)
- clang-tidy (for static analysis)
- ccache (for faster builds)

### IDE Setup

**VS Code:** Install recommended extensions from `.vscode/extensions.json`

**Qt Creator:** Open CMakeLists.txt directly

**CLion:** Open project, CMake will configure automatically

---

## Making Changes

### 1. Find an Issue

Look for issues labeled `good first issue` or `help wanted`:
- https://github.com/devTechs001/aether-media-engine/issues

### 2. Create a Plan

For larger changes, create a design document or discuss in the issue first.

### 3. Implement

```bash
# Make your changes
# Follow coding standards (see below)

# Build and test frequently
cmake --build build -j$(nproc)
ctest --test-dir build
```

### 4. Format Code

```bash
# Format your changes
clang-format -i src/your_file.cpp include/aether/your_header.hpp
```

### 5. Run Tests

```bash
# All tests
ctest --test-dir build --output-on-failure

# Specific tests
ctest -R your_test --verbose
```

---

## Pull Request Guidelines

### PR Title Format

```
type: short description

Examples:
feat: add AV1 hardware decoding support
fix: resolve memory leak in video renderer
docs: update build instructions for Windows
refactor: simplify pipeline initialization
test: add unit tests for demuxer
```

### Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `refactor`: Code refactoring
- `test`: Adding tests
- `chore`: Build/config changes
- `perf`: Performance improvements

### PR Description Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
- [ ] Unit tests added/updated
- [ ] Integration tests added/updated
- [ ] Manual testing performed

## Checklist
- [ ] Code follows project guidelines
- [ ] Self-review completed
- [ ] Comments added where necessary
- [ ] Documentation updated
- [ ] No new warnings
- [ ] Tests pass locally
```

---

## Coding Standards

### C++ Style

We use `clang-format` with the configuration in `.clang-format`. Key points:

```cpp
// 4 space indentation, no tabs
// 120 character line limit
// CamelCase for classes/structs
// snake_case for variables/functions
// UPPER_CASE for constants/macros

class VideoDecoder {
public:
    explicit VideoDecoder(const DecoderConfig& config);
    ~VideoDecoder() override;

    Result<void> Initialize();
    void Shutdown();

private:
    DecoderConfig m_config;
    std::unique_ptr<Impl> m_impl;
};

// Use smart pointers
auto decoder = std::make_unique<VideoDecoder>(config);

// Use const references for input parameters
void ProcessFrame(const VideoFrame& frame);

// Use Result<T> for error handling
Result<void> Initialize();
```

### Documentation

Use Doxygen-style comments:

```cpp
/**
 * @brief Initialize the decoder
 * @param config Decoder configuration
 * @return Success or error
 *
 * Detailed description if needed.
 */
virtual Result<void> Initialize(const DecoderConfig& config) = 0;
```

### Error Handling

```cpp
// Use Result<T> for recoverable errors
Result<void> Open(const std::string& path) {
    if (path.empty()) {
        return MakeErrorResult<void>(
            ErrorCode::InvalidArgument,
            "Path cannot be empty"
        );
    }
    // ...
    return {};
}

// Use exceptions for exceptional cases
if (!critical_resource) {
    throw AetherException(ErrorCode::NotInitialized, "Resource failed");
}
```

---

## Testing

### Writing Tests

```cpp
#include <gtest/gtest.h>
#include "aether/core/types.hpp"

TEST(VideoDecoderTest, InitializeWithValidConfig) {
    DecoderConfig config;
    config.codec_id = CodecID::H264;
    config.hardware_accel = false;

    auto decoder = CreateVideoDecoder(CodecID::H264);
    ASSERT_TRUE(decoder);

    EXPECT_TRUE(decoder->Open(config).has_value());
}

TEST(VideoDecoderTest, HandleInvalidConfig) {
    auto decoder = CreateVideoDecoder(CodecID::H264);
    DecoderConfig invalid_config;
    invalid_config.codec_id = CodecID::Unknown;

    EXPECT_FALSE(decoder->Open(invalid_config).has_value());
}
```

### Running Tests

```bash
# All tests
ctest --test-dir build

# Verbose output
ctest --test-dir build --verbose

# Specific test
ctest --test-dir build -R VideoDecoderTest

# With coverage
cmake -B build -DCODE_COVERAGE=ON
cmake --build build
ctest --test-dir build
```

---

## Documentation

### Code Documentation

- Document all public APIs
- Include examples for complex functions
- Keep comments up to date

### User Documentation

Update documentation in `docs/` for:
- New features
- Configuration changes
- API changes

Build documentation:

```bash
# API docs
cd build
cmake --build . --target docs

# User docs
cd docs
mkdocs build
```

---

## Commit Guidelines

### Commit Message Format

```
type(scope): subject

body (optional)

footer (optional)
```

### Examples

```
feat(decoder): add AV1 hardware decoding support

Implemented AV1 hardware decoding using NVIDIA NVDEC and Intel QSV.
Falls back to software decoding if hardware not available.

Closes #123

Co-authored-by: Jane Doe <jane@example.com>
```

### Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation
- `style`: Formatting
- `refactor`: Refactoring
- `test`: Tests
- `chore`: Maintenance

---

## Review Process

1. **Automated Checks**: CI/CD must pass
2. **Code Review**: At least one maintainer approval
3. **Testing**: All tests must pass
4. **Documentation**: Updated if needed

### Review Feedback

- Address all review comments
- Push fixes as separate commits
- Request re-review after addressing feedback

---

## Questions?

- Check existing [issues](https://github.com/devTechs001/aether-media-engine/issues)
- Start a [discussion](https://github.com/devTechs001/aether-media-engine/discussions)
- Ask in the issue you're working on

---

Thank you for contributing to AETHER Media Engine! 🎉
