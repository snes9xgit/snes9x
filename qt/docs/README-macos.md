# Building Super Snes9x on macOS

Covers the Qt GUI (`.app` bundle) and the libretro core (`.dylib`), both
self-signed with an ad-hoc signature so they run without a paid Apple
Developer Program membership.

## Requirements

- Xcode command line tools: `xcode-select --install`
- Homebrew packages: `brew install cmake ninja pkg-config qt libpng`
- Submodules:
  ```sh
  git submodule update --init --recursive \
      external/cubeb external/glslang external/SPIRV-Cross \
      external/rcheevos external/vulkan-headers
  ```

zlib and libcurl come from the macOS SDK. SDL3 is fetched automatically by
CMake unless you pass `-DUSE_SYSTEM_SDL3=ON`.

## Qt GUI

```sh
cmake -S qt -B qt/build-macos -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build qt/build-macos -j"$(sysctl -n hw.ncpu)"
qt/scripts/makeapp-macos.sh
```

The result is `qt/build-macos/super-snes9x-qt.app`, with the Qt frameworks
copied in by `macdeployqt` and everything ad-hoc signed. Add `MAKE_DMG=1` to
also produce a `.dmg`.

### Why the packaging script does more than run macdeployqt

Homebrew's `qt` is a meta-formula whose modules live in sibling kegs, and its
plugins carry a single relative rpath (`@loader_path/../../../../lib`) that
stops meaning anything once a plugin is copied into a bundle. macdeployqt
resolves frameworks using only the rpaths of the binary it is scanning —
`-libpath` does not extend that search — so it skips those modules and still
exits 0. The visible symptom is subtle: `QtSvg` goes missing while the
`iconengines/libqsvgicon` plugin that needs it is deployed, so every SVG icon
silently fails to draw.

`makeapp-macos.sh` therefore also prunes plugins the emulator never uses
(`QtPdf`, `QtVirtualKeyboard`, webp), copies in whatever is still missing,
rewrites absolute Homebrew install names to `@rpath`, strips rpaths pointing
outside the bundle, and then *verifies* that no Mach-O in the bundle
references either a framework it does not contain or an absolute
`/usr/local` path. That last check is the real gate — a bundle that fails it
would still run on the build machine, where the system Qt is present, and
fail on every other Mac.

`libpng` is keg-only in some Homebrew setups. If `pkg_check_modules` cannot
find it, prepend it to the search path:

```sh
export PKG_CONFIG_PATH="$(brew --prefix libpng)/lib/pkgconfig:$PKG_CONFIG_PATH"
```

### Display drivers on macOS

| Driver | Status |
| --- | --- |
| Qt Software | Works. CPU blit, no shaders. |
| OpenGL | Works, via a `NSOpenGL` 4.1 core profile context. |
| Vulkan | **Not built.** macOS has no native Vulkan driver. |

The Vulkan canvas and everything under `common/video/vulkan/` that calls the
Vulkan API are excluded from the macOS build, and the driver is removed from
the Display settings list. A config file carrying `display_driver = vulkan`
(copied from a Linux or Windows machine, say) falls back to OpenGL.

Two consequences of macOS only offering OpenGL through a *core* profile,
capped at 4.1:

- Shader presets must be GLSL 1.50 or newer. Older `.glslp` presets written
  against `#version 120`/`130` will not compile. `.slangp` presets, which are
  cross-compiled through glslang/SPIRV-Cross, are unaffected.
- OpenGL is deprecated by Apple. It still works on macOS 14, but the Qt
  Software driver is the safe fallback if a future release removes it.

### Deployment target

Homebrew's `qt` bottles are built for the host macOS release, so the app
inherits that floor — currently **macOS 14.0**, which is what
`CMAKE_OSX_DEPLOYMENT_TARGET` defaults to in `qt/CMakeLists.txt`.

To support older systems you need a Qt that itself targets them (the official
online-installer builds target 12.0):

```sh
cmake -S qt -B qt/build-macos -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 ...
```

Lowering the target while still linking Homebrew's Qt only produces "built for
newer macOS version" linker warnings — it does not actually make the app run
on an older system.

### Architecture

The Qt build is single-architecture: whatever the host is, matching the Qt you
link against. Homebrew ships a native-only Qt, so a universal `.app` would
require building Qt from source as a universal binary first. The libretro core
below has no such constraint and is built universal.

## Both at once

`build-all.sh --osx` builds the .app and the macOS core together, cleaning
stale artifacts first and printing the path of each result:

```sh
cd libretro
./build-all.sh --osx                 # .app + universal core
./build-all.sh --osx --compress      # also zip the core with its .info
./build-all.sh --osx --copy          # collect both into libretro/dist
./build-all.sh --osx --copy /path    # ... into /path instead
```

`--osx` is a mode, not an addition: with it the script builds *only* the
macOS artifacts, and without it only the Linux and Android ones. The Linux
GUI AppImages are native builds and the macOS artifacts need Xcode and
codesign, so no single host produces both. Passing `--osx` off macOS is
rejected immediately rather than failing partway through.

## libretro core

```sh
libretro/macos/build-macos.sh            # universal: x86_64 + arm64
libretro/macos/build-macos.sh x86_64     # single arch
```

Output: `libretro/macos/dist/supersnes9x_libretro.dylib` plus its `.info`.
Drop both into RetroArch's `cores` directory.

Each architecture is compiled separately and merged with `lipo`, because
passing several `-arch` flags to one `-flto` link is not reliable on Apple's
toolchain. The script re-signs after `lipo`, since merging rewrites the
Mach-O headers and invalidates any earlier signature.

You can also drive the Makefile directly:

```sh
make -C libretro platform=osx OSX_ARCHS=arm64 OSX_MIN_VERSION=11.0
```

`OSX_MIN_VERSION` defaults to 10.13; arm64 requires at least 11.0.

## Code signing

Everything is signed ad-hoc (`codesign -s -`). Without a $99/yr Apple
Developer Program membership that is the strongest signature available, and it
is enough to:

- satisfy the Apple Silicon requirement that all executable code be signed
- keep the bundle's own integrity seal valid
- let the app and core run normally on the machine that built them

What it does **not** do is notarize. Anything that arrives over the network —
downloaded, AirDropped, or copied from a disk image — picks up the
`com.apple.quarantine` attribute, and Gatekeeper will refuse it, usually with
the misleading message *"…is damaged and can't be opened."* The fix is to
strip the attribute after copying:

```sh
xattr -dr com.apple.quarantine /Applications/super-snes9x-qt.app
xattr -dr com.apple.quarantine ~/Library/.../supersnes9x_libretro.dylib
```

Right-click → Open does **not** work around this for ad-hoc signed apps on
current macOS; the `xattr` command is the reliable route.

A self-signed *certificate* created in Keychain Access is no better than an
ad-hoc signature here: Gatekeeper only trusts certificates chaining to Apple's
Developer ID root, which requires the paid membership. The only difference a
certificate would make is a stable signing identity across builds.

If you do have a Developer ID certificate, both scripts honour it:

```sh
SIGN_IDENTITY="Developer ID Application: Your Name (TEAMID)" \
    qt/scripts/makeapp-macos.sh
```

You would still need `xcrun notarytool submit` and `xcrun stapler staple`
afterwards for a download that opens without the `xattr` step.
