#!/usr/bin/env bash
#
# Master build: Qt GUI, GTK GUI, Linux libretro core, Android libretro core.
# With --osx instead: the macOS Qt .app bundle and the universal
# (x86_64 + arm64) macOS libretro core, and nothing else.
#
# Cleans every executable/.so first so stale artifacts can never be copied,
# then rebuilds everything and prints the full path of each fresh file.
#
# Usage:
#   ./build-all.sh                # Android cores for arm64-v8a + armeabi-v7a
#   ./build-all.sh --osx          # build ONLY the macOS .app + macOS core
#   ./build-all.sh --copy         # also collect artifacts into libretro/dist
#   ./build-all.sh --copy /path   # ... into /path instead
#   ./build-all.sh --compress     # zip each core with its .info
#   ANDROID_ABI=all ./build-all.sh
#
# --copy gathers every artifact (AppImages/.app, cores, .info files) flat
# into one directory after a fully successful build. It defaults to
# libretro/dist next to this script; pass a path (--copy /path, or
# --copy=/path) or set SHARE_DIR to send them elsewhere, e.g. a VM share:
#   ./build-all.sh --copy /mnt/hgfs/Shared
# The directory is created if missing.
# --compress zips each core together with its matching .info next to the
# core; AppImages are never compressed. With --copy, the destination
# receives the zips instead of the bare core/.info files.
#
# Notes:
# - The Linux core is built with linux/build-portable.sh (needs docker):
#   an Ubuntu 18.04 container with static libstdc++, so the released .so
#   loads on essentially any distro (verified GLIBC_2.14 floor).
# - The x86_64 GUI AppImages here are native builds. For public releases
#   build the Qt AppImage with qt/scripts/makeappimage-docker.sh (Ubuntu
#   22.04 baseline).
# - The x86 (32-bit) GUI AppImages are docker builds against Debian 12,
#   the only current i386 archive that ships Qt 6.
# - --osx is a separate mode rather than an addition, because the two halves
#   cannot be crossed: the Linux GUI AppImages are native builds, and the
#   macOS artifacts need Xcode and codesign. It must be run on macOS.
#   The docker-based Linux and Android cores do work on macOS -- run
#   linux/build-portable.sh and jni/build-android.sh directly for those.
# - The macOS artifacts are ad-hoc signed, not notarized. See
#   qt/docs/README-macos.md for the quarantine caveat.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
ANDROID_ABI="${ANDROID_ABI:-arm64-v8a armeabi-v7a}"
# Default destination for --copy: a dist/ directory beside this script. The
# environment can override it, and an explicit --copy <path> beats both.
SHARE_DIR="${SHARE_DIR:-${SCRIPT_DIR}/dist}"

# macOS has no nproc, so resolve the job count portably.
if command -v nproc >/dev/null 2>&1; then
    JOBS="$(nproc)"
else
    JOBS="$(sysctl -n hw.ncpu)"
fi

COPY_TO_SHARE=0
COMPRESS=0
BUILD_OSX=0
while [ "$#" -gt 0 ]; do
    case "$1" in
        --copy)
            COPY_TO_SHARE=1
            # The destination is optional, so only swallow the next argument
            # when there is one and it is not itself a flag. Guarding on $#
            # first keeps `set -u` from tripping over an unset $2.
            if [ "$#" -gt 1 ] && [ "${2#-}" = "$2" ]; then
                SHARE_DIR="$2"
                shift
            fi
            ;;
        --copy=*)
            COPY_TO_SHARE=1
            SHARE_DIR="${1#--copy=}"
            ;;
        --compress) COMPRESS=1 ;;
        --osx) BUILD_OSX=1 ;;
        *) echo "unknown option: $1" >&2; exit 2 ;;
    esac
    shift
done

# The macOS artifacts need clang, the macOS SDK and codesign, none of which
# exist off Darwin -- fail early rather than midway through a long build.
if [ "${BUILD_OSX}" -eq 1 ] && [ "$(uname -s)" != "Darwin" ]; then
    echo "error: --osx must be run on macOS (this host is $(uname -s))" >&2
    exit 2
fi

