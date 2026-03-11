# Media Player GUI

Modern cross-platform media player with Qt6/QML interface.

## Features

- 🎬 Video playback with multiple format support
- 🎵 Audio playback
- 📋 Playlist management
- ⏩ Variable playback speed (0.5x - 2.0x)
- 🔊 Volume control with mute
- 🖥️ Fullscreen mode (double-click)
- ⌨️ Keyboard shortcuts

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./mediagui
```

## Dependencies

- Qt 6.5+
- Qt Multimedia
- Qt Quick Controls

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| Space | Play/Pause |
| F | Fullscreen |
| ← | Seek backward |
| → | Seek forward |
| ↑ | Volume up |
| ↓ | Volume down |
| M | Mute |
| N | Next |
| P | Previous |
