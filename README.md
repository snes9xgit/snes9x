# Snes9x
*Snes9x - Portable Super Nintendo Entertainment System (TM) emulator*

This is the official source code repository for the Snes9x project.

Please check the [Wiki](https://github.com/snes9xgit/snes9x/wiki) for additional information.

## Nightly builds

Download nightly builds from continuous integration:

### snes9x

| OS            | status                                           |
|---------------|--------------------------------------------------|
| Windows       | [![Status][s9x-win-all]][appveyor]               |
| Linux (GTK)   | [![Status][snes9x_linux-gtk-amd64]][cirrus-ci]   |
| Linux (X11)   | [![Status][snes9x_linux-x11-amd64]][cirrus-ci]   |
| FreeBSD (X11) | [![Status][snes9x_freebsd-x11-amd64]][cirrus-ci] |
| macOS         | [![Status][snes9x_macOS-amd64]][cirrus-ci]       |

[appveyor]: https://ci.appveyor.com/project/snes9x/snes9x
[cirrus-ci]: http://cirrus-ci.com/github/snes9xgit/snes9x

[s9x-win-all]: https://ci.appveyor.com/api/projects/status/github/snes9xgit/snes9x?branch=master&svg=true
[snes9x_linux-gtk-amd64]: https://api.cirrus-ci.com/github/snes9xgit/snes9x.svg?task=snes9x_linux-gtk-amd64
[snes9x_linux-x11-amd64]: https://api.cirrus-ci.com/github/snes9xgit/snes9x.svg?task=snes9x_linux-x11-amd64
[snes9x_freebsd-x11-amd64]: https://api.cirrus-ci.com/github/snes9xgit/snes9x.svg?task=snes9x_freebsd-x11-amd64
[snes9x_macOS-amd64]: https://api.cirrus-ci.com/github/snes9xgit/snes9x.svg?task=snes9x_macOS-amd64

### libretro core

| OS                  | status                                                  |
|---------------------|---------------------------------------------------------|
| Linux/amd64         | [![Status][libretro_linux-amd64]][cirrus-ci]            |
| Linux/i386          | [![Status][libretro_linux-i386]][cirrus-ci]             |
| Linux/armhf         | [![Status][libretro_linux-armhf]][cirrus-ci]            |
| Linux/armv7-neon-hf | [![Status][libretro_linux-armv7-neon-hf]][cirrus-ci]    |
| Linux/arm64         | [![Status][libretro_linux-arm64]][cirrus-ci]            |
| Android/arm         | [![Status][libretro_android-arm]][cirrus-ci]            |
| Android/arm64       | [![Status][libretro_android-arm64]][cirrus-ci]          |
| Emscripten          | [![Status][libretro_emscripten]][cirrus-ci]             |
| macOS/amd64         | [![Status][libretro_macOS-amd64]][cirrus-ci]            |
| Nintendo Wii        | [![Status][libretro_nintendo-wii]][cirrus-ci]           |
| Nintendo Switch     | [![Status][libretro_nintendo-switch-libnx]][cirrus-ci]  |
| Nintendo GameCube   | [![Status][libretro_nintendo-ngc]][cirrus-ci]           |
| PSP                 | [![Status][libretro_playstation-psp]][cirrus-ci]        |

[libretro_linux-amd64]: https://api.cirrus-ci.com/github/snes9xgit/snes9x.svg?task=libretro_linux-amd64
[libretro_linux-i386]: https://api.cirrus-ci.com/github/snes9xgit/snes9x.svg?task=libretro_linux-i386
[libretro_linux-armhf]: https://api.cirrus-ci.com/github/snes9xgit/snes9x.svg?task=libretro_linux-armhf
[libretro_linux-armv7-neon-hf]: https://api.cirrus-ci.com/github/snes9xgit/snes9x.svg?task=libretro_linux-armv7-neon-hf
[libretro_linux-arm64]: https://api.cirrus-ci.com/github/snes9xgit/snes9x.svg?task=libretro_linux-arm64
[libretro_android-arm]: https://api.cirrus-ci.com/github/snes9xgit/snes9x.svg?task=libretro_android-arm
[libretro_android-arm64]: https://api.cirrus-ci.com/github/snes9xgit/snes9x.svg?task=libretro_android-arm64
[libretro_emscripten]: https://api.cirrus-ci.com/github/snes9xgit/snes9x.svg?task=libretro_emscripten
[libretro_macOS-amd64]: https://api.cirrus-ci.com/github/snes9xgit/snes9x.svg?task=libretro_macOS-amd64
[libretro_nintendo-wii]: https://api.cirrus-ci.com/github/snes9xgit/snes9x.svg?task=libretro_nintendo-wii
[libretro_nintendo-switch-libnx]: https://api.cirrus-ci.com/github/snes9xgit/snes9x.svg?task=libretro_nintendo-switch-libnx
[libretro_nintendo-ngc]: https://api.cirrus-ci.com/github/snes9xgit/snes9x.svg?task=libretro_nintendo-ngc
[libretro_playstation-psp]: https://api.cirrus-ci.com/github/snes9xgit/snes9x.svg?task=libretro_playstation-psp