QT_BIN="${REPO_DIR}/qt/build/super-snes9x-qt"
GTK_BIN="${REPO_DIR}/gtk/build/super-snes9x-gtk"
QT_APPIMAGE="${REPO_DIR}/qt/build/super-snes9x-qt-x86_64.AppImage"
GTK_APPIMAGE="${REPO_DIR}/gtk/build/super-snes9x-gtk-x86_64.AppImage"
QT_APPIMAGE_X86="${REPO_DIR}/qt/build-appimage-x86/super-snes9x-qt-x86.AppImage"
GTK_APPIMAGE_X86="${REPO_DIR}/gtk/build-appimage-x86/super-snes9x-gtk-x86.AppImage"
LINUX_SO="${SCRIPT_DIR}/linux/dist/x86_64/supersnes9x_libretro-x64.so"
LINUX_SO_X86="${SCRIPT_DIR}/linux/dist/x86/supersnes9x_libretro.so"
ANDROID_DIST="${SCRIPT_DIR}/jni/dist"

MACOS_BUILD_DIR="${REPO_DIR}/qt/build-macos"
MACOS_APP="${MACOS_BUILD_DIR}/super-snes9x-qt.app"
MACOS_DYLIB="${SCRIPT_DIR}/macos/dist/supersnes9x_libretro.dylib"

# Run the AppImage tools without FUSE (works everywhere, incl. containers).
export APPIMAGE_EXTRACT_AND_RUN=1

step() { echo; echo "==> $*"; }

# ---- Clean old artifacts -------------------------------------------------

step "Cleaning old executables and cores"
if [ "${BUILD_OSX}" -eq 1 ]; then
    rm -rf "${MACOS_APP}"
    rm -f "${MACOS_DYLIB}" "${MACOS_DYLIB%.dylib}.info" \
          "${MACOS_DYLIB%.dylib}.zip" \
          "${MACOS_BUILD_DIR}/super-snes9x-qt.dmg" \
          "${SCRIPT_DIR}/supersnes9x_libretro.dylib"
else
    rm -f "${QT_BIN}" "${GTK_BIN}" "${QT_APPIMAGE}" "${GTK_APPIMAGE}" \
          "${QT_APPIMAGE_X86}" "${GTK_APPIMAGE_X86}" \
          "${LINUX_SO}" "${LINUX_SO_X86}" \
          "${SCRIPT_DIR}/linux/dist/x86_64/supersnes9x_libretro.so" \
          "${SCRIPT_DIR}/supersnes9x_libretro.so"
    rm -rf "${REPO_DIR}/qt/build/AppDir" "${REPO_DIR}/gtk/build/AppDir"
    rm -rf "${ANDROID_DIST}" "${SCRIPT_DIR}/libs" "${SCRIPT_DIR}/obj"
fi
# Libretro objects live in the repo root and are shared with other Makefile
# builds; a clean here guarantees the core is rebuilt from current sources.
make -C "${SCRIPT_DIR}" clean >/dev/null 2>&1 || true

if [ "${BUILD_OSX}" -eq 1 ]; then

# ---- macOS Qt .app -------------------------------------------------------

step "Building Qt GUI (macOS .app)"
if [ ! -f "${MACOS_BUILD_DIR}/CMakeCache.txt" ]; then
    # Homebrew keeps qt keg-only, and libpng's .pc can sit outside the
    # default search path, so point CMake and pkg-config at them explicitly.
    # Written without arrays: macOS ships bash 3.2, where expanding an empty
    # array under `set -u` aborts the script.
    QT_PREFIX=""
    if command -v brew >/dev/null 2>&1; then
        QT_PREFIX="$(brew --prefix qt 2>/dev/null || true)"
        LIBPNG_PREFIX="$(brew --prefix libpng 2>/dev/null || true)"
        if [ -n "${LIBPNG_PREFIX}" ]; then
            export PKG_CONFIG_PATH="${LIBPNG_PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
        fi
    fi
    if [ -n "${QT_PREFIX}" ]; then
        cmake -S "${REPO_DIR}/qt" -B "${MACOS_BUILD_DIR}" -G Ninja \
              -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="${QT_PREFIX}"
    else
        cmake -S "${REPO_DIR}/qt" -B "${MACOS_BUILD_DIR}" -G Ninja \
              -DCMAKE_BUILD_TYPE=Release
    fi
