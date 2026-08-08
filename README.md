# SuperSnes9x
*SuperSnes9x - Portable Super Nintendo Entertainment System (TM) emulator*

This is the un-official source code fork for the Snes9x project.

Please check the official [Wiki](https://github.com/snes9xgit/snes9x/wiki) for additional information.

## SuperSnes9x Features:
- Super FX 3 (FX3) support for the 2026 LRG releases, with cycle-accurate GSU timing
- LRG SNES rumble dongle support (Ultimate Doom FX3, RT.SFC)
- Supports GameBoy, GameBoy Color, SuperGameBoy (1 and 2)
- libretro core for SNES / SGB / GB / GBC
- Voicer-kun peripheral emulation (plays its audio CDs from cue/bin images)
- Super Famicom Box (hotel SNES) emulation, with live English OSD translations
- Event carts - PowerFest '94, Nintendo Campus Challenge '92
- Voicer-kun - infrared transmitter/receiver that connects to the second controller port
- Kaillera Server/Client
- RetroAchievements (with Hardcore)
- PPU Sprite/Tile/Tiles viewers (SNES, GB, GBC, SGB)
- Audio waveform viewer (Logic-style per-channel tracks, solo/mute, level meters)
- Enhanced Cheat Search / Cheat Editor (SNES, GB — incl. GB cheats in SGB BIOS mode)
- Multi-Bind-Controller support
- Run-Ahead (Input lag reduction)
- Color Correction Support / Native CRT Colors
- SDL Support
- Multi-Language Pack

## SuperSnes9x libretro core

The SuperSnes9x libretro core (`supersnes9x_libretro.so` — SNES / SFC / SGB /
GB / GBC in one core) is built from this repository. There are no prebuilt
binaries yet; build the portable Linux core locally with Docker:

```bash
cd libretro/linux
./build-portable.sh x86_64        # output: libretro/linux/dist/x86_64/
```

Then copy `supersnes9x_libretro.so` together with
`libretro/supersnes9x_libretro.info` into your RetroArch cores directory
(e.g. `~/.config/retroarch/cores/`). For authentic Super Game Boy mode,
place the SGB BIOS ROMs (`SGB1.sfc` / `SGB2.sfc`) in RetroArch's system
directory — without them, GB content runs on the built-in BIOS-less core.

Or you can build all versions: appimage, linux, android (requires docker).

```bash
cd libretro
./build-all.sh
```

## macOS

The Qt GUI and the libretro core both build on macOS, self-signed so they run
without a paid Apple Developer account:

```bash
# Qt GUI -> qt/build-macos/super-snes9x-qt.app
cmake -S qt -B qt/build-macos -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build qt/build-macos -j"$(sysctl -n hw.ncpu)"
qt/scripts/makeapp-macos.sh

# libretro core -> libretro/macos/dist/ (universal x86_64 + arm64)
libretro/macos/build-macos.sh
```

Or build both at once with `cd libretro && ./build-all.sh --osx`. That flag
selects the macOS artifacts *instead of* the Linux and Android ones — the
Linux AppImages are native builds and the macOS ones need Xcode, so a single
host can't produce both.

The macOS GUI uses the Qt Software or OpenGL display driver; Vulkan is not
available on the platform. Because the builds are ad-hoc signed rather than
notarized, a copy that arrives over the network needs its quarantine flag
cleared: `xattr -dr com.apple.quarantine <path>`.

See [qt/docs/README-macos.md](qt/docs/README-macos.md) for details.

## Upstream builds (plain snes9x, not SuperSnes9x)

Official upstream snes9x builds, for reference:

| OS      | status / downloads                             |
|---------|------------------------------------------------|
| Windows | [![Status][s9x-win-all]][appveyor]             |
| All     | [upstream releases][snes9x-releases]           |

[appveyor]: https://ci.appveyor.com/project/snes9x/snes9x
[s9x-win-all]: https://ci.appveyor.com/api/projects/status/github/snes9xgit/snes9x?branch=master&svg=true
[snes9x-releases]: https://github.com/snes9xgit/snes9x/releases

(The Cirrus CI badge tables that used to be here were removed: Cirrus CI
has shut down, so those status images no longer load.)
