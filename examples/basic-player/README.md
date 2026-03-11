# Basic Player Example

This example demonstrates how to use AETHER Media Engine to create a simple media player.

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./basic-player
```

## Usage

```bash
./basic-player <media-file>
```

## Code Overview

The example shows:
- Engine initialization
- Player creation
- Media loading
- Playback control
- Event handling

See `main.cpp` for the complete implementation.