else
    # CMake writes Contents/Info.plist at *generate* time; there is no build
    # rule for it. The clean step above deletes the .app, so `cmake --build`
    # alone would relink the executable into a bundle with no Info.plist at
    # all -- no identifier, no icon, no Retina flag. Re-generate from the
    # existing cache to put it back.
    cmake -S "${REPO_DIR}/qt" -B "${MACOS_BUILD_DIR}" >/dev/null
fi
cmake --build "${MACOS_BUILD_DIR}" -j"${JOBS}"

step "Packaging and signing the .app (macdeployqt + ad-hoc codesign)"
"${REPO_DIR}/qt/scripts/makeapp-macos.sh" "${MACOS_BUILD_DIR}"

# ---- macOS libretro core -------------------------------------------------

step "Building macOS libretro core (universal: x86_64 + arm64)"
"${SCRIPT_DIR}/macos/build-macos.sh"

else

# ---- Qt GUI --------------------------------------------------------------

step "Building Qt GUI"
if [ ! -f "${REPO_DIR}/qt/build/CMakeCache.txt" ]; then
    cmake -S "${REPO_DIR}/qt" -B "${REPO_DIR}/qt/build" -DCMAKE_BUILD_TYPE=Release
fi
cmake --build "${REPO_DIR}/qt/build" -j"${JOBS}"

step "Packaging Qt AppImage"
(cd "${REPO_DIR}/qt/build" && ../scripts/makeappimage.sh)

# ---- GTK GUI -------------------------------------------------------------

step "Building GTK GUI"
if [ ! -f "${REPO_DIR}/gtk/build/CMakeCache.txt" ]; then
    cmake -S "${REPO_DIR}/gtk" -B "${REPO_DIR}/gtk/build" -DCMAKE_BUILD_TYPE=Release
fi
cmake --build "${REPO_DIR}/gtk/build" -j"${JOBS}"

step "Packaging GTK AppImage"
(cd "${REPO_DIR}/gtk/build" && ../scripts/makeappimage.sh)

# ---- 32-bit (x86) GUI AppImages -----------------------------------------

step "Building Qt AppImage (x86 32-bit, Debian 12 baseline, docker)"
"${REPO_DIR}/qt/scripts/makeappimage-x86-docker.sh"

step "Building GTK AppImage (x86 32-bit, Debian 12 baseline, docker)"
"${REPO_DIR}/gtk/scripts/makeappimage-x86-docker.sh"

# ---- Linux libretro core -------------------------------------------------

step "Building Linux libretro cores (portable, Ubuntu 18.04 baseline: x86_64 + x86)"
"${SCRIPT_DIR}/linux/build-portable.sh" x86_64 x86

# ---- Android libretro core ----------------------------------------------

step "Building Android libretro core (${ANDROID_ABI})"
bash "${SCRIPT_DIR}/jni/build-android.sh" "${ANDROID_ABI}"

fi

# ---- Summary -------------------------------------------------------------

echo
echo "================ Build artifacts ================"
missing=0
# -e rather than -f: the macOS artifact is an .app bundle, i.e. a directory.
show() {
    if [ -e "$1" ]; then
        echo "  $1"
    else
        echo "  MISSING: $1"
        missing=1
    fi
}
if [ "${BUILD_OSX}" -eq 1 ]; then
    show "${MACOS_APP}"
    show "${MACOS_DYLIB}"
