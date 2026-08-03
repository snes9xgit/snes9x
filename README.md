# SuperSnes9x
*SuperSnes9x - Portable Super Nintendo Entertainment System (TM) emulator*

This is the un-official source code fork for the Snes9x project.

Please check the official [Wiki](https://github.com/snes9xgit/snes9x/wiki) for additional information.

## SuperSnes9x Features:
- Kaillera Server/Client
- RetroAchievements (with Hardcode)
- PPU Sprite/Tile/Tiles viewers (SNES, GB)
- Multi-Bind-Controller support
- SDL Support
- Run-Ahead (Input lag reduction)
- Color Correction Support / Native CRT Colors
- Enhanced Cheat Search / Cheat Editor (SNES, GB)
- Supports GameBoy, GameBoy Color, Super GameBoy (1 and 2)
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
