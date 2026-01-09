# Abuse (SDL2 Port)

[![License: Public Domain](https://img.shields.io/badge/License-Public%20Domain-blue.svg)](https://en.wikipedia.org/wiki/Public_domain)
[![Build Status](https://img.shields.io/badge/Build-CMake-brightgreen.svg)](https://cmake.org/)

Welcome to the modernized port of the classic dark 2D side-scrolling platformer, **Abuse**. Originally developed by Crack dot Com in 1995, this version has been migrated from SDL 1.2 to **SDL 2.0** for modern compatibility and performance.

## 🚀 Key Improvements in this Port

- **Modern SDL 2.0 Integration**: Fully migrated from the deprecated SDL 1.2.
- **Hardware Acceleration**: Uses the SDL 2 Renderer API for efficient hardware-accelerated 2D rendering.
- **Modern Build System**: Replaced the legacy Autotools setup with a clean, fast **CMake** build environment.
- **Robust Rendering**: Maintains the classic 8-bit aesthetic while using modern texture uploading for scaling and fullscreen support.
- **Flexible Audio**: Added `HAVE_SDL_MIXER` support, allowing the game to build even if audio libraries are missing (using stubs).

## 🛠️ Requirements

To build and run Abuse, you will need:
- **SDL 2.0** development libraries.
- **CMake 3.10** or higher.
- **SDL2_mixer** (Optional, for sound).
- A C++11 compliant compiler (GCC/Clang).

## 🏗️ Building the Game

The build process is now standard for CMake projects:

```bash
mkdir build
cd build
cmake ..
make
```

The resulting `abuse` binary will be located in the `build` directory.

## 🕹️ Running Abuse

Ensure you have the game datafiles available. You can specify the data directory using the `-datadir` flag:

```bash
./build/abuse -datadir ./data
```

### Command-line Options
- `-size <arg>`: Set the screen resolution.
- `-fullscreen`: Enable fullscreen mode.
- `-nosound`: Disable sound.
- `-edit`: Start in editor mode.
- `-scale <arg>`: Scale the display by the specified amount.

## ⌨️ Configuration

Abuse creates a configuration file (`abuserc`) in your `~/.abuse` directory the first time it is run. You can customize keys and other settings there.

## 📜 Credits

- **Original Game**: Jonathan Clark, Dave Taylor, and the Crack Dot Com team.
- **SDL 1.2 Port**: Anthony Kruize, Sam Hocevar.
- **SDL 2 Migration**: Antigravity AI.

---

*Enjoy the classic carnage in high-definition (scaled) glory!*