else
    show "${QT_BIN}"
    show "${GTK_BIN}"
    show "${QT_APPIMAGE}"
    show "${GTK_APPIMAGE}"
    show "${QT_APPIMAGE_X86}"
    show "${GTK_APPIMAGE_X86}"
    show "${LINUX_SO}"
    show "${LINUX_SO_X86}"
    found_android=0
    for so in "${ANDROID_DIST}"/*/supersnes9x_libretro_android*.so; do
        [ -f "$so" ] && { echo "  $so"; found_android=1; }
    done
    if [ "${found_android}" -eq 0 ]; then
        echo "  MISSING: ${ANDROID_DIST}/<abi>/supersnes9x_libretro_android*.so"
        missing=1
    fi
fi
echo "================================================="

# ---- Compress cores (.so + matching .info) -------------------------------

if [ "${COMPRESS}" -eq 1 ]; then
    if [ "${missing}" -ne 0 ]; then
        echo "Not compressing: build has missing artifacts." >&2
        exit 1
    fi
    if ! command -v zip >/dev/null; then
        echo "error: zip is required for --compress (sudo apt install zip)" >&2
        exit 1
    fi
    step "Compressing cores (core + .info)"
    if [ "${BUILD_OSX}" -eq 1 ]; then
        zipf="${MACOS_DYLIB%.dylib}.zip"
        rm -f "${zipf}"
        zip -j -q "${zipf}" "${MACOS_DYLIB}" "${MACOS_DYLIB%.dylib}.info"
        echo "  ${zipf}"
    else
        for so in "${LINUX_SO}" "${LINUX_SO_X86}" \
                  "${ANDROID_DIST}"/*/supersnes9x_libretro_android*.so; do
            zipf="${so%.so}.zip"
            rm -f "${zipf}"
            zip -j -q "${zipf}" "${so}" "${so%.so}.info"
            echo "  ${zipf}"
        done
    fi
fi

# ---- Collect artifacts into one directory --------------------------------

if [ "${COPY_TO_SHARE}" -eq 1 ]; then
    if [ "${missing}" -ne 0 ]; then
        echo "Not copying to ${SHARE_DIR}: build has missing artifacts." >&2
        exit 1
    fi
    step "Copying artifacts to ${SHARE_DIR}"

    # A VM share that is not mounted silently accepts writes onto the local
    # disk, so when the destination looks like one, insist it really is
    # mounted. Ordinary directories just get created.
    case "${SHARE_DIR}" in
        /mnt/hgfs|/mnt/hgfs/*)
            if ! df -PT "${SHARE_DIR}" 2>/dev/null | grep -q hgfs; then
                echo "error: ${SHARE_DIR} is not on a mounted hgfs share" >&2
                echo "       (mount it with: sudo mount -t fuse.vmhgfs-fuse .host:/ /mnt/hgfs -o allow_other)" >&2
                exit 1
            fi
            ;;
        *)
            mkdir -p "${SHARE_DIR}"
            ;;
    esac

    if [ "${BUILD_OSX}" -eq 1 ]; then
        # -R because the .app is a directory tree. Remove any previous copy
        # first, or cp -R nests the new bundle inside the old one.
        rm -rf "${SHARE_DIR}/$(basename "${MACOS_APP}")"
        cp -Rv "${MACOS_APP}" "${SHARE_DIR}/"
        if [ "${COMPRESS}" -eq 1 ]; then
            cp -v "${MACOS_DYLIB%.dylib}.zip" "${SHARE_DIR}/"
        else
            cp -v "${MACOS_DYLIB}" "${MACOS_DYLIB%.dylib}.info" "${SHARE_DIR}/"
        fi
    else
        if [ "${COMPRESS}" -eq 1 ]; then
            cp -v "${QT_APPIMAGE}" "${GTK_APPIMAGE}" "${QT_APPIMAGE_X86}" "${GTK_APPIMAGE_X86}" \
                  "${LINUX_SO%.so}.zip" "${LINUX_SO_X86%.so}.zip" \
                  "${ANDROID_DIST}"/*/supersnes9x_libretro_android*.zip \
                  "${SHARE_DIR}/"
        else
            cp -v "${QT_APPIMAGE}" "${GTK_APPIMAGE}" "${QT_APPIMAGE_X86}" "${GTK_APPIMAGE_X86}" \
                  "${LINUX_SO}" "${LINUX_SO%.so}.info" \
                  "${LINUX_SO_X86}" "${LINUX_SO_X86%.so}.info" \
                  "${ANDROID_DIST}"/*/supersnes9x_libretro_android*.so \
                  "${ANDROID_DIST}"/*/supersnes9x_libretro_android*.info \
                  "${SHARE_DIR}/"
        fi
    fi
fi

exit "${missing}"
