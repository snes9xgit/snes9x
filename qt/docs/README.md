# Super Snes9x Qt — Building and Running (Linux)

This is the Qt 6 port of Super Snes9x (our fork of Snes9x — binaries are
named `super-snes9x-qt`/`super-snes9x-gtk` to distinguish them from
upstream), the recommended GUI on Linux (preferred over the older GTK
port). These instructions cover building from source, running,
and packaging an AppImage for distribution.

## Dependencies

A C++20 compiler, CMake ≥ 3.20, and Ninja (or Make), plus development
packages for Qt 6 and a few libraries. On Ubuntu 22.04 or newer:

```sh
sudo apt install build-essential cmake ninja-build pkg-config git \
    qt6-base-dev qt6-base-private-dev libqt6svg6-dev \
    libcurl4-openssl-dev zlib1g-dev libpng-dev \
    libwayland-dev wayland-protocols libx11-dev libxext-dev \
    libgl-dev libegl-dev libpulse-dev libasound2-dev \
    libudev-dev libdbus-1-dev libusb-1.0-0-dev libxkbcommon-dev
```

Notes:

- **SDL3** (used for game controller input) is downloaded and built
  automatically via CMake FetchContent. If your distro ships SDL3, you can
  use it instead with `-DUSE_SYSTEM_SDL3=ON`.
- **libqt6svg6-dev** matters: the UI icons are SVGs, so the Qt SVG plugin
  must be present (and gets bundled into AppImages).
- glslang, SPIRV-Cross, cubeb (audio), and imgui are vendored in
  `external/` — nothing to install for those.

## Building

From the `qt` directory:

```sh
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Useful CMake options:

| Option | Default | Effect |
| --- | --- | --- |
| `CMAKE_BUILD_TYPE` | (empty) | Use `Release` for shipping, `RelWithDebInfo` for development |
| `USE_SYSTEM_SDL3` | `OFF` | Link the distro's SDL3 instead of building it |
| `USE_SANITIZERS` | `OFF` | Enable address/UB sanitizers (development) |

## Running

The built binary is self-contained:

```sh
./build/super-snes9x-qt [rom-file]
```

Optionally install it system-wide (binary, desktop entry, icon, and cheats
database):

```sh
sudo cmake --install build
```

## Packaging an AppImage

### Quick local build

From a configured build directory:

```sh
cd build
../scripts/makeappimage.sh
```

This installs into `AppDir`, runs `linuxdeploy` with its Qt plugin to bundle
the Qt libraries and platform plugins, and produces
`super-snes9x-qt-x86_64.AppImage`. The required tools (`linuxdeploy`,
`linuxdeploy-plugin-qt`, `appimagetool`) are downloaded automatically on
first run.

**Caveat:** an AppImage inherits the glibc requirement of the machine that
built it. One built on, say, Ubuntu 24.04 will not run on older distros.
Fine for local testing — not for shipping.

### Shippable build (recommended)

```sh
scripts/makeappimage-docker.sh
```

Requires Docker. This runs the entire build inside an `ubuntu:22.04`
container — the oldest Ubuntu LTS whose archive carries Qt 6 (glibc 2.35) —
so the resulting `qt/build-appimage/super-snes9x-qt-x86_64.AppImage` runs on
Ubuntu 22.04 and anything newer. (Ubuntu 20.04 is not a viable baseline: it
has no Qt 6 packages and left standard support in April 2025.)

### Running an AppImage

```sh
chmod +x super-snes9x-qt-x86_64.AppImage
./super-snes9x-qt-x86_64.AppImage
```

AppImages need FUSE 2 (`libfuse2` on Ubuntu ≤ 24.04, `libfuse2t64` on
24.04+). Without it, run with `--appimage-extract-and-run`.
